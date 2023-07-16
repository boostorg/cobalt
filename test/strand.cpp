// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context_strand.hpp>

#if defined(BOOST_ASYNC_USE_BOOST_CONTAINER_PMR)
#include <boost/container/pmr/synchronized_pool_resource.hpp>
#endif

#include "doctest.h"
#include "test.hpp"

using namespace boost;


TEST_SUITE_BEGIN("strand");

async::promise<void> do_the_thing()
{
  static std::atomic<int> unique = 0;

  asio::steady_timer tim{co_await async::this_coro::executor, std::chrono::milliseconds(50)};
  co_await tim.async_wait(asio::deferred);


  unique++;
  CHECK(unique == 1);
  unique--;
}

TEST_CASE("strand")
{
  std::vector<std::thread> ths;

  asio::io_context ctx;
  asio::any_io_executor exec{asio::make_strand(ctx.get_executor())};
  async::pmr::synchronized_pool_resource sync;

  for (int i = 0; i < 8; i++)
    ths.push_back(
        std::thread{
          [&]
          {
            async::this_thread::set_default_resource(&sync);
            async::this_thread::set_executor(exec);
            +do_the_thing();
            ctx.run();
          }});


  for (auto & th : ths)
    th.join();


}

TEST_SUITE_END();
