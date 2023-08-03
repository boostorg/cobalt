//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/task.hpp>
#include <boost/async/main.hpp>
#include <boost/async/op.hpp>
#include <boost/async/spawn.hpp>
#include <boost/async/join.hpp>

#include "doctest.h"
#include "test.hpp"
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/thread_pool.hpp>

#include <boost/core/ignore_unused.hpp>

#include <new>

using namespace boost;

TEST_SUITE_BEGIN("task");

namespace
{

async::task<void> test0()
{
    co_return;
}

async::task<double> test2(int i)
{
    co_await test0();
    co_return i;
}


async::task<int> test1()
{
    co_await test2(42);
    co_await test2(42);


    co_await asio::post(co_await async::this_coro::executor, async::use_task);
    co_return 452;
}
}

TEST_CASE("test-1")
{

    bool done = false;

    asio::io_context ctx;
    asio::steady_timer tim{ctx};
    async::this_thread::set_executor(ctx.get_executor());

    async::spawn(
          ctx.get_executor(),
          test1(),
          [&](std::exception_ptr ex, int res)
          {
            CHECK(ex == nullptr);
            CHECK(res == 452);
            done = true;
          });

    ctx.run();
    CHECK(done);
}

CO_TEST_CASE("async-1")
{
    co_await test1();
    co_return;
}


static async::task<void> should_unwind(asio::io_context & ctx)
{
  co_await asio::post(ctx, async::use_op);
}

TEST_CASE("unwind")
{
  asio::io_context ctx;
  boost::async::this_thread::set_executor(ctx.get_executor());
  boost::ignore_unused(should_unwind(ctx));
}
namespace
{

async::task<int> return_(std::size_t ms)
{
  co_return 1234u;
}

async::task<int> return_(std::size_t ms, asio::executor_arg_t,
                         boost::async::executor )
{
  co_return 1234u;
}

async::task<void> delay_r(asio::io_context &ctx, std::size_t ms,
                                    asio::executor_arg_t, asio::any_io_executor )
{
  auto tim = async::use_op.as_default_on(asio::steady_timer(ctx, std::chrono::milliseconds{ms}));
  co_await tim.async_wait();
}


async::task<void> delay_r(asio::io_context &ctx, std::size_t ms)
{
  auto tim = async::use_op.as_default_on(asio::steady_timer(ctx, std::chrono::milliseconds{ms}));
  co_await tim.async_wait();
}

async::task<void> throw_()
{
  throw std::runtime_error("throw_");
  co_return ;
}

async::task<void> throw_post()
{
  co_await asio::post(async::this_thread::get_executor(), async::use_op);
  throw std::runtime_error("throw_");
  co_return ;
}

}


TEST_CASE("cancel-void")
{
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  asio::cancellation_signal signal;


  spawn(ctx, delay_r(ctx, 10000u), asio::bind_cancellation_slot(
      signal.slot(),
      [](std::exception_ptr ep)
      {
        CHECK(ep != nullptr);
      }));

  asio::post(ctx, [&]{signal.emit(asio::cancellation_type::all);});

  spawn(ctx, return_(1234u, asio::executor_arg, ctx.get_executor()),
        [](std::exception_ptr ep, std::size_t n)
        {
          CHECK(ep == nullptr);
          CHECK(n == 1234u);
        });


  ctx.run();
}

static async::task<void> delay_v(asio::io_context &ctx, std::size_t ms)
{
  asio::steady_timer tim(ctx, std::chrono::milliseconds{ms});
  co_await tim.async_wait(async::use_op);
}


TEST_CASE("cancel-int")
{
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  asio::cancellation_signal signal;

  spawn(ctx, delay_v(ctx, 10000u), asio::bind_cancellation_slot(
      signal.slot(),
      [](std::exception_ptr ep)
      {
        CHECK(ep != nullptr);
      }));

  asio::post(ctx, [&]{signal.emit(asio::cancellation_type::all);});
  spawn(ctx, throw_(),
          [](std::exception_ptr ep)
          {
            CHECK(ep != nullptr);
          });


  ctx.run();
}


TEST_CASE("throw-cpl")
{
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  asio::cancellation_signal signal;

  spawn(ctx, throw_(),
        [](std::exception_ptr ep)
        {
          std::rethrow_exception(ep);
        });


  CHECK_THROWS(ctx.run());
}

TEST_CASE("throw-cpl-delay")
{
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  asio::cancellation_signal signal;

  spawn(ctx, throw_post(),
        [](std::exception_ptr ep)
        {
          std::rethrow_exception(ep);
        });


  CHECK_THROWS(ctx.run());
}


CO_TEST_CASE("stop")
{
  CHECK_THROWS(
      co_await
          []() -> async::task<int>
          {
            co_await stop();
            co_return 42;
          }());
}


async::task<void> throw_if_test(asio::cancellation_signal & sig)
{

  CHECK(co_await async::this_coro::cancelled
        == asio::cancellation_type::none);
  sig.emit(asio::cancellation_type::terminal);
  CHECK(co_await async::this_coro::cancelled
        == asio::cancellation_type::terminal);
  CHECK_THROWS(co_await asio::post(async::use_op));
}


TEST_CASE("throw_if_cancelled")
{
  asio::cancellation_signal sig;

  asio::io_context ctx;
  boost::async::this_thread::set_executor(ctx.get_executor());
  async::spawn(ctx, throw_if_test(sig),
               asio::bind_cancellation_slot(sig.slot(), asio::detached));
  ctx.run();
}

CO_TEST_CASE("reawait")
{
  auto t = test0();
  co_await std::move(t);
  CHECK_NOTHROW(co_await std::move(t));
}


async::task<void> test_strand()
{
  co_await async::join(test1(), test1(), test1(), test1());
}

TEST_CASE("stranded")
{

  asio::thread_pool ctx;
  boost::async::this_thread::set_executor(ctx.get_executor());
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  async::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  ctx.join();
}


TEST_SUITE_END();