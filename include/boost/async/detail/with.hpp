// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_DETAIL_WITH_HPP
#define BOOST_ASYNC_DETAIL_WITH_HPP

#include <boost/async/detail/async_operation.hpp>
#include <boost/async/concepts.hpp>

#include <boost/asio/cancellation_signal.hpp>

namespace boost::async::detail
{

struct [[nodiscard]] with_impl
{
    struct promise_type;

    bool await_ready() { return false;}

    template<typename Promise>
    BOOST_NOINLINE auto await_suspend(std::coroutine_handle<Promise> h) -> std::coroutine_handle<promise_type>;
    inline void await_resume();

  private:
    with_impl(promise_type & promise) : promise(promise) {}
    promise_type & promise;
};

struct with_impl::promise_type
        : enable_awaitables<promise_type>,
          enable_async_operation,
          enable_await_allocator<promise_type>
{
    using enable_awaitables<promise_type>::await_transform;
    using enable_async_operation::await_transform;
    using enable_await_allocator<promise_type>::await_transform;


    using executor_type = executor;
    executor_type get_executor() const {return exec;}
    executor_type exec{this_thread::get_executor()};

    with_impl get_return_object()
    {
        return with_impl{*this};
    }

    void return_void() {}

    std::exception_ptr e;
    void unhandled_exception()
    {
        e = std::current_exception();
    }

    std::suspend_always initial_suspend() {return {};}
    auto final_suspend() noexcept
    {
        struct final_awaitable
        {
            promise_type *promise;

            bool await_ready() const noexcept
            {
                return false;
            }
            BOOST_NOINLINE
            auto await_suspend(std::coroutine_handle<promise_type> h) noexcept -> std::coroutine_handle<void>
            {
                return std::coroutine_handle<void>::from_address(h.promise().awaited_from.address());
            }

            void await_resume() noexcept
            {
            }
        };
        return final_awaitable{this};
    }
    using cancellation_slot_type = asio::cancellation_slot;
    cancellation_slot_type get_cancellation_slot() const {return slot_;}
    asio::cancellation_slot slot_;

    std::coroutine_handle<void> awaited_from{nullptr};

};

void with_impl::await_resume()
{
    auto e = promise.e;
    std::coroutine_handle<promise_type>::from_promise(promise).destroy();
    if (e)
        std::rethrow_exception(e);
}

template<typename Promise>
auto with_impl::await_suspend(std::coroutine_handle<Promise> h) -> std::coroutine_handle<promise_type>
{
    if constexpr (requires (Promise p) {p.get_executor();})
        promise.exec = h.promise().get_executor();

    if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        promise.slot_ = h.promise().get_cancellation_slot();

    promise.awaited_from = h;
    return std::coroutine_handle<promise_type>::from_promise(promise);
}

}

#endif //BOOST_ASYNC_DETAIL_WITH_HPP
