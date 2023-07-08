// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_YEET_HPP
#define BOOST_ASYNC_YEET_HPP

#include <coroutine>
#include <boost/assert.hpp>


namespace boost::async::detail
{

#if defined(_MSC_VER)
/* MSVC is buggy when it comes to final-suspend. if you self-yeet from final_suspend().await_suspend
 * it will destruct the coro frame twice. hence we build a workaround here that allows the coro to yeet itself.
 *
 * The coro indireciton below will behave alright when destroyed multiple times.
 */

struct yeeter
{
  struct promise_type
  {
    void * operator new(const std::size_t size, std::coroutine_handle<void>, std::coroutine_handle<void> )
    {
      static thread_local char mem[4096];
      BOOST_ASSERT(size < sizeof(mem));
      return mem;
    }

    void operator delete(void *, const std::size_t ) { }

    void return_void() {}

    std::suspend_always initial_suspend() noexcept {return std::suspend_always{};}
    auto final_suspend() noexcept
    {
      struct final_awaitable
      {
        std::coroutine_handle<void> target;

        bool await_ready() const noexcept
        {
          return false;
        }

        auto await_suspend(std::coroutine_handle<promise_type> h) noexcept -> std::coroutine_handle<void>
        {
          h.destroy();
          return target;
        }

        void await_resume() noexcept
        {
        }
      };
      return final_awaitable{target};
    }

    std::coroutine_handle<void> target;

    promise_type(std::coroutine_handle<void> target,
                 std::coroutine_handle<void> yeeted) : target(target) {}

    yeeter get_return_object() { return yeeter{std::coroutine_handle<promise_type>::from_promise(*this)}; }
    void unhandled_exception() {target.destroy(); throw;}
  };

  std::coroutine_handle<promise_type> promise;
};

inline yeeter yeet_impl(
    std::coroutine_handle<void> target,
    std::coroutine_handle<void> yeeted)
{
  yeeted.destroy();
  co_return ;
}


inline std::coroutine_handle<void> yeet(
    std::coroutine_handle<void> target,
    std::coroutine_handle<void> yeeted)
{
  return yeet_impl(target, yeeted).promise;
}

#else

// that's how it's supposed to be!
inline std::coroutine_handle<void> yeet(
    std::coroutine_handle<void> target,
    std::coroutine_handle<void> yeeted)
{
  yeeted.destroy();
  return target;
}

inline void yeet(
    std::coroutine_handle<std::noop_coroutine> target,
    std::coroutine_handle<void> yeeted)
{
  yeeted.destroy();
}


#endif

}

#endif //BOOST_ASYNC_YEET_HPP
