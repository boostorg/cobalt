//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/generator.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/select.hpp>

#include <boost/asio.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


TEST_SUITE_BEGIN("generator");

async::generator<int> gen()
{
  asio::steady_timer  tim{async::this_thread::get_executor()};
  for (int i = 0; i <10; i ++)
    co_yield i;

  co_return 10;
}

CO_TEST_CASE("generator-int")
{
  auto g = gen();

  int i = 0;
  while (g)
    CHECK(i ++ == co_await g);


  CHECK(i == 11);

  co_return ;
}

TEST_SUITE_END();