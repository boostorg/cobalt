// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/detail/wrapper.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/bind_allocator.hpp>


#include "doctest.h"

TEST_SUITE_BEGIN("wrappers");

TEST_CASE("regular")
{
    boost::asio::io_context ctx;
    boost::async::this_thread::set_executor(ctx.get_executor());
    bool ran = false;

    char buf[512];
    boost::async::pmr::monotonic_buffer_resource res{buf, 512};
    auto p = boost::async::detail::post_coroutine(ctx.get_executor(),
                                              boost::asio::bind_allocator(
                                              boost::async::pmr::polymorphic_allocator<void>(&res),
                                              [&]{ran = true;}
                                              )
                                          );
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
  boost::async::this_thread::set_executor(ct2.get_executor());
  auto h = boost::async::detail::post_coroutine(ct2.get_executor(), boost::asio::detached);
  boost::async::detail::self_destroy(h);
}

TEST_CASE("immediate")
{
  boost::asio::io_context ctx;
  boost::async::this_thread::set_executor(ctx.get_executor());
  bool called = false;
  auto l = [&]{called = true;};
  auto h = boost::async::detail::immediate_coroutine(l);
  boost::async::detail::self_destroy(h);
  CHECK(!called);

  h = boost::async::detail::immediate_coroutine(l);
  h();
  CHECK(called);
  ctx.run();
}



TEST_SUITE_END();