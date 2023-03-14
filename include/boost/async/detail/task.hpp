//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_TASK_HPP
#define BOOST_ASYNC_DETAIL_TASK_HPP

#include <boost/async/detail/async_operation.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/wrapper.hpp>
#include <boost/async/detail/this_thread.hpp>

#include <boost/asio/cancellation_signal.hpp>

#include <boost/container/pmr/memory_resource.hpp>

#include <coroutine>
#include <optional>
#include <utility>
#include <boost/asio/bind_allocator.hpp>

namespace boost::async
{

template<typename Return>
struct task;

namespace detail
{

template<typename T>
struct task_receiver;

template<typename T>
struct task_value_holder
{
  std::optional<T> result;
  bool result_taken = false;

  T get_result()
  {
    result_taken = true;
    return std::move(*result);
  }

  void return_value(T && ret)
  {
    result = std::move(ret);
    static_cast<task_receiver<T>*>(this)->set_done();
  }

};

template<>
struct task_value_holder<void>
{
  bool result_taken = false;
  void get_result() { result_taken = true; }

  inline void return_void();
};


template<typename T>
struct task_promise;

template<typename T>
struct task_receiver : task_value_holder<T>
{
  std::exception_ptr exception;
  void rethrow_if()
  {
    if (exception && !done) // detached error
      std::rethrow_exception(std::exchange(exception, nullptr));
    else if (exception)
    {
      this->result_taken = true;
      std::rethrow_exception(exception);
    }
  }
  void unhandled_exception()
  {
    exception = std::current_exception();
    set_done();
  }

  bool done = false;
  std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};

  void set_done()
  {
    done = true;
  }

  task_receiver() = default;
  task_receiver(task_receiver && lhs)
      : task_value_holder<T>(std::move(lhs)),
        exception(std::move(lhs.exception)), done(lhs.done), awaited_from(std::move(lhs.awaited_from)),
        promise(lhs.promise)
  {
    if (!done && !exception)
    {
      promise->receiver = this;
      lhs.exception = moved_from_exception();
    }

    lhs.done = true;
  }

  ~task_receiver()
  {
    if (!done && promise && promise->receiver == this)
    {
      promise->receiver = nullptr;
      if (!promise->started)
        std::coroutine_handle<task_promise<T>>::from_promise(*promise).destroy();
    }
  }

  task_receiver(task_promise<T> * promise)
      : promise(promise)
  {
      promise->receiver = this;
  }

  struct awaitable
  {
    task_receiver * self;
    asio::cancellation_slot cl;
    awaitable(task_receiver * self) : self(self)
    {
    }

    awaitable(awaitable && aw) : self(aw.self)
    {
    }

    ~awaitable ()
    {
    }

    // the race is fine -> if we miss it, we'll get it in resume.
    bool await_ready() const { return self->done; }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if (self->done) // ok, so we're actually done already, so noop
        return h;

      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if ((cl = h.promise().get_cancellation_slot()).is_connected())
          cl.emplace<forward_cancellation_with_interrupt>(self->promise->signal, self);


      if constexpr (requires (Promise p) {p.get_executor();})
        self->promise->exec.emplace(h.promise().get_executor());
      else
        self->promise->exec.emplace(this_thread::get_executor());
      self->awaited_from.reset(h.address());

      return std::coroutine_handle<task_promise<T>>::from_promise(*self->promise);
    }

    T await_resume()
    {
      if (cl.is_connected())
        cl.clear();
      self->rethrow_if();
      return self->get_result();
    }
  };

  task_promise<T>  * promise;

  awaitable get_awaitable() {return awaitable{this};}


  void interrupt_await()
  {
    exception = detached_exception();
    std::coroutine_handle<void>::from_address(awaited_from.release()).resume();
  }
};

inline void task_value_holder<void>::return_void()
{
  static_cast<task_receiver<void>*>(this)->set_done();
}

template<typename Return>
struct task_promise_result
{
  task_receiver<Return>* receiver{nullptr};
  void return_value(Return && ret)
  {
    if(receiver)
      receiver->return_value(std::move(ret));
  }

};

template<>
struct task_promise_result<void>
{
  task_receiver<void>* receiver{nullptr};
  void return_void()
  {
    if(receiver)
      receiver->return_void();
  }
};

struct async_initiate;
template<typename Return>
struct task_promise
    : promise_memory_resource_base,
      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
      promise_throw_if_cancelled_base,
      enable_awaitables<task_promise<Return>>,
      enable_await_allocator<task_promise<Return>>,
      enable_await_executor<task_promise<Return>>,
      enable_async_operation,
      task_promise_result<Return>
{
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<task_promise<Return>>::await_transform;
  using enable_await_allocator<task_promise<Return>>::await_transform;
  using enable_await_executor<task_promise<Return>>::await_transform;
  using enable_async_operation::await_transform;

  [[nodiscard]] task<Return> get_return_object()
  {
    return task<Return>{this};
  }

  mutable asio::cancellation_signal signal;
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type get_cancellation_slot() const
  {
    return signal.slot();
  }

  using executor_type = asio::io_context::executor_type;
  std::optional<asio::executor_work_guard<executor_type>> exec;
  executor_type get_executor() const
  {
      if (!exec)
          throw_exception(asio::bad_executor());
      return exec->get_executor();
  }

  template<typename ... Args>
  task_promise(Args & ...args)
      : promise_memory_resource_base(detail::get_memory_resource_from_args(args...))
  {}

  auto initial_suspend()
  {
    struct initial_awaitable
    {
      task_promise * promise;

      bool await_ready() const noexcept {return false;}
      void await_suspend(std::coroutine_handle<>) {}

      void await_resume()
      {
        promise->started = true;
      }
    };
    return initial_awaitable{this};
  }
  auto final_suspend() noexcept
  {
    struct final_awaitable
    {
      task_promise * promise;
      bool await_ready() const noexcept
      {
        return promise->receiver && promise->receiver->awaited_from.get() == nullptr;
      }

      auto await_suspend(std::coroutine_handle<task_promise> h) noexcept -> std::coroutine_handle<void>
      {
        std::coroutine_handle<void> res = std::noop_coroutine();
        if (promise->receiver && promise->receiver->awaited_from.get() != nullptr)
          res = std::coroutine_handle<void>::from_address(promise->receiver->awaited_from.release());

        h.destroy();
        return res;
      }

      void await_resume() noexcept
      {
      }
    };

    return final_awaitable{this};
  }

  void unhandled_exception()
  {
    if (this->receiver)
      this->receiver->unhandled_exception();
    else
      throw ;
  }

  ~task_promise()
  {
    if (this->receiver)
    {
      if (!this->receiver->done && !this->receiver->exception)
        this->receiver->exception = completed_unexpected();
      this->receiver->set_done();
      this->receiver->awaited_from.reset(nullptr);
    }

  }
  bool started = false;
  friend struct async_initiate;
};

}

}

#endif //BOOST_ASYNC_DETAIL_TASK_HPP
