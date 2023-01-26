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

template<typename T>
auto co_await_result_helper() -> decltype(std::declval<T>().await_resume());

template<typename T>
auto co_await_result_helper() -> decltype(std::declval<T>().operator co_await().await_resume());

template<typename T>
auto co_await_result_helper() -> decltype(operator co_await(std::declval<T>()).await_resume());

template<typename T>
using co_await_result_t = decltype(co_await_result_helper<T>());

}

#endif //BOOST_ASYNC_DETAIL_AWAIT_RESULT_HELPER_HPP
