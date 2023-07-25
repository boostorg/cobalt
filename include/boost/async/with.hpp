//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_WITH_HPP
#define BOOST_ASYNC_WITH_HPP

#include <exception>
#include <utility>
#include <boost/async/detail/util.hpp>
#include <boost/async/detail/with.hpp>


namespace boost::async
{

namespace detail
{

template<typename T>
auto invoke_await_exit(T && t, std::exception_ptr & e)
{
  return std::forward<T>(t).await_exit(e);
}

}


template<typename Arg, typename Func, typename Teardown>
    requires (requires (Func func, Arg & arg, Teardown & teardown, std::exception_ptr ep)
    {
        {std::move(func)(arg)} -> awaitable<detail::with_impl::promise_type>;
        {std::move(teardown)(std::move(arg), ep)} -> awaitable<detail::with_impl::promise_type>;
    })
auto with(Arg arg, Func func, Teardown teardown) -> detail::with_impl
{
    std::exception_ptr e;
    try
    {
        co_await std::move(func)(arg);
    }
    catch (...)
    {
        e = std::current_exception();
    }

    try
    {
        co_await std::move(teardown)(std::move(arg), e);
    }
    catch (...)
    {
        if (!e)
            e = std::current_exception();
    }
    if (e)
        std::rethrow_exception(e);
}


template<typename Arg, typename Func, typename Teardown>
    requires (requires (Func func, Arg & arg, Teardown & teardown, std::exception_ptr e)
    {
        {std::move(func)(arg)} -> std::convertible_to<void>;
        {std::move(teardown)(std::move(arg), e)} -> awaitable<detail::with_impl::promise_type>;

    })
auto with(Arg arg, Func func, Teardown teardown) -> detail::with_impl
{
    std::exception_ptr e;
    try
    {
        std::move(func)(arg);
    }
    catch (...)
    {
        e = std::current_exception();
    }

    try
    {
        co_await std::move(teardown)(arg, e);
    }
    catch (...)
    {
        if (!e)
            e = std::current_exception();
    }
    if (e)
        std::rethrow_exception(e);
}


template<typename Arg, typename Func>
  requires requires  (Arg args, std::exception_ptr ep) {{std::move(args).await_exit(ep)} -> awaitable<detail::with_impl::promise_type>;}
auto with(Arg && arg, Func && func) -> detail::with_impl
{
  return with(std::forward<Arg>(arg), std::forward<Func>(func), &detail::invoke_await_exit<Arg>);
}

}


#endif //BOOST_ASYNC_WITH_HPP
