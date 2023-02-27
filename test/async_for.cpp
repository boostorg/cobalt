// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/async_for.hpp>
#include <boost/async/generator.hpp>
#include "doctest.h"
#include "test.hpp"

using namespace boost;

std::vector<int> test_data = { 1,2,3,4,5,6,7,8,9,0};

async::generator<int> test_gen()
{
  for (auto & td : test_data)
  {
    if (&td == &test_data.back())
      co_return td;
    else
      co_yield td;
  }
}

CO_TEST_CASE("async_for")
{
  auto itr = test_data.begin();
  BOOST_ASYNC_FOR(auto i, test_gen())
  {
    CHECK(i == *itr++);
  }
}