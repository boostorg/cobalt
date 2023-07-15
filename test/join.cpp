//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/join.hpp>
#include <boost/async/generator.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/op.hpp>
#include <boost/async/thread.hpp>

#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

static async::promise<std::size_t> wdummy(asio::any_io_executor exec,
                                  std::chrono::milliseconds ms = std::chrono::milliseconds(25))
{
  if (ms == std::chrono::milliseconds ::max())
    throw std::runtime_error("wdummy_throw");

  asio::steady_timer tim{exec, ms};
  co_await tim.async_wait(asio::deferred);
  co_return ms.count();
}

static async::generator<int> wgen(asio::any_io_executor exec)
{
  asio::steady_timer tim{exec, std::chrono::milliseconds(25)};
  co_await tim.async_wait(asio::deferred);
  co_return 123;
}

static async::promise<void> wthrow()
{
  throw std::runtime_error("wthrow");
  co_return;
}

static async::promise<void> wnever()
{
  asio::steady_timer tim{async::this_thread::get_executor(),
                         std::chrono::steady_clock::time_point::max()};
  co_await tim.async_wait(asio::deferred);
  co_return;
}


TEST_SUITE_BEGIN("join");

CO_TEST_CASE("variadic")
{
  auto exec = co_await asio::this_coro::executor;
  auto d1 = wdummy(exec, std::chrono::milliseconds(100));
  auto d2 = wdummy(exec, std::chrono::milliseconds( 50));
  asio::steady_timer tim{co_await asio::this_coro::executor};
  auto g = wgen(exec);
  auto c = co_await join(d1, d2, wdummy(exec, std::chrono::milliseconds(150)),
                           g);

  CHECK(std::get<0>(c) == 100);
  CHECK(std::get<1>(c) ==  50);
  CHECK(std::get<2>(c) ==  150);
  CHECK(std::get<3>(c) ==  123);
}

CO_TEST_CASE("variadic-throw")
{
  auto exec = co_await asio::this_coro::executor;
  auto d1 = wdummy(exec, std::chrono::milliseconds(100));
  auto d2 = wdummy(exec, std::chrono::milliseconds( 50));
  asio::steady_timer tim{co_await asio::this_coro::executor};
  auto g = wgen(exec);
  CHECK_THROWS(co_await join(d1, d2, wdummy(exec, std::chrono::milliseconds(150)),
                         g, wthrow()));

  co_await d1;
  co_await d2;
}

CO_TEST_CASE("list")
{
  auto exec = co_await asio::this_coro::executor;
  std::vector<async::promise<std::size_t>> vec;
  vec.push_back(wdummy(exec, std::chrono::milliseconds(100)));
  vec.push_back(wdummy(exec, std::chrono::milliseconds( 50)));
  vec.push_back(wdummy(exec, std::chrono::milliseconds(150)));

  auto res = co_await join(std::move(vec));
  REQUIRE(res.size() == 3);
  CHECK(res[0] == 100);
  CHECK(res[1] == 50);
  CHECK(res[2] == 150);
}

CO_TEST_CASE("list-exception")
{
  auto exec = co_await asio::this_coro::executor;
  std::vector<async::promise<void>> vec;
  vec.push_back(wthrow());
  vec.push_back(wnever());

  CHECK_THROWS(co_await join(std::move(vec)));
}

CO_TEST_CASE("compliance")
{
  auto exec = co_await asio::this_coro::executor;
  {
    auto d = wdummy(exec, std::chrono::milliseconds(1));
    immediate i;
    co_await join(d, i);
  }

  {
    auto d = wdummy(exec, std::chrono::milliseconds(1));
    immediate_bool i;
    co_await join(d, i);
  }
  {
    auto d = wdummy(exec, std::chrono::milliseconds(1));
    immediate_handle i;
    co_await join(d, i);
  }
  {
    auto d = wdummy(exec, std::chrono::milliseconds(1));
    posted p;
    co_await join(d, p);
  }
  {
    auto d = wdummy(exec, std::chrono::milliseconds(1));
    posted_bool p;
    co_await join(d, p);
  }
  {
    auto d = wdummy(exec, std::chrono::milliseconds(1));
    posted_handle p;
    co_await join(d, p);
  }
}

TEST_SUITE_END();