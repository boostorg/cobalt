// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/detail/wrapper.hpp>

#include <asio/io_context.hpp>
#include "asio/bind_allocator.hpp"

int main()
{
    asio::io_context ctx;
    bool ran = false;

    char buf[512];
    std::pmr::monotonic_buffer_resource res{buf, 512};
    auto p = boost::async::detail::post_coroutine(ctx.get_executor(),
                                          asio::bind_allocator(
                                              std::pmr::polymorphic_allocator<void>(&res),
                                              [&]{ran = true; throw 42;}
                                              )
                                          );
    assert(p);
    assert(!ran);
    p.resume();
    assert(!ran);
    ctx.run();
    assert(ran);

    return 0;
}