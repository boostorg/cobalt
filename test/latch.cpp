// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/latch.hpp>
#include <coro/basic_coro.hpp>
#include <asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

TEST_SUITE_BEGIN("latch");

struct immediate
{
    bool await_ready() { return true; }
    bool await_suspend(std::coroutine_handle<void>) {assert(!"unreachable"); }
    int await_resume() {return 42;}
};

CO_TEST_CASE("immediate")
{
    coro::latch<int> l{co_await coro::this_coro::executor};
    l.await(immediate{});
    CHECK(!l.ex);
    CHECK(l.result);
    CHECK(l.result == 42);
}

struct immediate_throw
{
    bool await_ready() { return true; }
    bool await_suspend(std::coroutine_handle<void>) {assert(!"unreachable"); }
    int await_resume() {throw std::runtime_error("test"); return 42;}
};

CO_TEST_CASE("immediate_throw")
{
    coro::latch<int> l{co_await coro::this_coro::executor};
    l.await(immediate_throw{});
    CHECK(!l.result);
    CHECK(l.ex);
    CHECK_THROWS(co_await l, std::runtime_error("test"));
}

coro::task<double> posted(asio::any_io_executor exec)
{
    co_await asio::post(asio::deferred);
    co_return 4.2;
}

CO_TEST_CASE("posted")
{
    coro::latch<double> l{co_await coro::this_coro::executor};
    l.await(posted(co_await coro::this_coro::executor));
    CHECK(!l.result);
    CHECK(!l.ex);
    auto res = co_await l;
    CHECK(res);
    CHECK(4.2 == res);
}

coro::task<int> posted_throw(asio::any_io_executor exec)
{
    co_await asio::post(asio::deferred);
    throw std::logic_error("test");
}

CO_TEST_CASE("posted_throw")
{
    coro::latch<int> l{co_await coro::this_coro::executor};
    l.await(posted_throw(co_await coro::this_coro::executor));
    CHECK(!l.result);
    CHECK(!l.ex);
    CHECK_THROWS(co_await l, std::logic_error("test"));
}

coro::task<void> cancelled(asio::any_io_executor exec)
{
    asio::steady_timer tim{exec, std::chrono::seconds(1234)};
    co_await tim.async_wait(asio::deferred) ;
}

CO_TEST_CASE("cancelled")
{
    coro::latch<void> l{co_await coro::this_coro::executor};
    l.await(cancelled(co_await coro::this_coro::executor));
    CHECK(!l.result);
    CHECK(!l.ex);
    l.cancel();
    CHECK_THROWS(co_await l, asio::system_error(asio::error::operation_aborted));
}



TEST_SUITE_END();