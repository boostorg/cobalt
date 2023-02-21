//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_WAIT_HPP
#define BOOST_ASYNC_WAIT_HPP

#include <boost/async/concepts.hpp>
#include <boost/async/detail/wait.hpp>

namespace boost::async
{


template<asio::cancellation_type Ct = asio::cancellation_type::all, awaitable ... Promise>
auto wait(Promise && ... p) -> detail::wait_impl<Ct, Promise ...>
{
  return detail::wait_impl<Ct, Promise ...>{std::forward<Promise>(p)...};
}


template<asio::cancellation_type Ct = asio::cancellation_type::all, typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>>
auto wait(PromiseRange && p)
{
  return detail::ranged_wait_impl<Ct, PromiseRange>{std::forward<PromiseRange>(p)};
}




}


#endif //BOOST_ASYNC_WAIT_HPP
