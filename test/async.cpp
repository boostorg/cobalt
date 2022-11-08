//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <coro/async.hpp>
#include <coro/main.hpp>

#include "doctest.h"
#include "test.hpp"
#include "asio/detached.hpp"
#include <asio/steady_timer.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>

#include <new>

TEST_SUITE_BEGIN("async");

coro::async<void> test0()
{
    co_return;
}
coro::async<double> test2(int i)
{
    co_await test0();
    co_return i;
}


coro::async<int> test1(asio::any_io_executor exec)
{
    co_await test2(42);
    co_await test2(42);
    co_await asio::post(exec, asio::deferred);
    co_return 452;
}

TEST_CASE("test-1")
{

    bool done = false;

    asio::io_context ctx;
    asio::steady_timer tim{ctx};
    coro::set_executor(ctx.get_executor());

    coro::spawn(
          ctx.get_executor(),
          test1(ctx.get_executor()),
          [&](std::exception_ptr ex, int res)
          {
            CHECK(ex == nullptr);
            CHECK(res == 452);
            done = true;
          });

    ctx.run();
    CHECK(done);
}

CO_TEST_CASE("async-1")
{
    co_await test1(co_await asio::this_coro::executor);
    co_return;
}

TEST_SUITE_END();