//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_PROMISE_HPP
#define BOOST_ASYNC_DETAIL_PROMISE_HPP

#include <boost/async/detail/async_operation.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/wrapper.hpp>
#include <boost/async/detail/this_thread.hpp>

#include <boost/asio/cancellation_signal.hpp>

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <coroutine>
#include <optional>
#include <utility>
#include <boost/asio/bind_allocator.hpp>



namespace boost::async
{

template<typename Return>
struct promise;

namespace detail
{

template<typename T>
struct promise_receiver;

template<typename T>
struct promise_value_holder
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
    static_cast<promise_receiver<T>*>(this)->set_done();
  }

};

template<>
struct promise_value_holder<void>
{
  bool result_taken = false;
  void get_result() { result_taken = true; }

  inline void return_void();
};



template<typename T>
struct promise_receiver : promise_value_holder<T>
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
  std::coroutine_handle<void> awaited_from{nullptr};

  void set_done()
  {
    done = true;
  }

  promise_receiver() = default;
  promise_receiver(promise_receiver && lhs)
      : promise_value_holder<T>(std::move(lhs)),
        exception(std::move(lhs.exception)), done(lhs.done), awaited_from(lhs.awaited_from),
        reference(lhs.reference), cancel_signal(lhs.cancel_signal)
  {
    if (!done && !exception)
    {
      reference = this;
      lhs.exception = moved_from_exception();
    }

    lhs.done = true;
  }

  ~promise_receiver()
  {
    if (!done && reference == this)
      reference = nullptr;
  }

  promise_receiver(promise_receiver * (&reference), asio::cancellation_signal & cancel_signal)
      : reference(reference), cancel_signal(cancel_signal)
  {
    reference = this;
  }

  struct awaitable
  {
    promise_receiver * self;

    awaitable(promise_receiver * self) : self(self)
    {
    }

    awaitable(awaitable && aw) : self(aw.self)
    {
    }

    ~awaitable ()
    {
    }

    alignas(sizeof(void*)) char buffer[1024];
    container::pmr::monotonic_buffer_resource resource{buffer, sizeof(buffer)};

    // the race is fine -> if we miss it, we'll get it in resume.
    bool await_ready() const { return self->done; }

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      if (self->done) // ok, so we're actually done already, so noop
        return false;

      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
          sl.template emplace<forward_cancellation_with_interrupt>(self->cancel_signal, self);


      if constexpr (requires (Promise p) {p.get_executor();})
        self->awaited_from = detail::dispatch_coroutine(
            h.promise().get_executor(),
            asio::bind_allocator(
                container::pmr::polymorphic_allocator<void>(&resource),
                [h]() mutable { h.resume(); })
        );
      else
        self->awaited_from = h;

      return true;
    }

    T await_resume()
    {
      self->rethrow_if();
      return self->get_result();
    }
  };

  promise_receiver  * (&reference);
  asio::cancellation_signal & cancel_signal;

  awaitable get_awaitable() {return awaitable{this};}


  void interrupt_await()
  {
    exception = detached_exception();
    awaited_from.resume();
  }
};

inline void promise_value_holder<void>::return_void()
{
  static_cast<promise_receiver<void>*>(this)->set_done();
}

template<typename Return>
struct async_promise_result
{
  promise_receiver<Return>* receiver{nullptr};
  void return_value(Return && ret)
  {
    if(receiver)
      receiver->return_value(std::move(ret));
  }

};

template<>
struct async_promise_result<void>
{
  promise_receiver<void>* receiver{nullptr};
  void return_void()
  {
    if(receiver)
      receiver->return_void();
  }
};

struct async_initiate;
template<typename Return>
struct async_promise
    : promise_memory_resource_base,
      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
      promise_throw_if_cancelled_base,
      enable_awaitables<async_promise<Return>>,
      enable_await_allocator<async_promise<Return>>,
      enable_await_executor<async_promise<Return>>,
      enable_async_operation_interpreted,
      async_promise_result<Return>
{
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<async_promise<Return>>::await_transform;
  using enable_await_allocator<async_promise<Return>>::await_transform;
  using enable_await_executor<async_promise<Return>>::await_transform;
  using enable_async_operation_interpreted::await_transform;

  [[nodiscard]] promise<Return> get_return_object()
  {
    return promise<Return>{this};
  }

  mutable asio::cancellation_signal signal;
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type get_cancellation_slot() const
  {
    return signal.slot();
  }

  using executor_type = asio::io_context::executor_type;
  executor_type exec{boost::async::this_thread::get_executor()};
  executor_type get_executor() const {return exec;}

  using allocator_type = container::pmr::polymorphic_allocator<void>;
  allocator_type get_allocator() const {return alloc;}
  container::pmr::polymorphic_allocator<void> alloc{this_thread::get_default_resource()};

  template<typename ... Args>
  async_promise(Args & ...args)
      : exec{detail::get_executor_from_args(args...)}
      , alloc{detail::get_memory_resource_from_args(args...)}
  {}

  std::suspend_never initial_suspend()        {return {};}
  auto final_suspend() noexcept
  {
    struct final_awaitable
    {
      async_promise * promise;
      bool await_ready() const noexcept
      {
        return promise->receiver && promise->receiver->awaited_from.address() == nullptr;
      }

      auto await_suspend(std::coroutine_handle<async_promise> h) noexcept -> std::coroutine_handle<void>
      {
        std::coroutine_handle<void> res = std::noop_coroutine();
        if (promise->receiver && promise->receiver->awaited_from.address() != nullptr)
          res = promise->receiver->awaited_from;

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

  ~async_promise()
  {
    if (this->receiver)
    {
      if (!this->receiver->done && !this->receiver->exception)
        this->receiver->exception = completed_unexpected();
      this->receiver->set_done();
    }

  }

  friend struct async_initiate;
};

}

}

#endif //BOOST_ASYNC_DETAIL_PROMISE_HPP
