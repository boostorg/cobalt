//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/thread.hpp>
#include <boost/async/detail/async_operation.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

boost::async::thread thr()
{
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(100)};

  auto exec = co_await boost::asio::this_coro::executor;
  co_await tim.async_wait(boost::asio::deferred);
}

TEST_SUITE_BEGIN("thread");

TEST_CASE("run")
{
  auto t = thr();

  t.join();
}


boost::async::thread thr_stop()
{
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(100)};

  auto exec = co_await boost::asio::this_coro::executor;
  exec.context().stop();
  co_await tim.async_wait(boost::asio::deferred);
}

TEST_CASE("stop")
{
  auto t = thr_stop();
  t.join();
}

CO_TEST_CASE("await-thread")
{
  co_await thr();

  auto th = thr_stop();
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(200)};
  co_await tim.async_wait(boost::asio::deferred);
  co_await th;
  CHECK_THROWS(co_await th);
}


TEST_SUITE_END();