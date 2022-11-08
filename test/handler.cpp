// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/handler.hpp>

#include <asio/any_io_executor.hpp>
#include <asio/post.hpp>

struct dummy_promise
{
    using executor_type = asio::any_io_executor;
     executor_type get_executor() const;

};

static_assert(asio::detail::has_executor_type<dummy_promise>::value);


void test(coro::completion_handler<dummy_promise> ch)
{
    asio::post(std::move(ch));
}