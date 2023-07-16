//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_GENERATOR_HPP
#define BOOST_ASYNC_DETAIL_GENERATOR_HPP

#include <boost/async/detail/async_operation.hpp>
#include <boost/async/concepts.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/this_thread.hpp>
#include <boost/async/detail/wrapper.hpp>

#include <boost/asio/bind_allocator.hpp>
#include <boost/core/exchange.hpp>
#include <boost/variant2/variant.hpp>

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
    using impl = generator_receiver<Yield, Push>;
    return typename impl::awaitable{static_cast<impl*>(this), &push};
  }
  auto get_awaitable(      Push && push)
  {
    using impl = generator_receiver<Yield, Push>;
    return typename impl::awaitable{static_cast<impl*>(this), &push};
  }
};

template<typename Yield>
struct generator_receiver_base<Yield, void>
{
  bool pushed_value{false};

  auto get_awaitable()
  {
    using impl = generator_receiver<Yield, void>;
    return typename impl::awaitable{static_cast<impl*>(this), static_cast<void*>(nullptr)};
  }
};

template<typename Yield, typename Push>
struct generator_receiver : generator_receiver_base<Yield, Push>
{
  std::exception_ptr exception;
  std::optional<Yield> result;
  Yield get_result() {return *std::exchange(result, std::nullopt);}
  bool done = false;
  std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};
  std::unique_ptr<void, detail::coro_deleter<>> yield_from{nullptr};

  bool pro_active = false;

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

  generator_receiver(generator_receiver * &reference, asio::cancellation_signal & cancel_signal)
  : reference(reference), cancel_signal(cancel_signal)
  {
    reference = this;
  }

  generator_receiver  * &reference;
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
    std::exception_ptr ex;
    asio::cancellation_slot cl;

    variant2::variant<variant2::monostate, Push *, const Push *> to_push;


    awaitable(generator_receiver * self, Push * to_push) : self(self), to_push(to_push)
    {
    }
    awaitable(generator_receiver * self, const Push * to_push) : self(self), to_push(to_push)
    {
    }

    awaitable(const awaitable & aw) noexcept : self(aw.self), to_push(aw.to_push)
    {
    }

    bool await_ready() const
    {
       return self->ready();
    }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if (self->done) // ok, so we're actually done already, so noop
        return std::noop_coroutine();

      if (self->awaited_from != nullptr) // generator already being awaited, that's an error!
      {
          ex = already_awaited();
          return h;
      }

      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if ((cl = h.promise().get_cancellation_slot()).is_connected())
          cl.emplace<forward_cancellation>(self->cancel_signal);

      self->awaited_from.reset(h.address());

      std::coroutine_handle<void> res = std::noop_coroutine();
      if (self->yield_from != nullptr)
        res = std::coroutine_handle<void>::from_address(self->yield_from.release());
      return std::coroutine_handle<void>::from_address(res.address());
    }

    Yield await_resume()
    {
      if (cl.is_connected())
        cl.clear();
      if (ex)
        std::rethrow_exception(ex);
      if (self->exception)
        std::rethrow_exception(std::exchange(self->exception, nullptr));
      if (!self->result)
        boost::throw_exception(std::logic_error("async::generator returned"), BOOST_CURRENT_LOCATION);

      BOOST_ASSERT(to_push.index() > 0u);

      if constexpr (std::is_void_v<Push>)
        self->pushed_value = true;
      else
      {
        if (to_push.index() == 1)
          self->pushed_value.emplace(std::move(*variant2::get<1>(to_push)));
        else
          self->pushed_value.emplace(std::move(*variant2::get<2>(to_push)));
      }
      to_push = variant2::monostate{};

      // now we also want to resume the coroutine, so it starts work
      if (self->yield_from != nullptr && self->pro_active)
        asio::post(
            this_thread::get_executor(),
            [h = std::exchange(self->yield_from, nullptr)]() mutable
            {
              std::coroutine_handle<void>::from_address(h.release()).resume();
            });
      return self->get_result();
    }

    void interrupt_await() &
    {
      if (!self || !self->awaited_from)
        return ;
      self->exception = detached_exception();
      std::coroutine_handle<void>::from_address(self->awaited_from.release()).resume();
    }
  };

  void interrupt_await() &
  {
    exception = detached_exception();
    std::coroutine_handle<void>::from_address(awaited_from.release()).resume();
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
      enable_await_allocator<generator_promise<Yield, Push>>,
      enable_await_executor< generator_promise<Yield, Push>>,
      enable_async_operation
{
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<generator_promise<Yield, Push>>::await_transform;
  using enable_async_operation::await_transform;
  using enable_await_allocator<generator_promise<Yield, Push>>::await_transform;
  using enable_await_executor<generator_promise<Yield, Push>>::await_transform;

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

  using executor_type = executor;
  executor_type exec{boost::async::this_thread::get_executor()};
  const executor_type & get_executor() const {return exec;}

  template<typename ... Args>
  generator_promise(Args & ...args)
    : promise_memory_resource_base(detail::get_memory_resource_from_args(args...)),
      exec{detail::get_executor_from_args(args...)}
  {}

  std::suspend_never initial_suspend() {return {};}
  auto final_suspend() noexcept
  {
    struct final_awaitable
    {
      generator_promise * generator;
      bool await_ready() const noexcept
      {
        return generator->receiver && generator->receiver->awaited_from.get() == nullptr;
      }

      auto await_suspend(std::coroutine_handle<generator_promise> h) noexcept
      {
        std::coroutine_handle<void> res = std::noop_coroutine();
        if (generator->receiver && generator->receiver->awaited_from.get() != nullptr)
          res = std::coroutine_handle<void>::from_address(generator->receiver->awaited_from.release());
        if (generator->receiver)
            generator->receiver->done = true;


        if (auto & rec = h.promise().receiver; rec != nullptr)
        {
          if (!rec->done && !rec->exception)
            rec->exception = detail::completed_unexpected();
          rec->done = true;
          rec->awaited_from.reset(nullptr);
          rec = nullptr;
        }

        detail::self_destroy(h);
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

  auto await_transform(this_coro::pro_active val)
  {
    struct awaitable
    {
      generator_receiver<Yield, Push>* r;
      bool pro_active ;
      bool await_ready() {return true;}
      void await_suspend(std::coroutine_handle<>) const {BOOST_ASSERT(!"Must not be called");}
      void await_resume() const
      {
        if (r)
          r->pro_active = this->pro_active;
      }
    };

    return awaitable{receiver, static_cast<bool>(val)};
  }

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

  void interrupt_await() &
  {
    if (this->receiver)
    {
      this->receiver->exception = detached_exception();
      std::coroutine_handle<void>::from_address(this->receiver->awaited_from.release()).resume();
    }
  }

  ~generator_promise()
  {
    if (this->receiver)
    {
      if (!this->receiver->done && !this->receiver->exception)
        this->receiver->exception = detail::completed_unexpected();
      this->receiver->done = true;
      this->receiver->awaited_from.reset(nullptr);
    }
  }

  friend struct async_initiate;
};

template<typename Yield, typename Push>
struct generator_yield_awaitable
{
  generator_receiver<Yield, Push> * self;
  constexpr bool await_ready() { return self && self->pushed_value && !self->result; }

  std::coroutine_handle<void> await_suspend(std::coroutine_handle<generator_promise<Yield, Push>> h)
  {
    if (self == nullptr) // we're a terminator, kill it
    {
      if (auto & rec = h.promise().receiver; rec != nullptr)
      {
        if (!rec->done && !rec->exception)
          rec->exception = detail::completed_unexpected();
        rec->done = true;
        rec->awaited_from.reset(nullptr);
        rec = nullptr;
      }

      detail::self_destroy(h);
      return std::noop_coroutine();
    }
    std::coroutine_handle<void> res = std::noop_coroutine();
    if (self->awaited_from.get() != nullptr)
      res = std::coroutine_handle<void>::from_address(self->awaited_from.release());
    self->yield_from.reset(h.address());
    return res;
  }

  Push await_resume()
  {
    BOOST_ASSERT(self->pushed_value);
    return *std::exchange(self->pushed_value, std::nullopt);
  }
};

template<typename Yield>
struct generator_yield_awaitable<Yield, void>
{
  generator_receiver<Yield, void> * self;
  constexpr bool await_ready() { return self && self->pushed_value; }

  std::coroutine_handle<> await_suspend(std::coroutine_handle<generator_promise<Yield, void>> h)
  {
    if (self == nullptr) // we're a terminator, kill it
    {
      if (auto & rec = h.promise().receiver; rec != nullptr)
      {
        if (!rec->done && !rec->exception)
          rec->exception = detail::completed_unexpected();
        rec->done = true;
        rec->awaited_from.reset(nullptr);
        rec = nullptr;
      }
      detail::self_destroy(h);
      return std::noop_coroutine();
    }
    std::coroutine_handle<void> res = std::noop_coroutine();
    if (self->awaited_from.get() != nullptr)
      res= std::coroutine_handle<void>::from_address(self->awaited_from.release());
    self->yield_from.reset(h.address());
    return std::coroutine_handle<>::from_address(res.address());
  }

  void await_resume()
  {
    BOOST_ASSERT(self->pushed_value);
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

template<typename T>
struct generator_with_awaitable
{
  generator_base<T, void> &g;
  std::optional<typename detail::generator_receiver<T, void>::awaitable> awaitable;

  template<typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h)
  {
    g.cancel();
    awaitable.emplace(g.operator co_await());
    return awaitable->await_suspend(h);
  }

  void await_resume() {}

};

}

}

#endif //BOOST_ASYNC_DETAIL_GENERATOR_HPP
