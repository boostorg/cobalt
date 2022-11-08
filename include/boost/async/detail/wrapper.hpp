// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_WRAPPER_HPP
#define BOOST_ASYNC_WRAPPER_HPP


#include <boost/asio/bind_executor.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>

#include <boost/async/this_coro.hpp>
#include "concepts.hpp"
#include "util.hpp"

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
      return allocate_coroutine(size, asio::get_associated_allocator(token));
    }

    template<typename Executor, typename CompletionToken>
    void * operator new(const std::size_t size, Executor & exec, CompletionToken & token)
    {
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
};

template<typename Allocator = void>
struct post_coroutine_promise : partial_promise<Allocator>
{
    template<typename CompletionToken>
    auto yield_value(CompletionToken
    cpl)
    {
        struct awaitable_t
        {
            CompletionToken cpl;
            constexpr bool await_ready() noexcept { return false; }
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
            auto await_suspend(std::coroutine_handle<void> h) noexcept
            {

                auto c = std::move(cpl);
                h.destroy();
                auto exec = asio::get_associated_executor(c);
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

} // namespace std


namespace boost::async::detail
{


template <typename CompletionToken>
auto post_coroutine(CompletionToken token) noexcept
    -> std::coroutine_handle<post_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield std::move(token);
}

template <asio::execution::executor Executor, typename CompletionToken>
auto post_coroutine(Executor exec, CompletionToken token) noexcept
    -> std::coroutine_handle<post_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(exec, std::move(token));
}

template <with_get_executor Context, typename CompletionToken>
auto post_coroutine(Context &ctx, CompletionToken token) noexcept
    -> std::coroutine_handle<post_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(ctx.get_executor(), std::move(token));
}

template <typename CompletionToken>
auto dispatch_coroutine(CompletionToken token) noexcept
    -> std::coroutine_handle<dispatch_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield std::move(token);
}

template <asio::execution::executor Executor, typename CompletionToken>
auto dispatch_coroutine(Executor exec, CompletionToken token) noexcept
    -> std::coroutine_handle<dispatch_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(exec, std::move(token));
}

template <with_get_executor Context, typename CompletionToken>
auto dispatch_coroutine(Context &ctx, CompletionToken token) noexcept
    -> std::coroutine_handle<dispatch_coroutine_promise<asio::associated_allocator_t<CompletionToken>>>
{
    co_yield asio::bind_executor(ctx.get_executor(), std::move(token));
}


}

#endif //BOOST_ASYNC_WRAPPER_HPP
