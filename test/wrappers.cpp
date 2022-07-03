// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/detail/wrapper.hpp>

#include <asio/io_context.hpp>

int main()
{
    asio::io_context ctx;
    bool ran = false;

    std::aligned_storage_t<128u, 8u> store;
    auto p = coro::detail::post_coroutine(ctx.get_executor(), [&]{ran = true; throw 42;}, &store, 128u);
    assert(p);
    assert(!ran);
    p.resume();
    assert(!ran);
    ctx.run();
    assert(ran);

    return 0;
}