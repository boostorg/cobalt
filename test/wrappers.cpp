// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/cobalt/detail/wrapper.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/bind_allocator.hpp>


#include "doctest.h"

TEST_SUITE_BEGIN("wrappers");

TEST_CASE("regular")
{
    boost::asio::io_context ctx;
    boost::cobalt::this_thread::set_executor(ctx.get_executor());
    bool ran = false;

#if !defined(BOOST_COBALT_NO_PMR)
    char buf[512];
    boost::cobalt::pmr::monotonic_buffer_resource res{buf, 512};
    auto p = boost::cobalt::detail::post_coroutine(ctx.get_executor(),
                                              boost::asio::bind_allocator(
                                              boost::cobalt::pmr::polymorphic_allocator<void>(&res),
                                              [&]{ran = true;}
                                              )
                                          );
#else
  auto p = boost::cobalt::detail::post_coroutine(ctx.get_executor(), [&]{ran = true;});
#endif
    CHECK(p);
    CHECK(!ran);
    p.resume();
    CHECK(!ran);
    ctx.run();
    CHECK(ran);
}

TEST_CASE("expire")
{

  boost::asio::io_context ct2;
  boost::cobalt::this_thread::set_executor(ct2.get_executor());
  auto h = boost::cobalt::detail::post_coroutine(ct2.get_executor(), boost::asio::detached);
  boost::cobalt::detail::self_destroy(h);
}


TEST_SUITE_END();