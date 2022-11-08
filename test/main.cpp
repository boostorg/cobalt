// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/main.hpp>
#include "boost/async/detail/async_operation.hpp"
#include <boost/asio/steady_timer.hpp>
#include <boost/async/src.hpp>


boost::async::main co_main(int argc, char *argv[]) // <asio::thread_pool>
{
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(50)};
  co_await tim.async_wait(boost::asio::deferred);

  co_return 0;
}
