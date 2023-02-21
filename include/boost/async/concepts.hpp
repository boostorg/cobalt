// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_CONCEPTS_HPP
#define BOOST_ASYNC_CONCEPTS_HPP

#include <coroutine>
#include <concepts>

#include <boost/asio/is_executor.hpp>
#include <boost/asio/execution/executor.hpp>

namespace boost::async
{

template<typename Awaitable, typename Promise = void>
concept awaitable_type = requires (Awaitable aw, std::coroutine_handle<Promise> h)
{
    {aw.await_ready()} -> std::convertible_to<bool>;
    {aw.await_suspend(h)};
    {aw.await_resume()};
};

template<typename Awaitable, typename Promise = void>
concept awaitable =
        awaitable_type<Awaitable, Promise>
    || requires (Awaitable && aw) { {std::forward<Awaitable>(aw).operator co_await()} -> awaitable_type<Promise>;}
    || requires (Awaitable && aw) { {operator co_await(std::forward<Awaitable>(aw))} -> awaitable_type<Promise>;};



template<typename Promise = void>
struct enable_awaitables
{
    template<awaitable<Promise> Aw>
    Aw&& await_transform(Aw && aw)
    {
        return static_cast<Aw&&>(aw);
    }
};

template <typename T>
concept executor = asio::execution::executor<std::decay_t<T>> || asio::is_executor<std::decay_t<T>>::value;


template <typename T>
concept with_get_executor = requires (T& t)
{
  {t.get_executor()} -> executor;
};


}

#endif //BOOST_ASYNC_CONCEPTS_HPP
