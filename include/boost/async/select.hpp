//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_SELECT_HPP
#define BOOST_ASYNC_SELECT_HPP

#include <boost/async/concepts.hpp>
#include <boost/async/detail/select.hpp>

namespace boost::async
{

template<asio::cancellation_type Ct = asio::cancellation_type::all, awaitable ... Promise>
auto select(Promise && ... p) -> detail::select_impl<Ct, Promise ...>
{
  return detail::select_impl<Ct, Promise ...>{std::forward<Promise>(p)...};
}


template<asio::cancellation_type Ct = asio::cancellation_type::all, typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>>
auto select(PromiseRange && p)
{
  if (std::empty(p))
    throw_exception(std::invalid_argument("empty range selected"));

  return detail::ranged_select_impl<Ct, PromiseRange>{std::forward<PromiseRange>(p)};
}




}

#endif //BOOST_ASYNC_SELECT_HPP
