// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/compose.hpp>
#include <coro/ops.hpp>
#include <asio/compose.hpp>
#include <asio/detached.hpp>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/recycling_allocator.hpp>
#include <asio/bind_allocator.hpp>

#include <stdexcept>

#include "doctest.h"


struct async_foo_impl
{
    template<typename Handler>
    void operator()(asio::steady_timer & tim,
                    coro::composed_op<Handler, asio::any_io_executor, void(asio::error_code, double)>)
    {
        // some dummy op for demonstration
        asio::error_code  ec;

        co_await coro::ops::wait(tim, ec);
        auto alloc = co_await coro::this_coro::allocator;
        co_return {ec, 4.2};
    }

};

template<asio::completion_token_for<void(asio::error_code, double)> CompletionToken>
auto async_foo(asio::steady_timer & tim, CompletionToken && token) // async
{
    return coro::async_compose<CompletionToken, void(asio::error_code, double)>(
            async_foo_impl{}, token, tim);
}

double foo(asio::steady_timer & tim) // throwing
{
    return coro::compose<void(asio::error_code, double)>(
            async_foo_impl{}, tim);
}

double foo(asio::steady_timer & tim, asio::error_code & ec) // non-throwing
{
    return coro::compose<void(asio::error_code, double)>(
            async_foo_impl{}, ec, tim);
}

struct async_noop_impl
{
    template<typename Handler>
    void operator()(asio::io_context & ctx,
                    coro::composed_op<Handler, asio::io_context::executor_type , void(double)>)
    {
        // some dummy op for demonstration
        co_return {2.4};
    }

};

template<asio::completion_token_for<void(double)> CompletionToken>
auto async_noop(asio::io_context & ctx, CompletionToken && token) // async
{
    return coro::async_compose<CompletionToken, void(double)>(
            async_noop_impl{}, token, ctx);
}

double noop(asio::io_context & ctx) // throwing
{
    return coro::compose<void(double)>(
            async_noop_impl{}, ctx);
}

struct async_throw_impl
{
    template<typename Handler>
    void operator()(asio::steady_timer & tim, bool post_first,
                    coro::composed_op<Handler, asio::any_io_executor, void(asio::error_code, double)>)
    {
        if (post_first)
            co_await coro::ops::wait(tim);
        throw std::runtime_error("test-exception");
        co_return {std::error_code{}, 4.2};
    }

};

template<asio::completion_token_for<void(asio::error_code, double)> CompletionToken>
auto async_throw(asio::steady_timer & tim, bool post_first, CompletionToken && token) // async
{
    return coro::async_compose<CompletionToken, void(asio::error_code, double)>(
            async_throw_impl{}, token, tim, post_first);
}

double throw_(asio::steady_timer & tim, bool post_first) // throwing
{
    return coro::compose<void(asio::error_code, double)>(
            async_throw_impl{}, tim, post_first);
}

double throw_(asio::steady_timer & tim, bool post_first, asio::error_code & ec) // non-throwing
{
    return coro::compose<void(asio::error_code, double)>(
            async_throw_impl{}, ec, tim, post_first);
}

TEST_SUITE_BEGIN("composed");


TEST_CASE("wait")
{
    asio::recycling_allocator<void> ra;
    asio::io_context ctx;
    asio::steady_timer tim{ctx};
    async_foo(tim, [](auto ec, auto d) { CHECK(!ec); CHECK(d == 4.2);});
    async_foo(tim, asio::bind_allocator(ra, asio::detached));
    auto d = foo(tim);

    asio::error_code ec;
    CHECK(foo(tim) == 4.2);
    CHECK(foo(tim, ec) == 4.2);
    ctx.run();
}

TEST_CASE("noop")
{
    asio::recycling_allocator<void> ra;
    asio::io_context ctx;
    async_noop(ctx, [](auto d) {CHECK(d == 2.4);});
    async_noop(ctx, asio::bind_allocator(ra, asio::detached));
    CHECK(noop(ctx) == 2.4);
    ctx.run();
}

TEST_CASE("direct-throw")
{
    asio::error_code ec;
    asio::recycling_allocator<void> ra;
    asio::io_context ctx;
    asio::steady_timer tim{ctx};

    CHECK_THROWS(throw_(tim, false), std::runtime_error("test-exception"));
    CHECK_THROWS(throw_(tim, false, ec), std::runtime_error("test-exception"));
    CHECK_THROWS(async_throw(tim, false, asio::bind_allocator(ra, asio::detached)), std::runtime_error("test-exception"));
    CHECK(ctx.run() == 0);
}

TEST_CASE("posted-throw")
{
    asio::error_code ec;
    asio::recycling_allocator<void> ra;
    asio::io_context ctx;
    asio::steady_timer tim{ctx};

    CHECK_THROWS(throw_(tim, true), std::runtime_error("test-exception"));
    CHECK_THROWS(throw_(tim, true, ec), std::runtime_error("test-exception"));
    async_throw(tim, true, asio::bind_allocator(ra, asio::detached));
    CHECK_THROWS(ctx.run(), std::runtime_error("test-exception"));
}


template <typename CompletionToken>
auto async_always_throws_1(CompletionToken&& token)
{
    return async_initiate<CompletionToken, void()>(
            [](auto&& completion_handler) // Handler not moved-from on exception.
            {
                (void)completion_handler;
                throw 42;
            }, token);
}

struct async_always_throws_impl
{
    template<typename Handler, typename Executor>
    void operator()(asio::io_context & tim,
                    coro::composed_op<Handler, Executor, void(double)>)
    {
        co_await async_always_throws_1(asio::deferred);
        co_return {4.2};
    }

};

template<asio::completion_token_for<void(double)> CompletionToken>
auto async_always_throws(asio::io_context & tim, CompletionToken && token) // async
{
    return coro::async_compose<CompletionToken, void(double)>(
            async_always_throws_impl{}, token, tim);
}

TEST_CASE("op-throw")
{
    asio::io_context ctx;
    async_always_throws(ctx, asio::detached);

    CHECK_THROWS(ctx.run());
}

TEST_SUITE_END();