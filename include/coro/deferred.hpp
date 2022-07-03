// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_DEFERRED_HPP
#define CORO_DEFERRED_HPP

#include <asio/associated_allocator.hpp>
#include <asio/associated_cancellation_slot.hpp>
#include <asio/associated_executor.hpp>
#include <asio/as_tuple.hpp>
#include <asio/deferred.hpp>
#include <coroutine>
#include <coro/handler.hpp>

namespace coro
{
using asio::deferred;
constexpr auto deferred_tup = asio::as_tuple(asio::deferred);

template<typename Signature, typename ... Ts>
struct awaitable_deferred;

template<typename ... Args, typename ... Ts>
struct awaitable_deferred<void(Args...), Ts...>
{
    asio::deferred_async_operation<void(Args...), Ts...>  op;
    std::optional<std::tuple<Args...>> result;

    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        using completion = completion_handler<Promise, Args...>;
        std::move(op)(completion{h, result});
    }

    auto await_resume()
    {
        return std::move(result.value()) ;
    }
};

struct enable_deferred
{
    template<typename ... Ts>
    auto await_transform(asio::deferred_async_operation< Ts...> && op)
    {
        return awaitable_deferred<Ts...>{std::move(op)};
    }
};


template<typename Signature, typename ... Ts>
struct awaitable_deferred_interpreted;

template<typename ... Args, typename ... Ts>
struct awaitable_deferred_interpreted<void(Args...), Ts...>
{
    asio::deferred_async_operation<void(Args...), Ts...>  op;
    std::optional<std::tuple<Args...>> result;

    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        detail::throw_if_cancelled_impl(h.promise());
        using completion = completion_handler<Promise, Args...>;
        std::move(op)(completion{h, result});
    }

    auto await_resume()
    {
        return interpret_result(std::move(*result));
    }
};

template<typename Signature, typename ... Ts>
awaitable_deferred_interpreted(asio::deferred_async_operation<Signature, Ts...>)
                                          -> awaitable_deferred_interpreted<Signature, Ts...>;

struct enable_deferred_interpreted
{
    template<typename ... Ts>
    auto await_transform(asio::deferred_async_operation< Ts...> && op)
    {
        return awaitable_deferred_interpreted<Ts...>{std::move(op)};
    }
};

}

#endif //CORO_DEFERRED_HPP
