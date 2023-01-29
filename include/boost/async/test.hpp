// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_TEST_HPP
#define BOOST_ASYNC_TEST_HPP

#include <boost/async/main.hpp>
#include <boost/async/detail/wrapper.hpp>
#include <boost/asio/bind_executor.hpp>

namespace boost::async
{

namespace detail { struct test_case_promise; }

template<typename CompletionToken>
auto async_run(struct test_case && tc, CompletionToken&& token);

struct test_case
{
  using promise_type = detail::test_case_promise;

  explicit test_case(detail::test_case_promise * promise = nullptr) : promise{promise} {}
  test_case(const test_case& ) = delete;
  test_case(test_case&&) noexcept = default;
  bool await_ready() const { return promise == nullptr; }
  std::coroutine_handle<detail::test_case_promise> await_suspend(std::coroutine_handle<void> h);

  void await_resume()
  {
  }

 private:
  template<typename CompletionToken>
  friend auto async_run(struct test_case&& tc, CompletionToken&& token);
  mutable std::unique_ptr<detail::test_case_promise, async::detail::coro_deleter<detail::test_case_promise>> promise{nullptr};
};

namespace detail
{

struct test_case_promise : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                           promise_throw_if_cancelled_base,
                           async::detail::enable_awaitables<test_case_promise>,
                           async::detail::enable_async_operation
{
    using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using enable_awaitables<test_case_promise>::await_transform;
    using enable_async_operation::await_transform;

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
          return promise->awaited_from.address() != nullptr;
        }

        auto await_suspend(std::coroutine_handle<test_case_promise> h) noexcept -> std::coroutine_handle<void>
        {
          std::coroutine_handle<void> res = std::noop_coroutine();
          if (promise->awaited_from.address() != nullptr)
            res = std::exchange(promise->awaited_from, nullptr);

          h.destroy();
          return res;
        }

        void await_resume() noexcept
        {
        }
      };
      return final_awaitable{this};
    }

    async::test_case get_return_object()
    {
      return async::test_case{this};
    }

    void unhandled_exception() {throw;}
    void return_void() {}

    using executor_type = asio::io_context::executor_type;
    executor_type get_executor() const {return exec->context().get_executor();}

    using allocator_type = container::pmr::polymorphic_allocator<void>;
    allocator_type get_allocator() const {return container::pmr::polymorphic_allocator<void>{this_thread::get_default_resource()};}

    auto await_transform(this_coro::executor_t) const
    {
        struct exec_helper
        {
            executor_type value;

            constexpr static bool await_ready() noexcept
            {
                return true;
            }

            constexpr static void await_suspend(std::coroutine_handle<>) noexcept
            {
            }

            executor_type await_resume() const noexcept
            {
                return value;
            }
        };

        return exec_helper{get_executor()};
    }
    ~test_case_promise()
    {
      if (awaited_from != nullptr)
          awaited_from.destroy();
    }

    std::optional<typename asio::require_result<executor_type, asio::execution::outstanding_work_t ::tracked_t>::type> exec;
    std::coroutine_handle<void> awaited_from{nullptr};
};

}

template<typename CompletionToken>
auto async_run(test_case&& tc, CompletionToken&& token)
{
  return asio::async_initiate<CompletionToken, void()>(
      [](auto && h, test_case tc)
      {
        auto exec = asio::get_associated_executor(h);
        tc.promise->exec = boost::asio::require(exec, boost::asio::execution::outstanding_work.tracked);
        tc.promise->awaited_from = async::detail::dispatch_coroutine(std::move(h));

        asio::post(
            exec,
            [p = std::move(tc.promise)]() mutable
            {
              std::coroutine_handle<detail::test_case_promise>::from_promise(*p.release()).resume();
            });
      }, token, std::move(tc));
}

template<typename CompletionToken>
auto async_run(asio::io_context & context, test_case&& t, CompletionToken&& token)
{
    return async_run(std::move(t), asio::bind_executor(context.get_executor(), std::forward<CompletionToken>(token)));
}

template<typename Executor, typename CompletionToken>
    requires (asio::is_convertible<Executor, asio::io_context::executor_type>::value)
auto async_run(Executor executor,
         test_case&& t,
         CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(Executor))
{
    return async_run(std::move(t), asio::bind_executor(executor, std::forward<CompletionToken>(token)));
}

void run(test_case&& tc);

}


#endif //BOOST_ASYNC_TEST_HPP
