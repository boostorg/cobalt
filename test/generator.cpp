//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/generator.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/select.hpp>
#include <boost/async/op.hpp>

#include <boost/asio.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


TEST_SUITE_BEGIN("generator");

async::generator<int> gen()
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


async::generator<int, int> gen_push()
{
  co_await async::this_coro::pro_active(true);
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

async::generator<int> delay_gen(std::chrono::milliseconds tick)
{
  asio::steady_timer tim{co_await async::this_coro::executor, std::chrono::steady_clock::now()};
  for (int i = 0; i < 10; i ++)
  {
    co_await tim.async_wait(asio::deferred);
    tim.expires_at(tim.expires_at() + tick);
    co_yield i;
  }
  co_return 10;
}

CO_TEST_CASE("generator-select")
{
  asio::steady_timer tim{co_await async::this_coro::executor, std::chrono::milliseconds(50)};
  auto g1 = delay_gen(std::chrono::milliseconds(200));
  co_await tim.async_wait(asio::deferred);
  auto g2 = delay_gen(std::chrono::milliseconds(100));

  std::vector<std::size_t> seq{
    0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1
  };

  std::vector<std::size_t> num{
    0, 0, 1, 1, 2, 3, 2, 4, 5, 3, 6, 7, 4, 8, 9, 10
  };

  auto itr = seq.begin();
  auto ntr = num.begin();

  int i = 0;
  while (g1 && g2)
  {
    auto r =  co_await select(g1, g2);
    REQUIRE(itr != seq.end());
    REQUIRE(ntr != num.end());
    CHECK(r.index() == *itr++);
    visit(
        [&](int i)
        {
          CHECK(i == *ntr++);
        }, r);
  }
  CHECK(itr == seq.end());
  CHECK(ntr == num.end());
  CHECK(!g2);
  g1.cancel();
  CHECK_THROWS(co_await g1);
  co_return ;
}


async::generator<int> gshould_unwind(asio::io_context & ctx)
{
  co_await asio::post(ctx, async::use_op);
  co_return 0;
}

TEST_CASE("unwind")
{
  asio::io_context ctx;
  boost::async::this_thread::set_executor(ctx.get_executor());
  boost::ignore_unused(gshould_unwind(ctx));
}


async::generator<int> gen_stop()
{
  co_await async::this_coro::pro_active(true);
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
}

TEST_SUITE_END();