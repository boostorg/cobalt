// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_LATCH_HPP
#define CORO_LATCH_HPP

#include <coro/concepts.hpp>

#include <asio/cancellation_signal.hpp>

#include <coroutine>
#include <optional>
#include <exception>

namespace coro
{

template<typename T, typename Executor>
struct latch;


template<typename T, typename Executor = asio::any_io_executor>
struct latch_promise
{
    latch<T, Executor> & l;

    using executor_type = Executor;
    executor_type get_executor() const {return l.get_executor();}

    using cancellation_slot_type = asio::cancellation_slot;
    cancellation_slot_type get_cancellation_slot() { return l.get_cancellation_slot(); }


    template<typename Aw>
    void * operator new(const std::size_t size, latch<T, Executor> & l, Aw & )
    {
        if (size > 512)
            return new char[size];
        else
            return &l.buffer;
    }

    void operator delete(void * raw, const std::size_t size)
    {
        if (size > 512)
            delete [] static_cast<char*>(raw);
    }

    template<typename Aw>
    latch_promise(latch<T, Executor> & l, Aw &) : l(l) {}

    constexpr static std::suspend_never initial_suspend() {return {};}
    auto final_suspend() noexcept
    {
        struct cont {
            latch<T, Executor> & l;
            bool await_ready() noexcept {return !l.waiter; }
            auto await_suspend(std::coroutine_handle<latch_promise> h) noexcept
            {
                h.destroy();
                return std::exchange(l.waiter, nullptr);
            }
            void await_resume() noexcept {}
        };
        return cont{l};
    }

    void return_void() {}
    void unhandled_exception() {}
    void get_return_object() {}
};

/// low level latch - MUST be awaited before destruction - and only be used once!
template<typename T,
         typename Executor = asio::any_io_executor>
struct latch
{
    using executor_type = Executor;
    executor_type get_executor() const {return exec;}

    executor_type exec;
    asio::cancellation_signal signal;
    void cancel(asio::cancellation_type tp = asio::cancellation_type::all)
    {
        signal.emit(tp);
    }

    using cancellation_slot_type = asio::cancellation_slot;
    cancellation_slot_type get_cancellation_slot() { return signal.slot(); }

    latch(executor_type exec = {}) : exec(exec) {}
    latch(latch &&) = delete;
    latch(const latch &) = delete;

    std::exception_ptr ex;
    std::conditional_t<std::is_void_v<T>, bool, std::optional<T>> result{};

    std::coroutine_handle<void> waiter{nullptr};

    std::aligned_storage_t<512u> buffer;

    template<awaitable Aw>
    void await(Aw aw)
    try {
        if constexpr (std::is_void_v<T>)
        {
            co_await aw;
            result = true;
        }
        else
            result.emplace(co_await aw);
    }
    catch (...)
    {
        ex = std::current_exception();
    }

    bool await_ready() const {return !!result || !!ex;}
    void await_suspend(std::coroutine_handle<void> h)
    {
        waiter = h;
    }
    auto await_resume()
    {
        if (ex)
            std::rethrow_exception(std::exchange(ex, nullptr));
        if constexpr (std::is_void_v<T>)
            result = false;
        else
            return std::move(std::exchange(result, std::nullopt).value());
    }
};


}

namespace std
{

template<typename T, typename Executor, coro::awaitable Aw>
struct coroutine_traits<void, coro::latch<T, Executor>&, Aw>
{
    using promise_type = coro::latch_promise<T, Executor>;
};

}

#endif //CORO_LATCH_HPP
