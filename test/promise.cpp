//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/promise.hpp>
#include <boost/async/op.hpp>

#include "doctest.h"
#include "test.hpp"
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/awaitable.hpp>

#include <boost/asio/bind_cancellation_slot.hpp>

using namespace boost;

TEST_SUITE_BEGIN("promise");

async::promise<void> test0()
{
    co_return;
}

async::promise<double> test2(int i)
{
    co_await test0();
    co_return i;
}


async::promise<int> test1(asio::any_io_executor exec)
{
    co_await test2(42);
    co_await test2(42);

    CHECK(test2(42).get() == 42);

    co_await asio::post(exec, asio::deferred);
    co_return 452;
}


CO_TEST_CASE("async-1")
{
    co_await test1(co_await asio::this_coro::executor);
    co_return;
}


async::promise<void> should_unwind(asio::io_context & ctx)
{
  co_await asio::post(ctx, async::use_op);
}

TEST_CASE("unwind")
{
  asio::io_context ctx;
  boost::async::this_thread::set_executor(ctx.get_executor());
  +should_unwind(ctx);
}

async::promise<int> return_(std::size_t ms)
{
  co_return 1234u;
}

async::promise<int> return_(std::size_t ms, asio::executor_arg_t,
                            boost::async::executor_type )
{
  co_return 1234u;
}

async::promise<std::size_t> delay_r(asio::io_context &ctx, std::size_t ms,
                                    asio::executor_arg_t, asio::any_io_executor )
{
  auto tim = async::use_op.as_default_on(asio::steady_timer(ctx, std::chrono::milliseconds{ms}));
  co_await tim.async_wait();
  co_return ms;
}


async::promise<std::size_t> delay_r(asio::io_context &ctx, std::size_t ms)
{
   auto tim = async::use_op.as_default_on(asio::steady_timer(ctx, std::chrono::milliseconds{ms}));
  co_await tim.async_wait();
  co_return ms;
}

async::promise<void> throw_()
{
  throw std::runtime_error("throw_");
  co_return ;
}

async::promise<void> throw_post()
{
  co_await asio::post(async::this_thread::get_executor(), asio::deferred);
  throw std::runtime_error("throw_");
  co_return ;
}


CO_TEST_CASE("get")
{
  auto r = delay_r((co_await async::this_coro::executor).context(), 100);
  CHECK_THROWS(r.get());
  CHECK_THROWS(throw_().get());

  co_await r;

}


async::promise<void> delay_v(asio::io_context &ctx, std::size_t ms)
{
  asio::steady_timer tim(ctx, std::chrono::milliseconds{ms});
  co_await tim.async_wait(asio::deferred);
}


CO_TEST_CASE("cancel-int")
{
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  asio::cancellation_signal signal;

  CHECK_THROWS(co_await throw_());


  ctx.run();
}



CO_TEST_CASE("throw-cpl-delay")
{
  CHECK_THROWS(co_await throw_post());
}

CO_TEST_CASE("stop")
{
  CHECK_THROWS(
    co_await
        []() -> async::promise<void>
        {
          co_await stop();
        }());
}

TEST_SUITE_END();