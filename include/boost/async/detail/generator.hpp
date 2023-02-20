//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_GENERATOR_HPP
#define BOOST_ASYNC_DETAIL_GENERATOR_HPP

#include <boost/async/detail/async_operation.hpp>
#include <boost/async/detail/concepts.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/asio/bind_allocator.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/wrapper.hpp>


namespace boost::async
{


template<typename Yield, typename Push>
struct generator;

namespace detail
{

template<typename Yield, typename Push>
struct generator_yield_awaitable;

template<typename Yield, typename Push>
struct generator_receiver;

template<typename Yield, typename Push>
struct generator_receiver_base
{
  std::optional<Push> pushed_value;
  auto get_awaitable(const Push  & push)
  {
    pushed_value.emplace(push);
    using impl = generator_receiver<Yield, Push>;
    return typename impl::awaitable{static_cast<impl*>(this)};
  }
  auto get_awaitable(      Push && push)
  {
    pushed_value.emplace(std::forward<Push>(push));
    using impl = generator_receiver<Yield, Push>;
    return typename impl::awaitable{static_cast<impl*>(this)};
  }
};

template<typename Yield>
struct generator_receiver_base<Yield, void>
{
  bool pushed_value{false};

  auto get_awaitable()
  {
    pushed_value = true;

    using impl = generator_receiver<Yield, void>;
    return typename impl::awaitable{static_cast<impl*>(this)};
  }
};

template<typename Yield, typename Push>
struct generator_receiver : generator_receiver_base<Yield, Push>
{
  std::exception_ptr exception;
  std::optional<Yield> result;
  Yield get_result() {return *std::exchange(result, std::nullopt);}
  bool done = false;
  std::coroutine_handle<void> awaited_from{nullptr};
  std::unique_ptr<void, detail::coro_deleter<>> yield_from;

  bool ready() { return exception || result || done; }

  generator_receiver() = default;
  generator_receiver(generator_receiver && lhs)
  : exception(std::move(lhs.exception)), done(lhs.done), awaited_from(lhs.awaited_from),
  reference(lhs.reference), cancel_signal(lhs.cancel_signal)
  {
    if (!lhs.done && !lhs.exception)
    {
      reference = this;
      lhs.exception = moved_from_exception();
    }
    lhs.done = true;


  }

  ~generator_receiver()
  {
    if (!done && reference == this)
      reference = nullptr;
  }

  generator_receiver(generator_receiver * (&reference), asio::cancellation_signal & cancel_signal)
  : reference(reference), cancel_signal(cancel_signal)
  {
    reference = this;
  }

  generator_receiver  * (&reference);
  asio::cancellation_signal & cancel_signal;

  using yield_awaitable = generator_yield_awaitable<Yield, Push>;

  yield_awaitable get_yield_awaitable() {return {this}; }
  static yield_awaitable terminator()   {return {nullptr}; }

  template<typename T>
  void yield_value( T&& t)
  {
    result.emplace(std::forward<T>(t));
  }

  struct awaitable
  {
    generator_receiver *self;
    awaitable(generator_receiver * self) : self(self)
    {
    }

    awaitable(awaitable && aw) : self(aw.self)
    {
    }

    alignas(sizeof(void*)) char buffer[1024];
    container::pmr::monotonic_buffer_resource resource{buffer, sizeof(buffer)};

    // the race is fine -> if we miss it, we'll get it in resume.
    bool await_ready() const
    {
       return self->ready() && !self->pushed_value;
    }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if (self->done) // ok, so we're actually done already, so noop
        return std::noop_coroutine();

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

      std::coroutine_handle<void> res = std::noop_coroutine();
      if (self->yield_from != nullptr)
        res = std::coroutine_handle<void>::from_address(self->yield_from.release());
      return res;
    }

    Yield await_resume()
    {
      if (self->exception)
        std::rethrow_exception(std::exchange(self->exception, nullptr));
      if (!self->result)
        boost::throw_exception(std::logic_error("async::generator returned"));
      return self->get_result();
    }
  };

  void interrupt_await()
  {
    exception = detached_exception();
    awaited_from.resume();
  }

