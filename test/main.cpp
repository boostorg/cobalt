// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/main.hpp>
#include <coro/async_operation.hpp>
#include <asio/steady_timer.hpp>


coro::main co_main(int argc, char *argv[]) // <asio::thread_pool>
{
    asio::steady_timer tim{co_await asio::this_coro::executor, std::chrono::seconds(1)};
    co_await tim.async_wait(asio::deferred);

    co_return 0;
}
