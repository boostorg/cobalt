// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_WRAPPER_HPP
#define BOOST_ASYNC_WRAPPER_HPP

#include <boost/async/this_coro.hpp>
#include <boost/async/concepts.hpp>
#include <boost/async/detail/util.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/config.hpp>

#include <coroutine>
#include <utility>

namespace boost::async::detail
{

template<typename Allocator>
struct partial_promise_base
{
    template<typename CompletionToken>
    void * operator new(const std::size_t size, CompletionToken & token)
    {
      // gcc: 168 40
      // clang: 144 40
      return allocate_coroutine(size, asio::get_associated_allocator(token));
    }

    template<typename Executor, typename CompletionToken>
    void * operator new(const std::size_t size, Executor & exec, CompletionToken & token)
    {
      // gcc: 120 8 16
      // clang: 96 8 16
      return allocate_coroutine(size, asio::get_associated_allocator(token));
    }

    void operator delete(void * raw, const std::size_t size)
    {
      deallocate_coroutine<Allocator>(raw, size);
    }
};

template<>
struct partial_promise_base<std::allocator<void>> {};


template<>           struct partial_promise_base<void> {};
template<typename T> struct partial_promise_base<std::allocator<T>> {};

// alloc options are two: allocator or aligned storage
template<typename Allocator = void>
struct partial_promise : partial_promise_base<Allocator>
{
    auto initial_suspend() noexcept
    {
        return std::suspend_always();
    }

    auto final_suspend() noexcept
    {
        return std::suspend_always();
    }

    void return_void() {}
};

template<typename Allocator = void>
struct post_coroutine_promise : partial_promise<Allocator>
{
    template<typename CompletionToken>
    auto yield_value(CompletionToken cpl)
    {
        struct awaitable_t
        {
            CompletionToken cpl;
            constexpr bool await_ready() noexcept { return false; }
            BOOST_NOINLINE
            auto await_suspend(std::coroutine_handle<void> h) noexcept
            {
                auto c = std::move(cpl);
                h.destroy();
                asio::dispatch(std::move(c));
            }

            constexpr void await_resume() noexcept {}
        };
        return awaitable_t{std::move(cpl)};
    }

    std::coroutine_handle<post_coroutine_promise<Allocator>> get_return_object()
    {
        return std::coroutine_handle<post_coroutine_promise<Allocator>>::from_promise(*this);
    }

    void unhandled_exception()
    {
        std::coroutine_handle<post_coroutine_promise<Allocator>>::from_promise(*this).destroy();
        throw;
    }
};

template<typename Allocator = void>
struct dispatch_coroutine_promise : partial_promise<Allocator>
{
    template<typename CompletionToken>
    auto yield_value(CompletionToken cpl)
    {
        struct awaitable_t
        {
            CompletionToken cpl;
            constexpr bool await_ready() noexcept { return false; }

            BOOST_NOINLINE
            auto await_suspend(std::coroutine_handle<void> h) noexcept
            {
                auto c = std::move(cpl);
                h.destroy();
                asio::dispatch(std::move(c));
            }

            constexpr void await_resume() noexcept {}
        };

        return awaitable_t{std::move(cpl)};
    }

    std::coroutine_handle<dispatch_coroutine_promise<Allocator>> get_return_object()
    {
        return std::coroutine_handle<dispatch_coroutine_promise<Allocator>>::from_promise(*this);
    }

    void unhandled_exception()
    {
        std::coroutine_handle<dispatch_coroutine_promise<Allocator>>::from_promise(*this).destroy();
        throw;
    }
};


template<typename Allocator = void>
struct immediate_coroutine_promise : partial_promise<Allocator>
{
  template<typename CompletionToken>
  immediate_coroutine_promise(CompletionToken & cpl)
      : slot(asio::get_associated_cancellation_slot(cpl))
      , exec(asio::get_associated_executor(cpl, this_thread::get_executor()))
  {
  }

  template<typename CompletionToken>
  auto yield_value(CompletionToken cpl)
  {
    if (slot.is_connected())
        slot.clear();
    struct awaitable_t
    {
      CompletionToken cpl;
      constexpr bool await_ready() noexcept { return false; }
      BOOST_NOINLINE
      auto await_suspend(std::coroutine_handle<void> h) noexcept
      {
        auto c = std::move(cpl);
        h.destroy();
        std::move(c)();
      }

      constexpr void await_resume() noexcept {}
    };
    return awaitable_t{std::move(cpl)};
  }

