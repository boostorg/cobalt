// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_TEST_HPP
#define BOOST_ASYNC_TEST_HPP

#include <boost/async/detail/test.hpp>
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
