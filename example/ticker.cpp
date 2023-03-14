 // Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async.hpp>
#include <boost/async/main.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/json.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/case_conv.hpp>
// cause cmake
#include <boost/url/src.hpp>

#include <list>

#include <iostream>

// https://blockchain.info/ticker
// https://api.coingecko.com/api/v3/coins/list

using namespace boost;
using executor_type = async::use_op_t::executor_with_default<asio::io_context::executor_type>;
using socket_type   = typename asio::ip::tcp::socket::rebind_executor<executor_type>::other;
using acceptor_type = typename asio::ip::tcp::acceptor::rebind_executor<executor_type>::other;
using websocket_type = beast::websocket::stream<asio::ssl::stream<socket_type>>;

async::promise<json::value> get(asio::ssl::stream<asio::ip::tcp::socket> & sock,
                                beast::string_view host,
                                beast::string_view url,
                                beast::flat_buffer & buffer)
{
    beast::http::request<beast::http::empty_body> req{beast::http::verb::get, url, 11};
    req.set(beast::http::field::host, host);
    req.set(beast::http::field::accept, "application/json");
    req.set(beast::http::field::user_agent, "async/beast");
    co_await beast::http::async_write(sock, req, async::use_op);

    beast::http::response<beast::http::string_body> res;
    co_await beast::http::async_read(sock, buffer, res, async::use_op);

    co_return json::parse(res.body());
}

async::promise<asio::ssl::stream<socket_type>> connect(
        std::string host, boost::asio::ssl::context & ctx)
{
    asio::ip::tcp::resolver res{async::this_thread::get_executor()};
    auto ep = co_await res.async_resolve(host, "https", async::use_op);

    asio::ssl::stream<socket_type> sock{async::this_thread::get_executor(), ctx};
    co_await sock.next_layer().async_connect(*ep.begin(), async::use_op);
    co_await sock.async_handshake(asio::ssl::stream_base::client);

    co_return sock;
}

async::generator<json::object> json_reader(websocket_type & ws)
try {
    beast::flat_buffer buf;
    while (ws.is_open())
    {
        auto sz = co_await ws.async_read(buf);
        json::string_view data{static_cast<const char*>(buf.cdata().data()), buf.cdata().size()};
        buf.consume(sz);
        co_yield json::parse(data).as_object();
    }
    co_return {};
}
catch (std::exception & e)
{
  std::cerr << "Error reading: " << e.what() << std::endl;
  throw;
}

async::promise<void> connect_to_blockchain_info(websocket_type & ws)
{
    asio::ssl::context ctx{asio::ssl::context_base::tls_client};
    ctx.set_default_verify_paths();
    ws.set_option(beast::websocket::stream_base::decorator(
            [](beast::websocket::request_type& req)
            {
                req.set(beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " async-ticker");
                req.set(beast::http::field::origin, "https://exchange.blockchain.com");
            }));

    co_await ws.async_handshake("ws.blockchain.info", "/mercury-gateway/v1/ws");
}

using subscription = std::pair<std::string, std::weak_ptr<async::channel<json::object>>>;

async::promise<void> run_blockchain_info(async::channel<subscription> & subc)
try
{
    asio::ssl::context ctx{asio::ssl::context_base::tls_client};
    std::cout  << "Connecting to blockchain.info " << std::endl;
    websocket_type ws{co_await connect("blockchain.info", ctx)};
    std::cout  << "Connected to blockchain.info " << std::endl;
    co_await connect_to_blockchain_info(ws);
    std::cout  << "Initialized the websocket " << std::endl;

    boost::unordered_multimap<std::string,
                              std::weak_ptr<async::channel<json::object>>> subs;

    std::list<std::string> unconfirmed;

    auto rd = json_reader(ws);

    while (ws.is_open())
    {
        switch (auto msg = co_await async::select(rd, subc.read()); msg.index())
        {
            case 0:// message
            {
                auto ms = get<0>(msg);
                if (ms.at("event") == "rejected") // invalid sub, cancel however subbed
                {
                    auto rej = unconfirmed.front();
                    unconfirmed.pop_front();
                    auto r = subs.equal_range(rej);
                    for (const auto & [k, chn] : boost::make_iterator_range(r))
                        if (auto ptr = chn.lock())
                        {
                            co_await ptr->write(ms);
                            ptr->close();
                        }

                    subs.erase(r.first, r.second);
                    continue;
                }

                const auto & sym = json::value_to<std::string>(ms.at("symbol"));

                if (!unconfirmed.empty() && sym == unconfirmed.front())
                    unconfirmed.pop_front();

                bool has_expired = false;
                auto r = boost::make_iterator_range(subs.equal_range(sym));
                for (const auto & [k, chn] : r)
                    if (auto ptr = chn.lock())
                        co_await ptr->write(ms);
                    else
                        has_expired = true;

                if (has_expired)
                    erase_if(subs, [](const auto & p) {return p.second.expired();});

                if (r.empty() && ms.at("event") != "unsubscribed") // this is expired, so let's close it - unless this is the unsub message
                {
                    auto msg = json::serialize(json::object{ {"action", "unsubscribe"}, {"channel", "ticker"}, {"symbol", sym}});
                    co_await ws.async_write(asio::buffer(msg));
                }
            }
            break;
            case 1: //new sub
            {
                auto sym = get<1>(msg).first;
                std::cout << "New subscription: " << sym << std::endl;
                if (!subs.contains(sym))
                {
                    auto msg = json::serialize(json::object{ {"action", "subscribe"}, {"channel", "ticker"}, {"symbol", sym}});
                    unconfirmed.push_back(sym);
                    co_await ws.async_write(asio::buffer(msg));
                }
                subs.emplace(std::move(variant2::get<1>(msg)));
            }
        }
    }

    for (auto & [k ,c] : subs)
    {
        if (auto ptr = c.lock())
            ptr->close();
    }
}
catch(std::exception & e)
{
  printf("Exception: %s\n", e.what());
}

