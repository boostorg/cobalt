// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_CONCEPTS_HPP
#define CORO_CONCEPTS_HPP

#include <coroutine>
#include <concepts>

#include <asio/is_executor.hpp>
#include <asio/execution/executor.hpp>

namespace asio { struct any_io_executor; }

namespace coro
{

template<typename T>
struct synchronous_awaitable : std::false_type {};

template<typename Awaitable, typename Promise = void>
concept awaitable_type = requires (Awaitable aw, std::coroutine_handle<Promise> h)
{
    {aw.await_ready()} -> std::convertible_to<bool>;
    {aw.await_suspend(h)};
    {aw.await_resume()};
};

template<typename Awaitable, typename Promise = void>
concept immediate_awaitable_type =
        awaitable_type<Awaitable, Promise> &&
        (
            synchronous_awaitable<Awaitable>::value ||
            requires (Awaitable aw) {{
                std::bool_constant<std::declval<Awaitable>().await_ready()>{}
            } -> std::convertible_to<std::true_type>;}
        );


template<typename Awaitable, typename Promise = void>
concept awaitable =
        awaitable_type<Awaitable, Promise>
    || requires (Awaitable && aw) { {std::forward<Awaitable>(aw).operator co_await()} -> awaitable_type<Promise>;}
    || requires (Awaitable && aw) { {operator co_await(std::forward<Awaitable>(aw))} -> awaitable_type<Promise>;};

template<typename Awaitable, typename Promise = void>
concept ready_awaitable =
    (awaitable_type<Awaitable, Promise> && std::declval<Awaitable>().await_ready())
    ||
    (
        requires (Awaitable aw) { {aw.operator co_await()} -> awaitable_type<Promise>;}
        && std::declval<Awaitable>().operator co_await().await_ready()
    )
    ||
    (
        requires (Awaitable aw) { {operator co_await(aw)} -> awaitable_type<Promise>;}
        && operator co_await(std::declval<Awaitable>()).await_ready()
    );


template<typename Promise = void>
struct enable_awaitables
{
    template<awaitable<Promise> Aw>
    Aw&& await_transform(Aw && aw)
    {
        return static_cast<Aw&&>(aw);
    }
};

template<typename Promise = void>
struct enable_immediate_awaitables
{
    template<ready_awaitable<Promise> Aw>
    Aw await_transform(Aw && aw)
    {
        return static_cast<Aw>(aw);
    }
};

template <typename T>
concept executor = asio::execution::executor<std::decay_t<T>> || asio::is_executor<std::decay_t<T>>::value;

template <typename T, typename Executor = asio::any_io_executor>
concept execution_context = requires (T& t)
{
    {t.get_executor()} -> std::convertible_to<Executor>;
};

}

#endif //CORO_CONCEPTS_HPP
