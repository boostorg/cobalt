//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_ASYNC_HPP
#define BOOST_ASYNC_ASYNC_HPP

#include <boost/async/this_coro.hpp>
#include <boost/async/cancellation.hpp>
#include <boost/async/detail/concepts.hpp>
#include <boost/async/detail/async_operation.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/wrapper.hpp>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/append.hpp>
#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/bind_executor.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>

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
          enable_await_allocator<container::pmr::polymorphic_allocator<void>>,
          enable_async_operation_interpreted,
          async_promise_result<Return>
{
    using enable_await_allocator<container::pmr::polymorphic_allocator<void>>::await_transform;
    using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using enable_awaitables<async_promise<Return>>::await_transform;
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
    allocator_type get_allocator() const {return container::pmr::polymorphic_allocator<void>{this_thread::get_default_resource()};}

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

template<typename Return>
struct [[nodiscard]] promise
{
    using promise_type = detail::async_promise<Return>;

    promise(const promise &) = delete;
    promise& operator=(const promise &) = delete;

    promise(promise &&lhs) noexcept = default;
    promise& operator=(promise &&) noexcept = default;

    auto operator co_await () {return receiver_.get_awaitable();}

    // Ignore the returns value
    void operator +() && {detach();}

    void cancel(asio::cancellation_type ct = asio::cancellation_type{0b111u})
    {
      if (!receiver_.done)
        receiver_.cancel_signal.emit(ct);
    }

    bool ready() const  { return receiver_.done; }
    explicit operator bool () const
    {
      return !receiver_.done || !receiver_.result_taken;
    }

    Return get()
    {
      if (!ready())
        boost::throw_exception(std::logic_error("promise not ready"));

      receiver_.rethrow_if();
      return receiver_.get_result();
    }

    bool attached() const {return attached_;}
    void detach() {attached_ = false;}
    void attach() {attached_ = false;}

    ~promise()
    {
      if (attached_)
        cancel();
    }
  private:
    template<typename>
    friend struct detail::async_promise;

    promise(detail::async_promise<Return> * promise) : receiver_(promise->receiver, promise->signal), attached_{true}
    {
    }

    detail::promise_receiver<Return> receiver_;
    bool attached_;
    friend struct detail::async_initiate;
};

namespace detail
{

struct async_initiate
{
    template<typename Handler, typename T>
    void operator()(Handler && h, promise<T> a)
    {
        auto & rec = a.receiver_;
        if (rec.done)
          return asio::post(asio::append(h, rec.exception, rec.exception ? T() : rec.get_result()));

        auto alloc = asio::get_associated_allocator(h, container::pmr::polymorphic_allocator<void>{boost::async::this_thread::get_default_resource()});
        auto recs = std::allocate_shared<detail::promise_receiver<T>>(
                                alloc, std::move(rec));

        auto sl = asio::get_associated_cancellation_slot(h);
        if (sl.is_connected())
            sl.template emplace<detail::forward_cancellation>(recs->cancel_signal);

        auto p = recs.get();

        auto exec = asio::get_associated_executor(h);

        p->awaited_from = detail::dispatch_coroutine(
            asio::bind_executor(
                exec,
                asio::bind_allocator(
                    alloc,
                    [r = std::move(recs),
                     h = std::move(h)]() mutable
                     {
                        auto ex = r->exception;
                        auto rr = std::move(r->get_result());
                        r = nullptr ;
                        h(ex, std::move(rr));

                    }
                    )
                )
            );
        a.detach();
    }

    template<typename Handler>
    void operator()(Handler && h, promise<void> a)
    {
        auto & rec = a.receiver_;
        if (a.receiver_.done)
            return asio::post(asio::append(h, a.receiver_.exception));

        auto alloc = asio::get_associated_allocator(h, container::pmr::polymorphic_allocator<void>{boost::async::this_thread::get_default_resource()});
        auto recs = std::allocate_shared<detail::promise_receiver<void>>(
                                alloc, std::move(a.receiver_));

        if (recs->done)
            return asio::post(asio::append(h, recs->exception));


        auto sl = asio::get_associated_cancellation_slot(h);
        if (sl.is_connected())
            sl.template emplace<detail::forward_cancellation>(recs->cancel_signal);

        auto p = recs.get();

        auto exec = asio::get_associated_executor(h);

        p->awaited_from = detail::dispatch_coroutine(
                asio::bind_executor(
                    exec,
                    asio::bind_allocator(
                        alloc,
                        [r = std::move(recs),
                         h = std::move(h)]() mutable
                         {
                             auto ex = r->exception;
                             r = nullptr ;
                             h(ex);
                        }
                        )
                    )
                );
        a.detach();
    }


    template<typename Handler, typename T>
    struct promise_type
    {
        auto initial_suspend() noexcept { return std::suspend_never(); }
        auto final_suspend() noexcept { return std::suspend_never(); }
    };
};

}

template<typename T, typename CompletionToken>
auto spawn(promise<T> && t, CompletionToken&& token)
{
    return asio::async_initiate<CompletionToken, void(std::exception_ptr, T)>(
            detail::async_initiate{}, token, std::move(t));
}

template<typename CompletionToken>
auto spawn(promise<void> && t, CompletionToken&& token)
{
  return asio::async_initiate<CompletionToken, void(std::exception_ptr)>(
      detail::async_initiate{}, token, std::move(t));
}


template<typename ExecutionContext, typename T, typename CompletionToken>
    requires (std::is_convertible<ExecutionContext&, asio::execution_context&>::value)
auto spawn(ExecutionContext & context,
           promise<T> && t,
           CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(typename ExecutionContext::executor_type))
{
    return spawn(std::move(t), asio::bind_executor(context.get_executor(), std::forward<CompletionToken>(token)));
}

template<typename Executor, typename T, typename CompletionToken>
    requires (asio::is_executor<Executor>::value || asio::execution::is_executor<Executor>::value)
auto spawn(Executor executor,
           promise<T> && t,
           CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(Executor))
{
    return spawn(std::move(t), asio::bind_executor(executor, std::forward<CompletionToken>(token)));
}

}

namespace std
{

template<typename Handler, typename T>
struct coroutine_traits<void, boost::async::detail::async_initiate, Handler, T>
{
    using promise_type = boost::async::detail::async_initiate::promise_type<Handler, T>;
};

}


#endif //BOOST_ASYNC_ASYNC_HPP
