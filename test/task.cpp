//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/task.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/join.hpp>

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

cobalt::task<void> test0()
{
    co_return;
}

cobalt::task<double> test2(int i)
{
    co_await test0();
    co_return i;
}

cobalt::task<int> test1()
{
    co_await test2(42);
    co_await test2(42);


    co_await asio::post(co_await cobalt::this_coro::executor, cobalt::use_task);
    co_return 452;
}
}

TEST_CASE("test-1")
{

    bool done = false;

    asio::io_context ctx;
    asio::steady_timer tim{ctx};
    cobalt::this_thread::set_executor(ctx.get_executor());

    cobalt::spawn(
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

CO_TEST_CASE("cobalt-1")
{
    co_await test1();
    co_return;
}


static cobalt::task<void> should_unwind(asio::io_context & ctx)
{
  co_await asio::post(ctx, cobalt::use_op);
}

TEST_CASE("unwind")
{
  asio::io_context ctx;
  boost::cobalt::this_thread::set_executor(ctx.get_executor());
  boost::ignore_unused(should_unwind(ctx));
}
namespace
{


cobalt::task<int> return_([[maybe_unused]] std::size_t ms,
                         asio::executor_arg_t, boost::cobalt::executor )
{
  co_return 1234u;
}

cobalt::task<void> delay_r(asio::io_context &ctx, std::size_t ms)
{
  auto tim = cobalt::use_op.as_default_on(asio::steady_timer(ctx, std::chrono::milliseconds{ms}));
  co_await tim.async_wait();
}

cobalt::task<void> throw_()
{
  throw std::runtime_error("throw_");
  co_return ;
}

cobalt::task<void> throw_post()
{
  co_await asio::post(cobalt::this_thread::get_executor(), cobalt::use_op);
  throw std::runtime_error("throw_");
  co_return ;
}

}


TEST_CASE("cancel-void")
{
  asio::io_context ctx;
  cobalt::this_thread::set_executor(ctx.get_executor());
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

static cobalt::task<void> delay_v(asio::io_context &ctx, std::size_t ms)
{
  asio::steady_timer tim(ctx, std::chrono::milliseconds{ms});
  co_await tim.async_wait(cobalt::use_op);
}


TEST_CASE("cancel-int")
{
  asio::io_context ctx;
  cobalt::this_thread::set_executor(ctx.get_executor());
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
  cobalt::this_thread::set_executor(ctx.get_executor());
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
  cobalt::this_thread::set_executor(ctx.get_executor());
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
          []() -> cobalt::task<int>
          {
            co_await stop();
            co_return 42;
          }());
}


cobalt::task<void> throw_if_test(asio::cancellation_signal & sig)
{

  CHECK(co_await cobalt::this_coro::cancelled
        == asio::cancellation_type::none);
  sig.emit(asio::cancellation_type::terminal);
  CHECK(co_await cobalt::this_coro::cancelled
        == asio::cancellation_type::terminal);
  CHECK_THROWS(co_await asio::post(cobalt::use_op));
}


TEST_CASE("throw_if_cancelled")
{
  asio::cancellation_signal sig;

  asio::io_context ctx;
  boost::cobalt::this_thread::set_executor(ctx.get_executor());
  cobalt::spawn(ctx, throw_if_test(sig),
               asio::bind_cancellation_slot(sig.slot(), asio::detached));
  ctx.run();
}

CO_TEST_CASE("reawait")
{
  auto t = test0();
  co_await std::move(t);
  CHECK_NOTHROW(co_await std::move(t));
}


cobalt::task<int> test_strand1(asio::any_io_executor exec)
{
  REQUIRE(exec == co_await cobalt::this_coro::executor);
  co_await asio::post(co_await cobalt::this_coro::executor, cobalt::use_task);
  co_return 31;
}

cobalt::task<void> test_strand()
{
  auto e = co_await cobalt::this_coro::executor;
  co_await cobalt::join(test_strand1(e), test_strand1(e), test_strand1(e), test_strand1(e));
}

#if !defined(BOOST_COBALT_USE_IO_CONTEXT)

TEST_CASE("stranded")
{

  asio::thread_pool ctx;
  boost::cobalt::this_thread::set_executor(ctx.get_executor());
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  cobalt::spawn(asio::make_strand(ctx.get_executor()), test_strand(),[](std::exception_ptr ep) { CHECK(!ep); });
  ctx.join();
}

#endif

TEST_SUITE_END();