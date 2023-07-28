//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_THREAD_HPP
#define BOOST_ASYNC_DETAIL_THREAD_HPP

#include <boost/async/config.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/async/concepts.hpp>
#include <boost/async/this_coro.hpp>

#include <boost/asio/cancellation_signal.hpp>

#include <thread>

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


struct thread_state
{
  asio::io_context ctx{1u};
  asio::cancellation_signal signal;
  std::mutex mtx;
  std::optional<completion_handler<std::exception_ptr>> waitor;
  std::atomic<bool> done = false;
};

struct thread_promise : signal_helper_2,
                        promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                        promise_throw_if_cancelled_base,
                        enable_awaitables<thread_promise>,
                        enable_await_allocator<thread_promise>,
                        enable_await_executor<thread_promise>
{
  BOOST_ASYNC_DECL thread_promise();

  auto initial_suspend() noexcept
  {
    struct aw
    {
      bool await_ready() const {return false;}
      void await_suspend(std::coroutine_handle<thread_promise> h)
      {
        h.promise().mtx.unlock();
      }

      void await_resume() {}
    };
    return aw{};
  }
  std::suspend_never final_suspend() noexcept
  {
    wexec_.reset();
    return {};
  }

  void unhandled_exception() { throw; }
  void return_void() { }

  using executor_type = typename async::executor;
  const executor_type & get_executor() const {return *exec_;}

  using allocator_type = pmr::polymorphic_allocator<void>;
  using resource_type  = pmr::unsynchronized_pool_resource;

  resource_type * resource;
  allocator_type  get_allocator() const { return allocator_type(resource); }

  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<thread_promise>::await_transform;
  using enable_await_allocator<thread_promise>::await_transform;
  using enable_await_executor<thread_promise>::await_transform;

  BOOST_ASYNC_DECL
  boost::async::thread get_return_object();

  void set_executor(asio::io_context::executor_type exec)
  {
    wexec_.emplace(exec);
    exec_.emplace(exec);
  }

  std::mutex mtx;
 private:

  std::optional<asio::executor_work_guard<asio::io_context::executor_type>> wexec_;
  std::optional<async::executor> exec_;
};

struct thread_awaitable
{
  asio::cancellation_slot cl;
  std::optional<std::tuple<std::exception_ptr>> res;
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
      if ((cl = h.promise().get_cancellation_slot()).is_connected())
      {
        cl.assign(
            [st = state_](asio::cancellation_type type)
            {
              std::lock_guard<std::mutex> lock{st->mtx};
              asio::post(st->ctx,[st, type]{st->signal.emit(type);});
            });

      }

    state_->waitor.emplace(h, res);
    return true;
  }

  void await_resume()
  {
    if (cl.is_connected())
      cl.clear();
    if (thread_)
      thread_->join();
    if (!res) // await_ready
      return;
    if (auto ee = std::get<0>(*res))
      std::rethrow_exception(ee);
  }

  explicit thread_awaitable(std::shared_ptr<detail::thread_state> state)
      : state_(std::move(state)) {}

  explicit thread_awaitable(std::thread thread,
                            std::shared_ptr<detail::thread_state> state)
      : thread_(std::move(thread)), state_(std::move(state)) {}
 private:
  std::optional<std::thread> thread_;
  std::shared_ptr<detail::thread_state> state_;
};
}

}

#endif //BOOST_ASYNC_DETAIL_THREAD_HPP
