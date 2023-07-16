//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/wait_group.hpp>
#include <boost/async/promise.hpp>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

async::promise<void> gdelay(asio::any_io_executor exec,
                            std::chrono::milliseconds ms = std::chrono::milliseconds(25))
{
  if (ms.count() == 0u)
    co_return;
  if (ms == std::chrono::milliseconds ::max())
    throw std::runtime_error("wdummy_throw");

  asio::steady_timer tim{exec, ms};
  co_await tim.async_wait(async::use_op);
}


TEST_SUITE_BEGIN("wait_group");

CO_TEST_CASE("grp")
{
  auto e = co_await async::this_coro::executor;

  using std::chrono::operator""ms;

  async::wait_group wg;
  wg.push_back(gdelay(e));
  wg.push_back(gdelay(e, 0ms));
  wg.push_back(gdelay(e, 10ms));
  wg.push_back(gdelay(e, 20ms));

  co_await asio::post(e, async::use_op);
  CHECK(wg.size() == 4u);
  CHECK(wg.reap() == 1u);
  CHECK(wg.size() == 3u);

  co_await wg.wait_one();
  CHECK(wg.size() == 2u);

  wg.cancel();
  co_await wg;

  CHECK(wg.size() == 0u);
}


TEST_SUITE_END();

