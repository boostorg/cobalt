//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/op.hpp>
#include <boost/async/promise.hpp>
#include <boost/asio.hpp>
#include "doctest.h"
#include "test.hpp"

using namespace boost;

TEST_SUITE_BEGIN("op");

template<typename Timer>
struct test_wait_op : async::enable_op<test_wait_op<Timer>>
{
  Timer & tim;

  test_wait_op(Timer & tim) : tim(tim) {}

  bool ready(system::error_code & ) { return tim.expiry() < Timer::clock_type::now(); }
  void initiate(async::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};

template<typename Timer>
struct test_wait_op_2 : async::enable_op<test_wait_op_2<Timer>>
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


struct post_op : async::enable_op<post_op>
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

  auto val = [&]() -> async::promise<void> {co_await throw_;};

  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  spawn(ctx, val(), asio::detached);

  CHECK_THROWS(ctx.run());

}


async::test_case test_case_exception()
{

  auto throw_ = []<typename Token>(Token && tk)
  {
    throw std::runtime_error("test-exception");
    return boost::asio::post(std::forward<Token>(tk));
  };

  co_await throw_;
}

TEST_CASE("exception")
{
    CHECK_THROWS(run(test_case_exception()));
}


TEST_SUITE_END();