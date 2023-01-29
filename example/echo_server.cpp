//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <list>

namespace async = boost::async;
using boost::asio::ip::tcp;
using boost::asio::detached;
using tcp_acceptor = async::use_op_t::as_default_on_t<tcp::acceptor>;
using tcp_socket   = async::use_op_t::as_default_on_t<tcp::socket>;
namespace this_coro = boost::async::this_coro;

async::promise<void> echo(tcp_socket socket)
{
  try
  {
    char data[1024];
    for (;;)
    {
      std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data));
      co_await async_write(socket, boost::asio::buffer(data, n));
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

async::generator<tcp_socket> listen(boost::asio::any_io_executor exec)
{
  tcp_acceptor acceptor(exec, {tcp::v4(), 55555});
  for (;;)
    co_yield co_await acceptor.async_accept();
}

async::main co_main(int argc, char ** argv)
{
  std::list<async::promise<void>> workers;

  while (true)
  {
    if (workers.size() == 10u)
    {
      auto idx = co_await async::select(workers);
      workers.erase(std::next(workers.begin(), idx));
    }
    else
      workers.push_back(echo(co_await listen(co_await async::this_coro::executor)));
  }

  co_return 0u;
}