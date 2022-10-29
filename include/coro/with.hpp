//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CORO_WITH_HPP
#define CORO_WITH_HPP

#include <exception>
#include <utility>
#include <coro/util.hpp>

namespace coro
{

struct with_enter_tag {};
struct with_exit_tag { std::exception_ptr e; };

namespace detail
{

template<std::size_t ... Idx>
struct with_impl
{
    template<typename ... Args>
    static auto with(Args && ... args)
       -> decltype(get_last_variadic(args...)(get_variadic<Idx>(args...)...))
    {
        std::exception_ptr e;
        co_await tag_invoke(with_enter_tag{}, get_variadic<Idx>(args...)...);
        try
        {
            co_await get_last_variadic(args...)(get_variadic<Idx>(args...)...);
            co_await tag_invoke(with_exit_tag{e}, get_variadic<Idx>(args...)...);
        }
        catch (...)
        {
            e = std::current_exception();
        }

        try
        {
            co_await tag_invoke(with_exit_tag{e}, get_variadic<Idx>(args...)...);
        }
        catch (...)
        {
            if (!e)
                e = std::current_exception();
        }
        if (e)
            std::rethrow_exception(e);
    }
};

template<typename T>
struct get_with_impl;

template<std::size_t ... Idx>
struct get_with_impl<std::index_sequence<Idx...>>
{
    using type = with_impl<Idx...>;
};

}


template<typename ... Args>
auto with(Args &&... args)
{
    using impl = typename detail::get_with_impl<std::make_index_sequence<sizeof...(Args) - 1>>::type;
    return impl::with(args...);
}

}

#endif //CORO_WITH_HPP
