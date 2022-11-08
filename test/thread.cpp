//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <coro/thread.hpp>
#include <coro/async_operation.hpp>
#include <asio/steady_timer.hpp>
#include <coro/ops.hpp>

#include "doctest.h"

coro::thread thr()
{
  asio::steady_timer tim{co_await asio::this_coro::executor, std::chrono::seconds(1)};

  auto exec = co_await asio::this_coro::executor;
  asio::error_code ec;
  co_await coro::ops::wait(tim, ec);
}

TEST_CASE("thread")
{
  auto t = thr();

  t.join();
}