//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_AWAIT_RESULT_HELPER_HPP
#define BOOST_ASYNC_DETAIL_AWAIT_RESULT_HELPER_HPP

#include <utility>

namespace boost::async::detail
{

template<awaitable_type T>
auto co_await_result_helper() -> decltype(std::declval<T>());

template<typename T>
auto co_await_result_helper() -> decltype(std::declval<T>().operator co_await());

template<typename T>
auto co_await_result_helper() -> decltype(operator co_await(std::declval<T>()));

template<typename T>
using co_awaitable_type = decltype(co_await_result_helper<T>());

template<typename T>
using co_await_result_t = decltype(co_await_result_helper<T>().await_resume());

template<awaitable_type T>
T&& get_awaitable_type(T && t) { return std::forward<T>(t);}

template<typename T>
  requires (requires (T && t) {{operator co_await(std::forward<T>(t))} -> awaitable_type;} )
decltype(auto) get_awaitable_type(T && t) { return operator co_await(std::forward<T>(t));}

template<typename T>
requires (requires (T && t) {{std::forward<T>(t).operator co_await()} -> awaitable_type;} )
decltype(auto) get_awaitable_type(T && t)  { return std::forward<T>(t).operator co_await();}


}

#endif //BOOST_ASYNC_DETAIL_AWAIT_RESULT_HELPER_HPP