async::promise<void> read_and_close(beast::websocket::stream<socket_type> & st, beast::flat_buffer buf)
{
    system::error_code ec;
    co_await st.async_read(buf, asio::as_tuple(async::use_op));
    co_await st.async_close(beast::websocket::close_code::going_away, asio::as_tuple(async::use_op));
    st.next_layer().close(ec);
}

async::promise<void> run_session(beast::websocket::stream<socket_type> st,
                                 async::channel<subscription> & subc)
try
{
    beast::http::request<beast::http::empty_body> req;
    beast::flat_buffer buf;
    co_await beast::http::async_read(st.next_layer(), buf, req);
    // check the target
    auto r = urls::parse_uri_reference(req.target());
    if (r.has_error() || (r->segments().size() != 2u))
    {
        beast::http::response<beast::http::string_body> res{beast::http::status::bad_request, 11};
        res.body() = r.has_error() ? r.error().message() : "url needs two segments, e.g. /btc/usd";
        co_await beast::http::async_write(st.next_layer(), res);
        st.next_layer().close();
        co_return ;
    }

    co_await st.async_accept(req);

    auto sym = std::string(r->segments().front()) + "-" + std::string(r->segments().back());
    boost::algorithm::to_upper(sym);
    // close when data gets sent
    auto p = read_and_close(st, std::move(buf));

    auto ptr = std::make_shared<async::channel<json::object>>(1u);
    std::cout << "Subscribing to " << sym << std::endl;
    co_await subc.write(subscription{sym, ptr});
    std::cout << "Sent subscription" << std::endl;

    while (ptr->is_open())
    {
        auto bb = json::serialize(co_await ptr->read());
        co_await st.async_write(asio::buffer(bb));
    }

    co_await st.async_close(beast::websocket::close_code::going_away);
    st.next_layer().close();
    co_await p;

}
catch(std::exception & e)
{
    std::cerr << "Session ended with exception: " << e.what() << std::endl;
}

async::main co_main(int argc, char * argv[])
{
    acceptor_type acc{co_await async::this_coro::executor, asio::ip::tcp::endpoint (asio::ip::tcp::v4(), 8080)};
    std::cout << "Listening on localhost:8080" << std::endl;
    std::list<async::promise<void>> sessions;
    const int limit = 10; // allow 10 ongoing sessions

    async::channel<subscription> sub_manager;
    auto p = run_blockchain_info(sub_manager);
    while (true)
    {
        std::exception_ptr ex;
        try
        {
            if (sessions.size() >= limit)
            {
                auto idx = co_await async::select(sessions);
                sessions.erase(std::next(sessions.begin(), idx));
            }
            auto conn = co_await acc.async_accept();
            sessions.push_back(run_session(beast::websocket::stream<socket_type>{std::move(conn)}, sub_manager));
        }
        catch (std::exception& e)
        {
            ex = std::current_exception();
        }
        if (ex)
        {
            for (auto & sess : sessions)
                sess.cancel();

            if (!sessions.empty())
                co_await async::gather(sessions);
            std::rethrow_exception(ex);
        }
    }
    co_await p;

    co_return 0;
}