//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_THREAD_HPP
#define BOOST_ASYNC_THREAD_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/signal_set.hpp>

#include <thread>

#include "boost/async/detail/concepts.hpp"
#include "boost/async/detail/async_operation.hpp"
#include "boost/async/detail/forward_cancellation.hpp"
#include <boost/async/this_coro.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>


namespace boost::async
{

namespace detail
{
struct thread_promise;
}

struct thread;

namespace detail
{


struct signal_helper_2
{
  asio::cancellation_signal signal;
};

struct thread_promise : signal_helper_2,
                        promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                        promise_throw_if_cancelled_base,
                        enable_awaitables<thread_promise>,
                        enable_async_operation,
                        enable_await_allocator<thread_promise>
{
  thread_promise();

  std::suspend_always initial_suspend() {return {};}
  std::suspend_never    final_suspend() noexcept { return {};}

  void unhandled_exception() { throw ; }
  void return_void()
  {
  }

  void run();

  using executor_type = typename asio::io_context::executor_type;
  executor_type get_executor() const {return exec->get_executor();}

  using allocator_type = container::pmr::polymorphic_allocator<void>;
  using resource_type  = container::pmr::unsynchronized_pool_resource;

  mutable resource_type resource;
  allocator_type  get_allocator() const { return allocator_type(&resource); }

  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<thread_promise>::await_transform;
  using enable_async_operation::await_transform;
  using enable_await_allocator<thread_promise>::await_transform;

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

  boost::async::thread get_return_object();

  struct state_t
  {
    asio::io_context ctx{1u};
    asio::cancellation_signal signal;
    std::mutex mtx;
    std::exception_ptr ep;
    std::optional<completion_handler<>> waitor;
    bool done = false;
  };
 private:

  std::shared_ptr<state_t> state = std::make_shared<state_t>();
  std::optional<asio::executor_work_guard<executor_type>> exec;
};

struct thread_awaitable
{
  bool await_ready() const
  {
    if (state_ == nullptr)
      throw std::logic_error("Thread expired");
    std::lock_guard<std::mutex> lock{state_->mtx};
    return state_->done;
  }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    BOOST_ASSERT(state_);

    std::lock_guard<std::mutex> lock{state_->mtx};
    if (state_->done)
      return false;

    if constexpr (requires (Promise p) {p.get_cancellation_slot();})
      if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
      {
        if (thread_) // connect to cancel the thread
          sl.assign(
              [st = state_](asio::cancellation_type type)
              {
                std::lock_guard<std::mutex> lock{st->mtx};
                asio::post(st->ctx,[st, type]{st->signal.emit(type);});
              });
        else
          sl.assign(
              [st = state_](asio::cancellation_type type)
              {
                std::lock_guard<std::mutex> lock{st->mtx};
                if (type == interrupt_await)
                  asio::post(*std::exchange(st->waitor, std::nullopt));
                else
                  asio::post(st->ctx,[st, type]{st->signal.emit(type);});
              });
      }

    state_->waitor.emplace(h);
    return true;
  }

  void await_resume()
  {
    if (thread_)
      thread_-> join();
    if (state_->ep)
      std::rethrow_exception(state_->ep);
  }

  explicit thread_awaitable(std::shared_ptr<detail::thread_promise::state_t> state)
        : state_(std::move(state)) {}

  explicit thread_awaitable(std::thread thread,
                            std::shared_ptr<detail::thread_promise::state_t> state)
        : thread_(std::move(thread)), state_(std::move(state)) {}
 private:
  std::optional<std::thread> thread_;
  std::shared_ptr<detail::thread_promise::state_t> state_;
};

}

struct thread
{
  using promise_type = detail::thread_promise;

  void cancel(asio::cancellation_type type = asio::cancellation_type::all)
  {
    if (state_)
      asio::post(state_->ctx,[s= state_, type]{s->signal.emit(type);});
  }

  void join() {thread_.join();}
  bool joinable() const {return thread_.joinable();}
  void detach()
  {
    thread_.detach();
    state_ = nullptr;
  }

  auto operator co_await() &-> detail::thread_awaitable
  {
    return  detail::thread_awaitable{std::move(state_)};
  }
  auto operator co_await() && -> detail::thread_awaitable
  {
    return  detail::thread_awaitable{std::move(thread_), std::move(state_)};
  }

  ~thread()
  {
    if (state_)
      std::exchange(state_, nullptr)->ctx.stop();

    if (thread_.joinable())
      thread_.join();
  }
  thread(thread &&) noexcept = default;
 private:
  thread(std::thread thr, std::shared_ptr<detail::thread_promise::state_t> state)
      : thread_(std::move(thr)), state_(std::move(state))
  {
  }

  std::thread thread_;
  std::shared_ptr<detail::thread_promise::state_t> state_;
  friend struct detail::thread_promise;;
};


}

#endif //BOOST_ASYNC_THREAD_HPP
