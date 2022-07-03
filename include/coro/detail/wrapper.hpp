// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_WRAPPER_HPP
#define CORO_WRAPPER_HPP

#include <asio/executor.hpp>
#include <asio/dispatch.hpp>
#include <asio/post.hpp>

#include <coro/allocator.hpp>
#include <coro/coro_traits.hpp>
#include <coro/util.hpp>

#include <coroutine>
#include <utility>

namespace coro::detail
{

template<typename Allocator>
struct partial_promise_base
{
    template<typename ... Args>
    void * operator new(const std::size_t size, Args & ... args)
    {
        return allocate_coroutine(size, asio::get_associated_allocator(get_last_variadic(args...)));
    }

    void operator delete(void * raw, const std::size_t size)
    {
        deallocate_coroutine<Allocator>(raw, size);
    }
};

template<> struct partial_promise_base<void> {};

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
        struct awaitable_t
        {
            constexpr bool await_ready() noexcept { return true; }

            auto await_suspend(std::coroutine_handle<partial_promise> h) noexcept
            {
                h.destroy();
            }

            constexpr void await_resume() noexcept {}
        };

        return awaitable_t{};
    }


    void return_void() {}

    std::coroutine_handle<partial_promise> get_return_object()
    {
        return std::coroutine_handle<partial_promise>::from_promise(*this);
    }

    void unhandled_exception()
    {
        std::coroutine_handle<partial_promise>::from_promise(*this).destroy();
        throw;
    }
};


// alloc options are two: allocator or aligned storage
struct static_partial_promise
{
    auto initial_suspend() noexcept
    {
        return std::suspend_always();
    }

    auto final_suspend() noexcept
    {
        struct awaitable_t
        {
            constexpr bool await_ready() noexcept { return true; }

            auto await_suspend(std::coroutine_handle<static_partial_promise> h) noexcept
            {
                h.destroy();
            }

            constexpr void await_resume() noexcept {}
        };

        return awaitable_t{};
    }

    template<typename T, typename U>
    void * operator new(const std::size_t size, T&, U&, void * ptr, std::size_t max_size) noexcept
    {
        if (size > max_size)
            return nullptr;
        return ptr;
    }

    static std::coroutine_handle<static_partial_promise> get_return_object_on_allocation_failure()
    {
        return nullptr;
    }

    void operator delete(void * raw, const std::size_t size) { }
    void return_void() {}

    std::coroutine_handle<static_partial_promise> get_return_object()
    {
        return std::coroutine_handle<static_partial_promise>::from_promise(*this);
    }

    void unhandled_exception()
    {
        std::coroutine_handle<static_partial_promise>::from_promise(*this).destroy();
        throw;
    }
};

}

namespace std {

template <typename T, typename ... Args>
struct coroutine_traits<coroutine_handle<coro::detail::partial_promise<T>>, Args...>
{
    using promise_type = coro::detail::partial_promise<T>;
};

template <typename ... Args>
struct coroutine_traits<coroutine_handle<coro::detail::static_partial_promise>, Args...>
{
    using promise_type = coro::detail::static_partial_promise;
};


} // namespace std


namespace coro::detail
{


template <typename CompletionToken>
auto post_coroutine(CompletionToken token) noexcept
    -> std::coroutine_handle<partial_promise<asio::associated_allocator_t<CompletionToken>>>
{
    asio::post(std::move(token));
    co_return;
}

template <asio::execution::executor Executor, typename CompletionToken>
auto post_coroutine(Executor exec, CompletionToken token) noexcept
    -> std::coroutine_handle<partial_promise<asio::associated_allocator_t<CompletionToken>>>
{
    asio::post(exec, std::move(token));
    co_return;
}

template <detail::with_get_executor Context, typename CompletionToken>
auto post_coroutine(Context &ctx, CompletionToken token) noexcept
    -> std::coroutine_handle<partial_promise<asio::associated_allocator_t<CompletionToken>>>
{
    asio::post(ctx, std::move(token));
    co_return;
}

template <typename CompletionToken>
auto dispatch_coroutine(CompletionToken token) noexcept
    -> std::coroutine_handle<partial_promise<asio::associated_allocator_t<CompletionToken>>>
{
    asio::dispatch(std::move(token));
    co_return;
}

template <asio::execution::executor Executor, typename CompletionToken>
auto dispatch_coroutine(Executor exec, CompletionToken token) noexcept
    -> std::coroutine_handle<partial_promise<asio::associated_allocator_t<CompletionToken>>>
{
    asio::dispatch(exec, std::move(token));
    co_return;
}

template <detail::with_get_executor Context, typename CompletionToken>
auto dispatch_coroutine(Context &ctx, CompletionToken token) noexcept
    -> std::coroutine_handle<partial_promise<asio::associated_allocator_t<CompletionToken>>>
{
    asio::dispatch(ctx, std::move(token));
    co_return;
}

/// these are optimizations to avoid allocations. IF the raw memory is not enough it returns a null handle!

template <asio::execution::executor Executor, typename CompletionToken>
auto post_coroutine(Executor exec, CompletionToken token, void*, std::size_t) noexcept
    -> std::coroutine_handle<static_partial_promise>
{
    asio::post(exec, std::move(token));
    co_return;
}

template <asio::execution::executor Executor, typename CompletionToken>
auto dispatch_coroutine(Executor exec, CompletionToken token, void*, std::size_t) noexcept
    -> std::coroutine_handle<static_partial_promise>
{
    asio::dispatch(exec, std::move(token));
    co_return;
}


/// This might compied the completion handler twice!
template <asio::execution::executor Executor, typename CompletionToken>
auto try_post_coroutine(Executor exec, CompletionToken token, void* ptr, std::size_t sz) noexcept
    -> std::coroutine_handle<void>
{
    auto p = post_coroutine(exec, std::move(token), ptr, sz);

    if (p)
        return p;
    else
        return post_coroutine(exec, std::move(token));

}

template <asio::execution::executor Executor, typename CompletionToken>
auto try_dispatch_coroutine(Executor exec, CompletionToken token, void* ptr, std::size_t sz) noexcept
    -> std::coroutine_handle<void>
{
    auto p = dispatch_coroutine(exec, std::move(token), ptr, sz);
        if (p)
        return p;
    else
        return dispatch_coroutine(exec, std::move(token));

}

}

#endif //CORO_WRAPPER_HPP
