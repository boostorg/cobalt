//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/thread.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"
#include "boost/async/spawn.hpp"

boost::async::thread thr()
{
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(100)};

  auto exec = co_await boost::asio::this_coro::executor;
  co_await tim.async_wait(boost::async::use_op);
}

TEST_SUITE_BEGIN("thread");

TEST_CASE("run")
{
  auto t = thr();

  t.join();
  CHECK_THROWS(t.get_executor());
}


boost::async::thread thr_stop()
{
  boost::asio::steady_timer tim{co_await boost::asio::this_coro::executor, std::chrono::milliseconds(100)};

#if !defined(BOOST_ASYNC_USE_IO_CONTEXT)
  auto exec = co_await boost::asio::this_coro::executor;
  auto exc = exec.target<boost::asio::io_context::executor_type>();
  REQUIRE(exc != nullptr);
#else
  auto exc = co_await boost::asio::this_coro::executor;
#endif

  exc->context().stop();
  co_await tim.async_wait(boost::async::use_op);
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
  co_await tim.async_wait(boost::async::use_op);
  co_await th;
  CHECK_THROWS(co_await th);
}

boost::async::task<std::thread::id> on_thread()
{
  co_return std::this_thread::get_id();
}

CO_TEST_CASE("spawn_onto_thread")
{
  using namespace boost;
  auto t = thr();

  auto id = co_await async::spawn(t.get_executor(), on_thread(), async::use_op);

  auto id2 = t.get_id();

  auto id3 = std::this_thread::get_id();
  CHECK(id == id2);
  CHECK(id3 != id);
  t.join();
}


TEST_SUITE_END();