  std::coroutine_handle<immediate_coroutine_promise<Allocator>> get_return_object()
  {
    return std::coroutine_handle<immediate_coroutine_promise<Allocator>>::from_promise(*this);
  }

  void unhandled_exception()
  {
    std::coroutine_handle<immediate_coroutine_promise<Allocator>>::from_promise(*this).destroy();
    throw;
  }

  asio::cancellation_slot slot;
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type get_cancellation_slot() const
  {
    return slot;
  }

  asio::io_context::executor_type exec;
  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() const
  {
    return exec;
  }
};

template<typename Allocator = void>
struct transactable_coroutine_promise : partial_promise<Allocator>
{
  template<typename BeginTransaction, typename CompletionToken>
  transactable_coroutine_promise(BeginTransaction & transaction, CompletionToken & cpl)
      : begin_transaction_this(&transaction)
      , begin_transaction_func(
          +[](void * ptr)
          {
            (*static_cast<BeginTransaction*>(ptr))();
          })
      , slot(asio::get_associated_cancellation_slot(cpl))
      , exec(asio::get_associated_executor(cpl, this_thread::get_executor()))
  {
  }

  void * begin_transaction_this;
  void (*begin_transaction_func)(void*);

  void begin_transaction()
  {
    begin_transaction_func(begin_transaction_this);
  }

  template<typename CompletionToken>
  auto yield_value(CompletionToken cpl)
  {
    if (slot.is_connected())
      slot.clear();
    struct awaitable_t
    {
      CompletionToken cpl;
      constexpr bool await_ready() noexcept { return false; }
      BOOST_NOINLINE
      auto await_suspend(std::coroutine_handle<void> h) noexcept
      {
        auto c = std::move(cpl);
        h.destroy();
        std::move(c)();
      }

      constexpr void await_resume() noexcept {}
    };
    return awaitable_t{std::move(cpl)};
  }

  std::coroutine_handle<transactable_coroutine_promise<Allocator>> get_return_object()
  {
    return std::coroutine_handle<transactable_coroutine_promise<Allocator>>::from_promise(*this);
  }

  void unhandled_exception()
  {
    std::coroutine_handle<transactable_coroutine_promise<Allocator>>::from_promise(*this).destroy();
    throw;
  }

  asio::cancellation_slot slot;
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type get_cancellation_slot() const
  {
    return slot;
  }

  asio::io_context::executor_type exec;
  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() const
  {
    return exec;
  }
};

}

namespace std
{

template <typename T, typename ... Args>
struct coroutine_traits<coroutine_handle<boost::async::detail::post_coroutine_promise<T>>, Args...>
{
    using promise_type = boost::async::detail::post_coroutine_promise<T>;
};

template <typename T, typename ... Args>
struct coroutine_traits<coroutine_handle<boost::async::detail::dispatch_coroutine_promise<T>>, Args...>
{
    using promise_type = boost::async::detail::dispatch_coroutine_promise<T>;
};

template <typename T, typename ... Args>
struct coroutine_traits<coroutine_handle<boost::async::detail::immediate_coroutine_promise<T>>, Args...>
{
  using promise_type = boost::async::detail::immediate_coroutine_promise  <T>;
};

template <typename T, typename ... Args>
struct coroutine_traits<coroutine_handle<boost::async::detail::transactable_coroutine_promise<T>>, Args...>
{
  using promise_type = boost::async::detail::transactable_coroutine_promise<T>;
};


} // namespace std


namespace boost::async::detail
{


template <typename CompletionToken>
auto post_coroutine(CompletionToken token)
    -> std::coroutine_handle<post_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield std::move(token);
}

template <asio::execution::executor Executor, typename CompletionToken>
auto post_coroutine(Executor exec, CompletionToken token)
    -> std::coroutine_handle<post_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(exec, std::move(token));
}

template <with_get_executor Context, typename CompletionToken>
auto post_coroutine(Context &ctx, CompletionToken token)
    -> std::coroutine_handle<post_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(ctx.get_executor(), std::move(token));
}

template <typename CompletionToken>
auto dispatch_coroutine(CompletionToken token)
    -> std::coroutine_handle<dispatch_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield std::move(token);
}

