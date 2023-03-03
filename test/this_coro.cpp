// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/this_coro.hpp>
#include <boost/async/detail/util.hpp>

#include <boost/container/pmr/unsynchronized_pool_resource.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

struct sig_helper
{
  asio::cancellation_signal sig;
};

struct coro_feature_tester
      : sig_helper,
        async::promise_cancellation_base<>,
        async::promise_throw_if_cancelled_base,
        async::enable_await_allocator<coro_feature_tester>
{
  using async::promise_cancellation_base<>::await_transform;
  using async::promise_throw_if_cancelled_base::await_transform;
  using async::enable_await_allocator<coro_feature_tester>::await_transform;

  std::suspend_never initial_suspend() {return {};}
  std::suspend_never final_suspend() noexcept {return {};}

  coro_feature_tester(coro_feature_tester * & ref)
    : async::promise_cancellation_base<>(sig.slot())
  {
    ref = this;
  }
  void return_void() {}
  void unhandled_exception() {throw;}
  void get_return_object() {}

  container::pmr::unsynchronized_pool_resource res;
  using allocator_type = container::pmr::polymorphic_allocator<void>;
  allocator_type get_allocator() {return alloc;}
  allocator_type alloc{&res};
};

namespace std
{
template<>
struct coroutine_traits<void, coro_feature_tester*> //< don't do THIS at
{
  using promise_type = coro_feature_tester;
};
}

#define SELF_TEST_CASE_IMPL(Function, ...)                                                                         \
static void Function(coro_feature_tester * this_);                                                                 \
DOCTEST_TEST_CASE(__VA_ARGS__)                                                                                     \
{                                                                                                                  \
    Function(nullptr);                                                                                             \
}                                                                                                                  \
static void Function(coro_feature_tester * this_)

#define SELF_TEST_CASE(...) SELF_TEST_CASE_IMPL(DOCTEST_ANONYMOUS(SELF_DOCTEST_ANON_FUNC_), __VA_ARGS__)

TEST_SUITE_BEGIN("this_coro");

SELF_TEST_CASE("promise_cancellation_base")
{
  CHECK(!this_->cancelled());
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::none);

  this_->sig.emit(asio::cancellation_type::terminal);

  CHECK(this_->cancelled() == asio::cancellation_type::terminal);
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::terminal);

  co_await async::this_coro::reset_cancellation_state();

  CHECK(!this_->cancelled());
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::none);

  this_->sig.emit(asio::cancellation_type::terminal);

  CHECK(this_->cancelled() == asio::cancellation_type::terminal);
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::terminal);

  co_await async::this_coro::reset_cancellation_state(asio::enable_partial_cancellation());

  CHECK(!this_->cancelled());
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::none);

  this_->sig.emit(asio::cancellation_type::total);

  CHECK(!this_->cancelled());
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::none);
  this_->sig.emit(asio::cancellation_type::all);
  CHECK(this_->cancelled() == (asio::cancellation_type::terminal | asio::cancellation_type::partial));


  co_await async::this_coro::reset_cancellation_state(
      asio::enable_partial_cancellation(),
      asio::enable_terminal_cancellation());

  CHECK(!this_->cancelled());
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::none);

  this_->sig.emit(asio::cancellation_type::total);

  CHECK(!this_->cancelled());
  CHECK(this_->cancellation_state().cancelled() == asio::cancellation_type::none);
  this_->sig.emit(asio::cancellation_type::all);
  CHECK(this_->cancelled() == (asio::cancellation_type::terminal | asio::cancellation_type::partial));
}


SELF_TEST_CASE("promise_throw_if_cancelled_base")
{
  CHECK(co_await asio::this_coro::throw_if_cancelled());
  co_await asio::this_coro::throw_if_cancelled(false);
  CHECK(!co_await asio::this_coro::throw_if_cancelled());
}


SELF_TEST_CASE("enable_await_allocator")
{
  CHECK(this_->get_allocator() == co_await async::this_coro::allocator);
}


TEST_SUITE_END();