// Copyright (c) 2023 Matthijs Möhlmann
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/async.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/race.hpp>
#include <boost/async/this_thread.hpp>
#include <boost/beast.hpp>

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <stdexcept>

namespace async = boost::async;
namespace beast = boost::beast;

using executor_type = async::use_op_t::executor_with_default<async::executor>;
using socket_type = typename boost::asio::ip::tcp::socket::rebind_executor<
    executor_type>::other;
using ssl_socket_type = boost::asio::ssl::stream<socket_type>;
using acceptor_type = typename boost::asio::ip::tcp::acceptor::rebind_executor<
    executor_type>::other;
using websocket_type = beast::websocket::stream<ssl_socket_type>;


async::promise<ssl_socket_type> connect(std::string_view host,
                                        boost::asio::ssl::context &ctx) {
  boost::asio::ip::tcp::resolver resolve{async::this_thread::get_executor()};
  auto endpoints = co_await resolve.async_resolve(host, "https", async::use_op);

  // Timer for timeouts

  ssl_socket_type sock{async::this_thread::get_executor(), ctx};
  printf("connecting\n");

  co_await sock.next_layer().async_connect(*endpoints.begin());
  printf("connected\n");

  // Connected, now do the handshake
  printf("handshaking\n");
  co_await sock.async_handshake(boost::asio::ssl::stream_base::client);
  printf("hand shook\n");
  co_return sock;
}

async::main co_main(int argc, char **argv)
{
  boost::asio::ssl::context ctx{boost::asio::ssl::context::tls_client};
  auto conn = co_await connect("boost.org", ctx);
  printf("connected\n");
  beast::http::request<beast::http::empty_body> req{beast::http::verb::get, "/index.html", 11};
  req.set(beast::http::field::host, "boost.org");
  co_await beast::http::async_write(conn, req, async::use_op);

  // read the response
  beast::flat_buffer b;
  beast::http::response<beast::http::string_body> response;
  co_await beast::http::async_read(conn, b, response, async::use_op);

  // write the response
  printf("%s\n", response.body().c_str());
  co_return 0;
}
