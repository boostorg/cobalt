//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/op.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/promise.hpp>

#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/steady_timer.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

TEST_SUITE_BEGIN("op");

template<typename Timer>
struct test_wait_op : cobalt::op<system::error_code>
{
  Timer & tim;

  test_wait_op(Timer & tim) : tim(tim) {}

  void ready(cobalt::handler<system::error_code> h)
  {
    if (tim.expiry() < Timer::clock_type::now())
      h({});
  }
  void initiate(cobalt::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};

template<typename Timer>
struct test_wait_op_2 : cobalt::op<system::error_code>
{
  Timer & tim;

  test_wait_op_2(Timer & tim) : tim(tim) {}

  void ready(cobalt::handler<system::error_code> h)
  {
    if (tim.expiry() < Timer::clock_type::now())
      h(system::error_code(asio::error::operation_aborted));
  }
  void initiate(cobalt::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};


struct post_op : cobalt::op<>
{
  asio::any_io_executor exec;

  post_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(cobalt::completion_handler<> complete)
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

  (co_await cobalt::as_result(post_op(co_await asio::this_coro::executor))).value();
  (co_await cobalt::as_result(tim.async_wait(cobalt::use_op))).value();
}

struct op_throw_op
{
  template<typename Handler>
  void operator()(Handler &&)
  {
    throw std::runtime_error("test-exception");

  }
};

template<typename CompletionToken>
auto op_throw(CompletionToken&& token)
{
  return asio::async_initiate<CompletionToken, void(std::exception_ptr)>(
      op_throw_op{}, token);
}


TEST_CASE("op-throw")
{

  auto val = [&]() -> cobalt::task<void> {CHECK_THROWS(co_await op_throw(cobalt::use_op));};

  asio::io_context ctx;
  cobalt::this_thread::set_executor(ctx.get_executor());
  CHECK_NOTHROW(spawn(ctx, val(), asio::detached));

  CHECK_NOTHROW(ctx.run());
}

struct throw_op : cobalt::op<std::exception_ptr>
{
  asio::any_io_executor exec;

  throw_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(cobalt::completion_handler<std::exception_ptr> complete)
  {
    asio::post(exec, asio::append(std::move(complete), std::make_exception_ptr(std::runtime_error("test-exception"))));
  }
};


CO_TEST_CASE("exception-op")
{
  CHECK_THROWS(co_await throw_op(co_await asio::this_coro::executor));
}

struct initiate_op : cobalt::op<>
{
  asio::any_io_executor exec;

  initiate_op(asio::any_io_executor exec) : exec(exec) {}

  void initiate(cobalt::completion_handler<> complete)
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
  auto [ec] = co_await cobalt::as_tuple(chn.async_receive(cobalt::use_op));
  CHECK(!ec);

  CHECK(!called);
  co_await cobalt::as_tuple(asio::post(co_await asio::this_coro::executor, cobalt::use_op));
  CHECK(called);
}

struct test_async_initiate
{

  template<typename Handler>
  void operator()(Handler && h, std::shared_ptr<int> ptr)
  {
    CHECK(ptr);
    asio::dispatch(
        asio::get_associated_immediate_executor(
            h, asio::get_associated_executor(h)),
        std::move(h));
  }
};

template<typename Token>
auto test_cobalt(std::shared_ptr<int> & ptr, Token && token)
{
  return asio::async_initiate<Token, void()>(test_async_initiate{}, token, ptr);
}

CO_TEST_CASE("no-move-from")
{
  std::shared_ptr<int> p = std::make_shared<int>();
  CHECK(p);
  co_await test_cobalt(p, cobalt::use_op);
  CHECK(p);
}




TEST_SUITE_END();