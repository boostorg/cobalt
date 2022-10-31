// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_LATCH_HPP
#define CORO_LATCH_HPP

#include <coro/concepts.hpp>
#include <coro/async_operation.hpp>

#include <asio/cancellation_signal.hpp>

#include <coroutine>
#include <optional>
#include <exception>
#include <memory_resource>

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
        return l.resource.allocate(size);
    }

    void operator delete(void * raw, const std::size_t size)
    {
        // noop bc monotonic resource
    }

    template<typename Aw>
    latch_promise(latch<T, Executor> & l, Aw &) : l(l) {}

    constexpr static std::suspend_never initial_suspend() {return {};}
    auto final_suspend() noexcept
    {
        struct cont
        {
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
    void unhandled_exception()
    {
        //whut to do here?
    }
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

    alignas(sizeof(void*)) char buffer[1024];
    std::pmr::monotonic_buffer_resource resource{buffer, 1024};

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

    struct awaitable_t
    {
        latch & l;
        bool await_ready() const {return !!l.result || !!l.ex;}
        void await_suspend(std::coroutine_handle<void> h)
        {
            l.waiter = h;
        }
        auto await_resume()
        {
            if (l.ex)
                std::rethrow_exception(std::exchange(l.ex, nullptr));
            if constexpr (std::is_void_v<T>)
                l.result = false;
            else
                return std::move(std::exchange(l.result, std::nullopt).value());
        }
    };

    awaitable_t operator co_await() {return awaitable_t{*this};}
};

struct enable_yielding_tasks
{
    struct task;
    struct task_reference
    {
        task & task_;
        struct promise_type;

        constexpr static bool await_ready() {return true;}
        static void await_suspend(std::coroutine_handle<void> h) {}
        task & await_resume() const
        {
            return task_;
        }
    };

    template<async_operation Op>
    auto yield_value(Op && op) -> task_reference
    {
        co_await std::forward<Op>(op);
    }

    template<awaitable Aw>
    auto yield_value(Aw && aw) -> task_reference
    {
        co_await std::forward<Aw>(aw);
    }

    auto final_suspend() noexcept;



};

struct enable_yielding_tasks::task_reference::promise_type
{
    std::suspend_never  initial_suspend() const {return {};}
    std::suspend_always final_suspend() const noexcept {return {};}
    void return_void() {}

    template<async_operation Op>
    auto await_transform(Op && op) -> std::suspend_always
    {

    }

    template<awaitable Aw>
    auto await_transform(Aw && aw) -> std::suspend_always
    {

    }
};


struct enable_yielding_tasks::task
{

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
