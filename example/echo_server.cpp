//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async.hpp>
#include <boost/async/main.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <list>

// tag::decls[]
namespace async = boost::async;
using boost::asio::ip::tcp;
using boost::asio::detached;
using tcp_acceptor = async::use_op_t::as_default_on_t<tcp::acceptor>;
using tcp_socket   = async::use_op_t::as_default_on_t<tcp::socket>;
namespace this_coro = boost::async::this_coro;
//end::decls[]

// tag::echo[]
async::promise<void> echo(tcp_socket socket)
{
  try // <1>
  {
    char data[1024];
    for (;;) // <2>
    {
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data)); // <3>
      co_await async_write(socket, boost::asio::buffer(data, n)); // <4>
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo: exception: %s\n", e.what());
  }
}
// end::echo[]


// tag::listen[]
async::generator<tcp_socket> listen()
{
  tcp_acceptor acceptor({co_await async::this_coro::executor}, {tcp::v4(), 55555});
  for (;;) // <1>
  {
    tcp_socket sock = co_await acceptor.async_accept(); // <2>
    co_yield std::move(sock); // <3>
  }
}
// end::listen[]

// tag::run_server[]
async::promise<void> run_server(async::wait_group & workers)
{
  auto l = listen(); // <1>
  while (true)
  {
    if (workers.size() == 10u)
      co_await workers.wait_one();  // <2>
    else
      workers.push_back(echo(co_await l)); // <3>
  }
}
// end::run_server[]

// tag::main[]
async::main co_main(int argc, char ** argv)
{
  co_await async::with(async::wait_group(), &run_server); // <1>
  co_return 0u;
}
// end::main[]