  void rethrow_if()
  {
    if (exception)
      std::rethrow_exception(exception);
  }
};

template<typename Yield, typename Push>
struct generator_promise
    : promise_memory_resource_base,
      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
      promise_throw_if_cancelled_base,
      enable_awaitables<generator_promise<Yield, Push>>,
      enable_await_allocator<container::pmr::polymorphic_allocator<void>>,
      enable_async_operation_interpreted
{
  using enable_await_allocator<container::pmr::polymorphic_allocator<void>>::await_transform;
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<generator_promise<Yield, Push>>::await_transform;
  using enable_async_operation_interpreted::await_transform;

  [[nodiscard]] generator<Yield, Push> get_return_object()
  {
    return generator<Yield, Push>{this};
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
  allocator_type get_allocator() const {return container::pmr::polymorphic_allocator<void>{this_thread::get_default_resource()};}

  std::suspend_never initial_suspend()        {return {};}
  auto final_suspend() noexcept
  {
    struct final_awaitable
    {
      generator_promise * generator;
      bool await_ready() const noexcept
      {
        return generator->receiver && generator->receiver->awaited_from.address() == nullptr;
      }

      auto await_suspend(std::coroutine_handle<generator_promise> h) noexcept -> std::coroutine_handle<void>
      {
        std::coroutine_handle<void> res = std::noop_coroutine();
        if (generator->receiver && generator->receiver->awaited_from.address() != nullptr)
          res = generator->receiver->awaited_from;
        if (generator->receiver)
            generator->receiver->done = true;
        h.destroy();
        return res;
      }

      void await_resume() noexcept
      {
        if (generator->receiver)
          generator->receiver->done = true;
      }
    };

    return final_awaitable{this};
  }

  void unhandled_exception()
  {
    if (this->receiver)
      this->receiver->exception = std::current_exception();
    else
      throw ;
  }

  void return_value(const Yield & res)
  {
    if (this->receiver)
      this->receiver->yield_value(res);
  }

  void return_value(Yield && res)
  {
    if (this->receiver)
      this->receiver->yield_value(std::move(res));
  }

  generator_receiver<Yield, Push>* receiver{nullptr};

  template<typename Yield_>
  auto yield_value(Yield_ && ret)
  {
    if(receiver)
    {
      receiver->yield_value(std::forward<Yield_>(ret));
      return receiver->get_yield_awaitable();
    }
    else
      return generator_receiver<Yield, Push>::terminator();
  }

  void interrupt_await()
  {
    if (this->receiver)
    {
      this->receiver->exception = detached_exception();
      this->receiver->awaited_from.resume();
    }
  }

  ~generator_promise()
  {
    if (this->receiver)
    {
      if (!this->receiver->done && !this->receiver->exception)
        this->receiver->exception = detail::completed_unexpected();
      this->receiver->done = true;
    }
  }

  friend struct async_initiate;
};

template<typename Yield, typename Push>
struct generator_yield_awaitable
{
  generator_receiver<Yield, Push> * self;
  constexpr bool await_ready() { return self->pushed_value && !self->result; }

  std::coroutine_handle<void> await_suspend(std::coroutine_handle<generator_promise<Yield, Push>> h)
  {
    std::coroutine_handle<void> res = std::noop_coroutine();
    if (self->awaited_from.address() != nullptr)
      res = self->awaited_from;
    self->yield_from.reset(h.address());
    return res;
  }

  Push await_resume()
  {
    return *std::exchange(self->pushed_value, std::nullopt);
  }
};

template<typename Yield>
struct generator_yield_awaitable<Yield, void>
{
  generator_receiver<Yield, void> * self;
  constexpr bool await_ready() { return self->pushed_value; }

  auto await_suspend(std::coroutine_handle<generator_promise<Yield, void>> h)
  {
    std::coroutine_handle<void> res = std::noop_coroutine();
    if (self->awaited_from.address() != nullptr)
      res = self->awaited_from;
    self->yield_from.reset(h.address());
    return res;
  }

  void await_resume()
  {
    self->pushed_value = false;
  }
};


template<typename Yield, typename Push>
struct generator_base
{
  auto operator()(      Push && push)
  {
    return static_cast<generator<Yield, Push>*>(this)->receiver_.get_awaitable(std::move(push));
  }
  auto operator()(const Push &  push)
  {
    return static_cast<generator<Yield, Push>*>(this)->receiver_.get_awaitable(push);
  }
};

template<typename Yield>
struct generator_base<Yield, void>
{
  auto operator co_await ()
  {
    return static_cast<generator<Yield, void>*>(this)->receiver_.get_awaitable();
  }
};

}

}

#endif //BOOST_ASYNC_DETAIL_GENERATOR_HPP
