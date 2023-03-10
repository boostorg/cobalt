// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/run.hpp>

#include "doctest.h"

using namespace boost;

TEST_SUITE_BEGIN("run");

TEST_CASE("run")
{
  CHECK(42 == run([]() -> async::task<int> {co_return 42;}()));

  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  CHECK(42 == run([]() -> async::task<int> {co_return 42;}()));

  CHECK(ctx.get_executor() == async::this_thread::get_executor());
}

TEST_SUITE_END();
