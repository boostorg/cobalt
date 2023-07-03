//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/eager_op.hpp>
#include <boost/async/spawn.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/select.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

TEST_SUITE_BEGIN("eager_op");

template<typename Timer>
struct test_wait_eager_op : async::eager_op<system::error_code>
{
  Timer & tim;

  test_wait_eager_op(Timer & tim) : tim(tim) {}

  void initiate(async::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};


CO_TEST_CASE("eager_op")
{

  asio::steady_timer tim1{co_await asio::this_coro::executor, std::chrono::milliseconds(10)};
  asio::steady_timer tim2{co_await asio::this_coro::executor, std::chrono::milliseconds(20)};
  asio::steady_timer tim3{co_await asio::this_coro::executor, std::chrono::milliseconds(30)};


  test_wait_eager_op t1{tim1}, t2{tim2}, t3{tim3};
  CHECK((co_await async::select(t1, t2, t3)) == 0u);
  CHECK((co_await async::select(t2, t3))     == 0u);
  CHECK((co_await async::select(t3))         == 0u);
}

TEST_CASE("eager_op-throw")
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

struct throw_eager_op : async::eager_op<std::exception_ptr>
{
  asio::any_io_executor exec;

  throw_eager_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(async::completion_handler<std::exception_ptr> complete)
  {
    asio::post(exec, asio::append(std::move(complete), std::make_exception_ptr(std::runtime_error("test-exception"))));
  }
};

CO_TEST_CASE("exception-eager_op")
{
  CHECK_THROWS(co_await throw_eager_op(co_await asio::this_coro::executor));
}

struct initiate_eager_op : async::eager_op<>
{
  asio::any_io_executor exec;

  initiate_eager_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(async::completion_handler<> complete)
  {
    throw std::runtime_error("test-exception");
    asio::post(exec, std::move(complete));
  }
};

CO_TEST_CASE("initiate-exception-eager_op")
{
  CHECK_THROWS(co_await throw_eager_op(co_await asio::this_coro::executor));
}

CO_TEST_CASE("immediate_executor")
{
  auto called = false;
  asio::post(co_await asio::this_coro::executor, [&]{called = true;});
  asio::experimental::channel<void(system::error_code)> chn{co_await asio::this_coro::executor, 2u};
  CHECK(chn.try_send(system::error_code()));


  co_await chn.async_receive(async::use_eager_op);

  CHECK(!called);
  co_await asio::post(co_await asio::this_coro::executor, async::use_eager_op);
  CHECK(called);
}

struct interruptible_handler
{
  asio::steady_timer & tim;

  std::optional<std::tuple<system::error_code>> result;
  using ct = async::completion_handler<system::error_code>;

  bool completed = false;
  ct * ct_ = nullptr;
  interruptible_handler(asio::steady_timer & tim) : tim(tim)
  {
    tim.async_wait(ct{std::noop_coroutine(), result, &completed, &ct_});
  }

  interruptible_handler(interruptible_handler && lhs)
      : tim(lhs.tim), completed(lhs.completed), ct_(lhs.ct_)
  {
    if (lhs.ct_)
    {
      ct_->result = &result;
      ct_->completed_immediately = &completed;
    }
  }

  bool await_ready() { return completed; }

  bool await_suspend(std::coroutine_handle<void> h)
  {
    return completed;
  }

  void await_resume()
  {

  }

};

CO_TEST_CASE("interruptible_handler")
{
  auto called = false;
  asio::post(co_await asio::this_coro::executor, [&]{called = true;});
  asio::experimental::channel<void(system::error_code)> chn{co_await asio::this_coro::executor, 2u};
  CHECK(chn.try_send(system::error_code()));


  co_await chn.async_receive(async::use_eager_op);

  CHECK(!called);
  co_await asio::post(co_await asio::this_coro::executor, async::use_eager_op);
  CHECK(called);
}





TEST_SUITE_END();