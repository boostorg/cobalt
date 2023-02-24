//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_TEST_HPP
#define BOOST_ASYNC_DETAIL_TEST_HPP

#include <boost/async/detail/async_operation.hpp>
#include <boost/async/concepts.hpp>

namespace boost::async { struct test_case;}

namespace boost::async::detail
{

struct test_case_promise : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                           promise_throw_if_cancelled_base,
                           async::enable_awaitables<test_case_promise>,
                           async::detail::enable_async_operation,
                           enable_await_executor<test_case_promise>
{
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<test_case_promise>::await_transform;
  using enable_async_operation::await_transform;
  using enable_await_executor<test_case_promise>::await_transform;

  std::suspend_always initial_suspend()
  {
    return {};
  }
  auto final_suspend() noexcept
  {

    struct final_awaitable
    {
      test_case_promise * promise;
      bool await_ready() const noexcept
      {
        return promise->awaited_from.get() != nullptr;
      }

      auto await_suspend(std::coroutine_handle<test_case_promise> h) noexcept -> std::coroutine_handle<void>
      {
        std::coroutine_handle<void> res = std::noop_coroutine();
        if (promise->awaited_from.get() != nullptr)
          res = std::coroutine_handle<void>::from_address(promise->awaited_from.release());

        h.destroy();
        return res;
      }

      void await_resume() noexcept
      {
      }
    };
    return final_awaitable{this};
  }

  async::test_case get_return_object();

  void unhandled_exception() {throw;}
  void return_void() {}

  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() const {return exec->context().get_executor();}

  using allocator_type = container::pmr::polymorphic_allocator<void>;
  allocator_type get_allocator() const {return container::pmr::polymorphic_allocator<void>{this_thread::get_default_resource()};}

  std::optional<typename asio::require_result<executor_type, asio::execution::outstanding_work_t ::tracked_t>::type> exec;
  std::unique_ptr<void, detail::coro_deleter<>>  awaited_from{nullptr};
};


}

#endif //BOOST_ASYNC_DETAIL_TEST_HPP
