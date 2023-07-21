// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/detail/fork.hpp>


#include <boost/async/config.hpp>
using namespace boost;

#include "doctest.h"
#include "test.hpp"

struct tester : async::detail::fork::shared_state
{
  char buf[4096];

  tester() : async::detail::fork::shared_state{buf, 4096} {}

  static async::detail::fork step(tester & , int i= 0)
  {
    if (i == 42)
      co_await async::detail::fork::wired_up;
    co_return;
  }
};

CO_TEST_CASE("fork")
{
  tester t;
  auto x = tester::step(t);
  CHECK(x.done());

  x = tester::step(t, 42);

  CHECK(!x.done());
  co_return;
}