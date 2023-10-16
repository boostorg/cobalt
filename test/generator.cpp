//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/promise.hpp>
#include <boost/cobalt/race.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/core/ignore_unused.hpp>

#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


template<typename ... Ts>
struct doctest::StringMaker<variant2::variant<Ts...>>
{
  static String convert(const variant2::variant<Ts...> & v)
  {
    String init = doctest::toString(v.index());
    init += ":";
    init += variant2::visit(
        []<typename T>(const T& value)
        {
          return doctest::toString(value);
        }, v);
    return init;
  }


};


TEST_SUITE_BEGIN("generator");

cobalt::generator<int> gen()
{
  for (int i = 0; i <10; i ++)
    co_yield i;

  co_return 10;
}



CO_TEST_CASE("generator-int")
{
  auto g = gen();
  int i = 0;
  {
    auto aw = g.operator co_await();
    CHECK(aw.await_ready());
    CHECK(i ++ == co_await std::move(aw));
  }

  while (g)
    CHECK(i ++ == co_await g);



  CHECK(i == 11);

  co_return ;
}


cobalt::generator<int, int> gen_push()
{
  int val = 1u;
  for (int i = 0; i < 10; i++)
  {
    auto v = co_yield val;
    CHECK(v == val);
    val += v;
  }


  co_return val;
}


CO_TEST_CASE("generator-push")
{
  auto g = gen_push();

  int i = 1;
  int nw = 1;
  while (g)
  {
    nw = co_await g(i);
    CHECK(i == nw);
    i *= 2;
  }

  CHECK(i == 2048);
  co_return ;
}

cobalt::generator<int> delay_gen(std::chrono::milliseconds tick)
{
  asio::steady_timer tim{co_await cobalt::this_coro::executor, std::chrono::steady_clock::now()};
  for (int i = 0; i < 10; i ++)
  {
    co_await tim.async_wait(cobalt::use_op);
    tim.expires_at(tim.expiry() + tick);
    co_yield i;
  }
  co_return 10;
}

cobalt::generator<int> lazy_delay_gen(std::chrono::milliseconds tick)
{
  co_await cobalt::this_coro::initial;
  asio::steady_timer tim{co_await cobalt::this_coro::executor, std::chrono::steady_clock::now()};
  for (int i = 0; i < 10; i ++)
  {
    co_await tim.async_wait(cobalt::use_op);
    tim.expires_at(tim.expiry() + tick);
    co_yield i;
  }
  co_return 10;
}


#if !defined(BOOST_COBALT_NO_SELF_DELETE)

CO_TEST_CASE("generator-left_race")
{
  asio::steady_timer tim{co_await cobalt::this_coro::executor, std::chrono::milliseconds(50)};
  auto g1 = delay_gen(std::chrono::milliseconds(200));
  co_await tim.async_wait(cobalt::use_op);
  auto g2 = delay_gen(std::chrono::milliseconds(100));

  using v = variant2::variant<int, int>;
  auto v1 = [](int value) -> v {return v{variant2::in_place_index<0u>, value};};
  auto v2 = [](int value) -> v {return v{variant2::in_place_index<1u>, value};};

  CHECK(v1(0) == co_await left_race(g1, g2));
  CHECK(v2(0) == co_await left_race(g1, g2));
  CHECK(v2(1) == co_await left_race(g1, g2));
  CHECK(v1(1) == co_await left_race(g1, g2));
  CHECK(v2(2) == co_await left_race(g1, g2));
  CHECK(v2(3) == co_await left_race(g1, g2));
  CHECK(v1(2) == co_await left_race(g1, g2));
  CHECK(v2(4) == co_await left_race(g1, g2));
  CHECK(v2(5) == co_await left_race(g1, g2));
  CHECK(v1(3) == co_await left_race(g1, g2));
  CHECK(v2(6) == co_await left_race(g1, g2));
  CHECK(v2(7) == co_await left_race(g1, g2));
  CHECK(v1(4) == co_await left_race(g1, g2));
  CHECK(v2(8) == co_await left_race(g1, g2));
  CHECK(v2(9) == co_await left_race(g1, g2));
  CHECK(v2(10) == co_await left_race(g1, g2));


  CHECK(!g2);
  g1.cancel();
  CHECK_THROWS(co_await g1);
}


CO_TEST_CASE("lazy-generator-left_race")
{
  asio::steady_timer tim{co_await cobalt::this_coro::executor, std::chrono::milliseconds(50)};
  auto g1 = lazy_delay_gen(std::chrono::milliseconds(200));
  co_await tim.async_wait(cobalt::use_op);
  auto g2 = lazy_delay_gen(std::chrono::milliseconds(100));

  using v = variant2::variant<int, int>;
  auto v1 = [](int value) -> v {return v{variant2::in_place_index<0u>, value};};
  auto v2 = [](int value) -> v {return v{variant2::in_place_index<1u>, value};};

  CHECK(v1(0) == co_await left_race(g1, g2));
  CHECK(v2(0) == co_await left_race(g1, g2));
  CHECK(v2(1) == co_await left_race(g1, g2));
  CHECK(v2(2) == co_await left_race(g1, g2));
  CHECK(v1(1) == co_await left_race(g1, g2));
  CHECK(v2(3) == co_await left_race(g1, g2));
  CHECK(v2(4) == co_await left_race(g1, g2));
  CHECK(v1(2) == co_await left_race(g1, g2));
  CHECK(v2(5) == co_await left_race(g1, g2));
  CHECK(v2(6) == co_await left_race(g1, g2));
  CHECK(v1(3) == co_await left_race(g1, g2));
  CHECK(v2(7) == co_await left_race(g1, g2));
  CHECK(v2(8) == co_await left_race(g1, g2));
  CHECK(v1(4) == co_await left_race(g1, g2));
  CHECK(v2(9) == co_await left_race(g1, g2));
  CHECK(v2(10) == co_await left_race(g1, g2));


  CHECK(!g2);
  g1.cancel();
  CHECK_THROWS(co_await g1);
}

#endif

cobalt::generator<int> gshould_unwind(asio::io_context & ctx)
{
  co_await asio::post(ctx, cobalt::use_op);
  co_return 0;
}

TEST_CASE("unwind")
{
  asio::io_context ctx;
  boost::cobalt::this_thread::set_executor(ctx.get_executor());
  boost::ignore_unused(gshould_unwind(ctx));
}


cobalt::generator<int> gen_stop()
{
  int val = 1u;
  for (int i = 0; i < 10; i++)
  {
    if (i == 4)
      co_await stop();
    co_yield i;

  }
  co_return val;
}


CO_TEST_CASE("stop")
{
  auto g = gen_stop();
  while (g)
    co_await g;

  auto gg =std::move(g);
}

cobalt::generator<int, int> eager()
{
  int i = co_await cobalt::this_coro::initial;
  for (; i < 10; i += co_yield i);

  co_return i;
}


CO_TEST_CASE("eager")
{
  auto g = eager();


  CHECK(2 == co_await g(2));
  CHECK(6 == co_await g(4));
  CHECK(9 == co_await g(3));
  CHECK(15 == co_await g(6));
  CHECK(!g);

  auto gg =std::move(g);
}


TEST_SUITE_END();