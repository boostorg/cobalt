//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/select.hpp>
#include <boost/async/generator.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/op.hpp>

#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


static  async::promise<std::size_t> dummy(asio::any_io_executor exec,
                                  std::chrono::milliseconds ms = std::chrono::milliseconds(50))
{
  asio::steady_timer tim{exec, ms};
  co_await tim.async_wait(async::use_op);
  co_return ms.count();
}


static  async::promise<std::size_t> nothrow_dummy(asio::any_io_executor exec,
                                          std::chrono::milliseconds ms = std::chrono::milliseconds(50))
try {
  asio::steady_timer tim{exec, ms};
  co_await tim.async_wait(async::use_op);
  co_return ms.count();
}
catch(...)
{
  co_return std::numeric_limits<std::size_t>::max();
}

static async::generator<int> gen(asio::any_io_executor exec)
{
  asio::steady_timer tim{exec, std::chrono::milliseconds(50000)};
  co_await tim.async_wait(async::use_op);
  co_return 123;
}

TEST_SUITE_BEGIN("select");

CO_TEST_CASE("variadic")
{
  auto exec = co_await asio::this_coro::executor;
  auto d1 = dummy(exec, std::chrono::milliseconds(100));
  auto d2 = dummy(exec, std::chrono::milliseconds( 50));
  auto g = gen(exec);
  std::mt19937 src{1u};
  auto c = co_await select(src, d1, d2, dummy(exec, std::chrono::milliseconds(100000)), g);
  CHECK(c.index() == 1u);
  CHECK(boost::variant2::get<1>(c) == 50);
  CHECK(d1);
  //CHECK(!d1.ready()); NOTE: Inderministic on msvc, due to the additional post!
  CHECK( d2.ready());
  CHECK(100 == co_await d1);
  CHECK(!d1);
  CHECK( d1.ready());
  co_await d2;

  g.cancel();
  CHECK_THROWS(co_await g);
}


CO_TEST_CASE("list")
{

  std::mt19937::result_type seed;
  SUBCASE("1") {seed = 1u;}
  SUBCASE("2") {seed = 2u;}
  SUBCASE("3") {seed = 3u;}
  SUBCASE("4") {seed = 4u;}
  SUBCASE("5") {seed = 5u;}
  SUBCASE("6") {seed = 6u;}
  SUBCASE("7") {seed = 7u;}
  SUBCASE("8") {seed = 8u;}
  SUBCASE("9") {seed = 9u;}

  CAPTURE(seed);
  std::mt19937 src{seed};

  auto exec = co_await asio::this_coro::executor;
  std::vector<async::promise<std::size_t>> vec;
  vec.push_back(dummy(exec, std::chrono::milliseconds(100)));
  vec.push_back(dummy(exec, std::chrono::milliseconds( 50)));
  vec.push_back(dummy(exec, std::chrono::milliseconds(100000)));

  auto c = co_await select(src, vec);
  CHECK(c.first == 1u);
  CHECK(c.second == 50);
  CHECK(!vec[0].ready());
  CHECK( vec[1].ready());
  CHECK(co_await vec[0]);
  CHECK( vec[0].ready());
  CHECK( vec[1].ready());
  vec[2].cancel();
  CHECK( vec[2]);
  CHECK_THROWS(co_await vec[2]);
  CHECK_THROWS(co_await vec[2]);
  CHECK(!vec[2]);
}

CO_TEST_CASE("empty-list")
{
  auto exec = co_await asio::this_coro::executor;
  std::vector<async::promise<std::size_t>> vec;
  CHECK_THROWS(co_await select(vec));
}


CO_TEST_CASE("stop")
{
  auto d = nothrow_dummy(co_await asio::this_coro::executor,
                 std::chrono::milliseconds(10));
  CHECK((co_await left_select(d, stop())).index() == 0);
}

CO_TEST_CASE("compliance")
{
  auto exec = co_await asio::this_coro::executor;
  auto d = dummy(exec, std::chrono::milliseconds(100000));
  {
    immediate i;
    CHECK((co_await select(d, i)).index() == 1);
  }

  {
    immediate_bool i;
    CHECK((co_await select(d, i)).index() == 1);
  }

  {
    immediate_handle i;
    CHECK((co_await select(d, i)).index() == 1);
  }
  {
    posted p;
    CHECK((co_await select(d, p)).index() == 1);
  }
  {
    posted_bool p;
    CHECK((co_await select(d, p)).index() == 1);
  }
  {
    posted_handle p;
    CHECK((co_await select(d, p)).index() == 1);
  }
  d.cancel();
  CHECK_THROWS(co_await d);
}


TEST_SUITE_END();
