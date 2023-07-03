//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/op.hpp>
#include <boost/async/spawn.hpp>
#include <boost/async/promise.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

TEST_SUITE_BEGIN("op");

template<typename Timer>
struct test_wait_op : async::op<system::error_code>
{
  Timer & tim;

  test_wait_op(Timer & tim) : tim(tim) {}

  void ready(async::handler<system::error_code> h)
  {
    if (tim.expiry() < Timer::clock_type::now())
      h({});
  }
  void initiate(async::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};

template<typename Timer>
struct test_wait_op_2 : async::op<system::error_code>
{
  Timer & tim;

  test_wait_op_2(Timer & tim) : tim(tim) {}

  void ready(async::handler<system::error_code> h)
  {
    if (tim.expiry() < Timer::clock_type::now())
      h(system::error_code(asio::error::operation_aborted));
  }
  void initiate(async::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};


struct post_op : async::op<>
{
  asio::any_io_executor exec;

  post_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(async::completion_handler<> complete)
  {
    asio::post(std::move(complete));
  }
};


CO_TEST_CASE("op")
{

  asio::steady_timer tim{co_await asio::this_coro::executor, std::chrono::milliseconds(10)};

  co_await test_wait_op{tim};
  co_await test_wait_op{tim};

  tim.expires_after(std::chrono::milliseconds(10));

  co_await test_wait_op_2{tim};
  CHECK_THROWS(co_await test_wait_op_2{tim});

  co_await post_op(co_await asio::this_coro::executor);
  co_await tim.async_wait(async::use_op);
}

TEST_CASE("op-throw")
{
  auto throw_ = []<typename Token>(Token && tk)
  {
    throw std::runtime_error("test-exception");
    return boost::asio::post(std::forward<Token>(tk));
  };

  auto val = [&]() -> async::task<void> {co_await throw_;};

  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  spawn(ctx, val(), asio::detached);

  CHECK_THROWS(ctx.run());
}

struct throw_op : async::op<std::exception_ptr>
{
  asio::any_io_executor exec;

  throw_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(async::completion_handler<std::exception_ptr> complete)
  {
    asio::post(exec, asio::append(std::move(complete), std::make_exception_ptr(std::runtime_error("test-exception"))));
  }
};


CO_TEST_CASE("exception-op")
{
  CHECK_THROWS(co_await throw_op(co_await asio::this_coro::executor));
}

struct initiate_op : async::op<>
{
  asio::any_io_executor exec;

  initiate_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(async::completion_handler<> complete)
  {
    throw std::runtime_error("test-exception");
    asio::post(exec, std::move(complete));
  }
};


CO_TEST_CASE("initiate-exception-op")
{
  CHECK_THROWS(co_await throw_op(co_await asio::this_coro::executor));
}

CO_TEST_CASE("immediate_executor")
{
  auto called = false;
  asio::post(co_await asio::this_coro::executor, [&]{called = true;});
  asio::experimental::channel<void(system::error_code)> chn{co_await asio::this_coro::executor, 2u};
  CHECK(chn.try_send(system::error_code()));


  co_await chn.async_receive(async::use_op);

  CHECK(!called);
  co_await asio::post(co_await asio::this_coro::executor, async::use_op);
  CHECK(called);
}




TEST_SUITE_END();