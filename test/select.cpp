//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/select.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/op.hpp>

#include <boost/asio.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


async::promise<std::size_t> dummy(asio::any_io_executor exec,
                                  std::chrono::milliseconds ms = std::chrono::milliseconds(50))
{
  asio::steady_timer tim{exec, ms};
  co_await tim.async_wait(asio::deferred);
  co_return ms.count();
}

TEST_SUITE_BEGIN("select");

CO_TEST_CASE("variadic")
{
  auto exec = co_await asio::this_coro::executor;
  auto d1 = dummy(exec, std::chrono::milliseconds(100));
  auto d2 = dummy(exec, std::chrono::milliseconds( 50));
  auto c = co_await select(d1, d2, dummy(exec, std::chrono::milliseconds(100000)));
  CHECK(c.index() == 1u);
  CHECK(boost::variant2::get<1>(c) == 50);
  CHECK(d1);
  CHECK(!d1.ready());
  CHECK( d2.ready());
  CHECK(100 == co_await d1);
  CHECK(!d1);
  CHECK( d1.ready());
  co_await d2;
}


CO_TEST_CASE("list")
{
  auto exec = co_await asio::this_coro::executor;
  std::vector<async::promise<std::size_t>> vec;
  vec.push_back(dummy(exec, std::chrono::milliseconds(100)));
  vec.push_back(dummy(exec, std::chrono::milliseconds( 50)));
  vec.push_back(dummy(exec, std::chrono::milliseconds(100000)));

  auto c = co_await select(vec);
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

TEST_SUITE_END();