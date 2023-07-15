  //
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_LEFT_SELECT_HPP
#define BOOST_ASYNC_LEFT_SELECT_HPP

#include <boost/async/concepts.hpp>
#include <boost/async/detail/left_select.hpp>
#include <boost/async/detail/wrapper.hpp>

namespace boost::async
{

template<asio::cancellation_type Ct = asio::cancellation_type::all,
         awaitable<detail::transactable_coroutine_promise<>> ... Promise>
auto left_select(Promise && ... p) -> detail::left_select_variadic_impl<Ct, Promise ...>
{
  return detail::left_select_variadic_impl<Ct, Promise ...>(static_cast<Promise&&>(p)...);
}


template<asio::cancellation_type Ct = asio::cancellation_type::all, typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>,
                     detail::transactable_coroutine_promise<>>
auto left_select(PromiseRange && p)
{
  if (std::empty(p))
    throw_exception(std::invalid_argument("empty range left_selected"));

  return detail::left_select_ranged_impl<Ct, PromiseRange>{static_cast<PromiseRange&&>(p)};
}




}

#endif //BOOST_ASYNC_LEFT_SELECT_HPP
