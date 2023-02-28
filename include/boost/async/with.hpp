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

struct with_exit_tag { };

// tag_invoke(with_exit_tag, value, std::exception_ptr)

/// default tag_invoke for any thing with an `await_exit` method
template<typename T>
    requires (requires (T &t) {{t.await_exit()}  -> awaitable<detail::with_impl::promise_type>; })
auto tag_invoke(const boost::async::with_exit_tag & wet , T && t, std::exception_ptr)
{
    return std::forward<T>(t).await_exit();
}

template<typename T>
requires (requires (T &t, std::exception_ptr e) {{t.await_exit(e)}  -> awaitable<detail::with_impl::promise_type>; })
auto tag_invoke(const boost::async::with_exit_tag & wet , T && t, std::exception_ptr e)
{
    return std::forward<T>(t).await_exit(e);
}

template<typename Arg, typename Func>
    requires (requires (Func func, Arg & arg)
    {
        {std::move(func)(arg)} -> awaitable<detail::with_impl::promise_type>;
    })
auto with(Arg arg, Func func) -> detail::with_impl
{
    std::exception_ptr e;
    try
    {
        co_await std::move(func)(arg);
        co_await tag_invoke(with_exit_tag{}, std::move(arg), e);
    }
    catch (...)
    {
        e = std::current_exception();
    }

    try
    {
        co_await tag_invoke(with_exit_tag{}, std::move(arg), e);
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
    requires (requires (Func func, Arg & arg)
    {
        {std::move(func)(arg)} -> std::convertible_to<void>;
    })
auto with(Arg arg, Func func) -> detail::with_impl
{
    std::exception_ptr e;
    try
    {
        std::move(func)(arg);
        co_await tag_invoke(with_exit_tag{}, arg, e);
    }
    catch (...)
    {
        e = std::current_exception();
    }

    try
    {
        co_await tag_invoke(with_exit_tag{}, arg, e);
    }
    catch (...)
    {
        if (!e)
            e = std::current_exception();
    }
    if (e)
        std::rethrow_exception(e);
}

}


#endif //BOOST_ASYNC_WITH_HPP
