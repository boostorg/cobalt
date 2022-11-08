//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/thread.hpp>
#include <boost/async/async_operation.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"

boost::async::thread thr()
{
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(100)};

  auto exec = co_await boost::asio::this_coro::executor;
  co_await tim.async_wait(boost::asio::deferred);
}

TEST_CASE("thread")
{
  auto t = thr();

  t.join();
}