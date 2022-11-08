//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CORO_THREAD_HPP
#define CORO_THREAD_HPP

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/signal_set.hpp>

#include <thread>

#include <coro/concepts.hpp>
#include <coro/allocator.hpp>
#include <coro/async_operation.hpp>
#include <coro/ops.hpp>
#include <coro/this_coro.hpp>


namespace coro
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
  thread_promise() : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>(
      signal_helper_2::signal.slot(), asio::enable_total_cancellation())
  {
  }

  std::suspend_always initial_suspend() {return {};}
  std::suspend_never    final_suspend() noexcept { return {};}

  void unhandled_exception() { throw ; }
  void return_void()
  {
  }

  void run()
  {
    coro::set_default_resource(&resource);
    auto st = std::move(state);

    if (st->signal.slot().is_connected())
        st->signal.slot().assign([this](asio::cancellation_type tp){signal.emit(tp);});

    exec.emplace(st->ctx.get_executor());

    asio::post(st->ctx.get_executor(),
               [this]
               {
                  std::coroutine_handle<thread_promise>::from_promise(*this).resume();
               });

    st->ctx.run();
  }


  using executor_type = typename asio::io_context::executor_type;
  executor_type get_executor() const {return exec->get_executor();}

  using allocator_type = std::pmr::polymorphic_allocator<void>;
  using resource_type  = std::pmr::unsynchronized_pool_resource;

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

  inline coro::thread get_return_object();



  struct state_t
  {
    asio::io_context ctx;
    asio::cancellation_signal signal;
  };
 private:

  std::shared_ptr<state_t> state = std::make_shared<state_t>();
  std::optional<asio::executor_work_guard<executor_type>> exec;

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

coro::thread detail::thread_promise::get_return_object()
{
  auto st = state;
  return coro::thread{std::thread{[this]{run();}}, std::move(st)};
}

}

#endif //CORO_THREAD_HPP
