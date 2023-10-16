//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/race.hpp>
#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/promise.hpp>
#include <boost/cobalt/op.hpp>

#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


static cobalt::promise<std::size_t> dummy(asio::any_io_executor exec,
                                  std::chrono::milliseconds ms = std::chrono::milliseconds(50))
{
  asio::steady_timer tim{exec, ms};
  co_await tim.async_wait(cobalt::use_op);
  co_return ms.count();
}

static cobalt::generator<int> gen(asio::any_io_executor exec)
{
  asio::steady_timer tim{exec, std::chrono::milliseconds(50000)};
  co_await tim.async_wait(cobalt::use_op);
  co_return 123;
}

TEST_SUITE_BEGIN("left_race");
#if !defined(BOOST_COBALT_NO_SELF_DELETE)

CO_TEST_CASE("variadic")
{
  auto exec = co_await asio::this_coro::executor;
  auto d1 = dummy(exec, std::chrono::milliseconds(100));
  auto d2 = dummy(exec, std::chrono::milliseconds( 50));
  auto g = gen(exec);
  auto c = co_await left_race(d1, d2, dummy(exec, std::chrono::milliseconds(100000)), g);
  CHECK(c.index() == 1u);
  CHECK(boost::variant2::get<1>(c) == 50);
  CHECK(d1);
  CHECK(!d1.ready());
  CHECK( d2.ready());
  CHECK(100 == co_await d1);
  CHECK(!d1);
  CHECK( d1.ready());
  co_await d2;

  g.cancel();
  CHECK_THROWS(co_await g);
}
#endif

CO_TEST_CASE("list")
{
  auto exec = co_await asio::this_coro::executor;
  std::vector<cobalt::promise<std::size_t>> vec;
  vec.push_back(dummy(exec, std::chrono::milliseconds(100)));
  vec.push_back(dummy(exec, std::chrono::milliseconds( 50)));
  vec.push_back(dummy(exec, std::chrono::milliseconds(100000)));

  auto c = co_await left_race(vec);
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
  std::vector<cobalt::promise<std::size_t>> vec;
  CHECK_THROWS(co_await left_race(vec));
}


CO_TEST_CASE("stop")
{
  auto d = dummy(co_await asio::this_coro::executor,
                 std::chrono::milliseconds(10));
  CHECK((co_await left_race(d, stop())).index() == 0);
}

CO_TEST_CASE("compliance")
{
  auto exec = co_await asio::this_coro::executor;
  auto d = dummy(exec, std::chrono::milliseconds(100000));
  SUBCASE("immediate")
  {
    immediate i;
    CHECK((co_await left_race(d, i)).index() == 1);
  }
  SUBCASE("immediate_bool")

  {
    immediate_bool i;
    CHECK((co_await left_race(d, i)).index() == 1);
  }

  SUBCASE("immediate_handle")
  {
    immediate_handle i;
    CHECK((co_await left_race(d, i)).index() == 1);
  }

  SUBCASE("posted")
  {
    posted p;
    CHECK((co_await left_race(d, p)).index() == 1);
  }
  SUBCASE("posted_bool")
  {
    posted_bool p;
    CHECK((co_await left_race(d, p)).index() == 1);
  }
  SUBCASE("posted_handle")
  {
    posted_handle p;
    CHECK((co_await left_race(d, p)).index() == 1);
  }
  d.cancel();
  CHECK_THROWS(co_await d);
}
CO_TEST_CASE("immediate timer")
{
  immediate i;
  asio::steady_timer tim{co_await cobalt::this_coro::executor, std::chrono::steady_clock::time_point::max()};
  CHECK((co_await left_race(tim.async_wait(cobalt::use_op), i)) == 1);

}

CO_TEST_CASE("compliance_ranged")
{
  CHECK(co_await cobalt::left_race(std::vector<immediate>(3u))        == 0);
  CHECK(co_await cobalt::left_race(std::vector<immediate_bool>(1u))   == 0);
  CHECK(co_await cobalt::left_race(std::vector<immediate_handle>(1u)) == 0);
  CHECK(co_await cobalt::left_race(std::vector<posted>(3u))           == 0);
  CHECK(co_await cobalt::left_race(std::vector<posted_bool>(1u))      == 0);
  CHECK(co_await cobalt::left_race(std::vector<posted_handle>(1u))    == 0);
}

TEST_SUITE_END();
