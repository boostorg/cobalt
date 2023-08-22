//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_DETACHED_HPP
#define BOOST_ASYNC_DETAIL_DETACHED_HPP

#include <boost/async/detail/exception.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/wrapper.hpp>
#include <boost/async/detail/this_thread.hpp>

#include <boost/asio/cancellation_signal.hpp>




#include <boost/core/exchange.hpp>

#include <coroutine>
#include <optional>
#include <utility>
#include <boost/asio/bind_allocator.hpp>

namespace boost::async
{

struct detached;

namespace detail
{

struct detached_promise
    : promise_memory_resource_base,
      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
      promise_throw_if_cancelled_base,
      enable_awaitables<detached_promise>,
      enable_await_allocator<detached_promise>,
      enable_await_executor<detached_promise>
{
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<detached_promise>::await_transform;
  using enable_await_allocator<detached_promise>::await_transform;
  using enable_await_executor<detached_promise>::await_transform;

  [[nodiscard]] detached get_return_object();

  auto await_transform(
      async::this_coro::reset_cancellation_source_t<asio::cancellation_slot> reset) noexcept
  {
    struct result
    {
      detached_promise * promise;
      asio::cancellation_slot slot;

      constexpr bool await_ready() const noexcept
      {
        return true;
      }

      void await_suspend(std::coroutine_handle<void>) noexcept
      {
      }

      auto await_resume()
      {
        promise->reset_cancellation_source(std::move(slot));
      }
    };

    return result{this, std::move(reset.source)};
  }

  using executor_type = executor;
  executor_type exec;
  const executor_type & get_executor() const {return exec;}

  template<typename ... Args>
  detached_promise(Args & ...args)
      : promise_memory_resource_base(detail::get_memory_resource_from_args(args...)),
        exec{detail::get_executor_from_args(args...)}
  {
  }

  std::suspend_never initial_suspend()        {return {};}
  std::suspend_never final_suspend() noexcept {return {};}

  void return_void() {}
  void unhandled_exception()
  {
    throw ;
  }
};

}

}

#endif //BOOST_ASYNC_DETAIL_DETACHED_HPP
