// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/main.hpp>
#include <coro/async_operation.hpp>
#include <asio/steady_timer.hpp>
#include <coro/ops.hpp>
#include <experimental/source_location>
#include <source_location>

coro::main co_main(int argc, char *argv[])
{
    asio::steady_timer tim{co_await asio::this_coro::executor, std::chrono::seconds(1)};
//    auto [ec] = co_await tim.async_wait(coro::deferred);
//
    asio::error_code  ec;
    co_await coro::ops::wait(tim, ec);
    printf("tim wait: %s\n", ec.message().c_str());
    auto lc = std::experimental::source_location::current();
    printf("SRC: %s %s %d %d\n", lc.file_name(), lc.function_name(), lc.line(), lc.column());
    printf("FOOBAR %d\n", __has_builtin(__builtin_source_location));

    co_return 0;
}