template <asio::execution::executor Executor, typename CompletionToken>
auto dispatch_coroutine(Executor exec, CompletionToken token)
    -> std::coroutine_handle<dispatch_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(exec, std::move(token));
}

template <with_get_executor Context, typename CompletionToken>
auto dispatch_coroutine(Context &ctx, CompletionToken token)
    -> std::coroutine_handle<dispatch_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(ctx.get_executor(), std::move(token));
}

template <typename CompletionToken>
auto  immediate_coroutine(CompletionToken token)
    -> std::coroutine_handle<immediate_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
  co_yield std::move(token);
}

// note this might NOT invoke the token
template<typename Awaitable, typename CompletionToken>
  requires requires (
      Awaitable & aw,
      std::coroutine_handle<immediate_coroutine_promise<asio::associated_allocator_t<CompletionToken>>> h)
      {
        {aw.await_suspend(h)} -> std::convertible_to<void>;
      }
void suspend_for_callback(Awaitable & aw, CompletionToken && ct)
{
  aw.await_suspend(immediate_coroutine(std::forward<CompletionToken>(ct)));
}

template<typename Awaitable, typename CompletionToken>
requires requires (
    Awaitable & aw,
        std::coroutine_handle<immediate_coroutine_promise<asio::associated_allocator_t<CompletionToken>>> h)
{
  {aw.await_suspend(h)} -> std::convertible_to<bool>;
}
void suspend_for_callback(Awaitable & aw, CompletionToken && ct)
{
  auto h = immediate_coroutine(std::forward<CompletionToken>(ct));
  if (!aw.await_suspend(h))
    h.resume();
}

template<typename Awaitable, typename CompletionToken>
requires requires (
    Awaitable & aw,
        std::coroutine_handle<immediate_coroutine_promise<asio::associated_allocator_t<CompletionToken>>> h)
{
  {aw.await_suspend(h)} -> std::convertible_to<std::coroutine_handle<>>;
}
void suspend_for_callback(Awaitable & aw, CompletionToken && ct)
{
  aw.await_suspend(immediate_coroutine(std::forward<CompletionToken>(ct)))
    .resume();
}

template <typename Transaction, typename CompletionToken>
auto  transactable_coroutine(Transaction transaction, CompletionToken token)
  -> std::coroutine_handle<transactable_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
  co_yield std::move(token);
}


// note this might NOT invoke the token
template<typename Awaitable, typename Transaction, typename CompletionToken>
requires requires (
    Awaitable & aw,
        std::coroutine_handle<transactable_coroutine_promise<asio::associated_allocator_t<CompletionToken>>> h)
{
  {aw.await_suspend(h)} -> std::convertible_to<void>;
}
void suspend_for_callback_with_transaction(Awaitable & aw, Transaction && t, CompletionToken && ct)
{
  aw.await_suspend(transactable_coroutine(
      std::forward<Transaction>(t),
      std::forward<CompletionToken>(ct)));
}

template<typename Awaitable, typename Transaction, typename CompletionToken>
requires requires (
    Awaitable & aw,
        std::coroutine_handle<transactable_coroutine_promise<asio::associated_allocator_t<CompletionToken>>> h)
{
  {aw.await_suspend(h)} -> std::convertible_to<bool>;
}
void suspend_for_callback_with_transaction(Awaitable & aw, Transaction && t, CompletionToken && ct)
{
  auto h = transactable_coroutine(
      std::forward<Transaction>(t),
      std::forward<CompletionToken>(ct));
  if (!aw.await_suspend(h))
    h.resume();
}

template<typename Awaitable, typename Transaction, typename CompletionToken>
requires requires (
    Awaitable & aw,
        std::coroutine_handle<transactable_coroutine_promise<asio::associated_allocator_t<CompletionToken>>> h)
{
  {aw.await_suspend(h)} -> std::convertible_to<std::coroutine_handle<>>;
}
void suspend_for_callback_with_transaction(Awaitable & aw, Transaction && t, CompletionToken && ct)
{
  aw.await_suspend(transactable_coroutine(
      std::forward<Transaction>(t),
      std::forward<CompletionToken>(ct)))
      .resume();
}


}

#endif //BOOST_ASYNC_WRAPPER_HPP
