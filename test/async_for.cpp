// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/async_for.hpp>
#include <boost/async/generator.hpp>
#include "doctest.h"
#include "test.hpp"

using namespace boost;

std::array<int, 10> test_data = {1,2,3,4,5,6,7,8,9,0};

async::generator<int> test_data_gen()
{
  for (auto & td : test_data)
  {
    if (&td == &test_data.back())
      co_return td;
    else
      co_yield td;
  }
}

async::generator<int> once_gen()
{
  co_return 0;
}

async::generator<int> throw_gen()
{
  co_yield 42;
  throw std::runtime_error("throw_gen");
  co_return 0;
}

TEST_SUITE_BEGIN("async_for");

/// If the awaitable is not empty the loop must be entered for every value exactly once
CO_TEST_CASE("empty_awaitable")
{
  auto tg = once_gen();
  co_await tg;

  /// also[lvalue]: The iterated expression can be an lvalue
  BOOST_ASYNC_FOR(auto i, tg)
  {
    CHECK(false);
  }
}

/// If the awaitable is not empty the loop must be entered for every value exactly once
CO_TEST_CASE("regular")
{
  auto itr = test_data.begin();
  /// also[rvalue]: The iterated expression can be an rvalue
  BOOST_ASYNC_FOR(auto i, test_data_gen())
    {
      CHECK(i == *itr++);
    }
}

/// Any exception thrown from the co_await must be propagated.
CO_TEST_CASE("exception")
{
  auto inner = []() -> async::generator<int>
  {
    BOOST_ASYNC_FOR(auto i, throw_gen()); // should throw
  };

  CHECK_THROWS(co_await inner());
}