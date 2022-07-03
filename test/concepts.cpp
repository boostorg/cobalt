// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/concepts.hpp>

struct foo
{

};

struct foo_aw
{
    bool await_ready();
    bool await_suspend(std::coroutine_handle<void>);
    void await_resume();
};

struct my_promise ;


struct special_aw
{
    bool await_ready();
    bool await_suspend(std::coroutine_handle<my_promise>);
    void await_resume();
};

foo_aw operator co_await(foo);


static_assert(coro::awaitable_type<std::suspend_never>);
static_assert(coro::awaitable_type<foo_aw>);
static_assert(!coro::awaitable_type<foo>);

static_assert(coro::awaitable<std::suspend_never>);
static_assert(coro::awaitable<foo_aw>);
static_assert(coro::awaitable<foo>);

static_assert(coro::awaitable<std::suspend_never, int>);
static_assert(coro::awaitable<foo_aw, int>);
static_assert(coro::awaitable<foo, int>);

static_assert(!coro::awaitable<special_aw, int>);
static_assert(!coro::awaitable<special_aw>);
static_assert(coro::awaitable<special_aw, my_promise>);