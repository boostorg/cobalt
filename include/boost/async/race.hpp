  //
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_RACEa_HPP
#define BOOST_ASYNC_RACEa_HPP

#include <boost/async/concepts.hpp>
#include <boost/async/detail/race.hpp>
#include <boost/async/detail/wrapper.hpp>

namespace boost::async
{

template<asio::cancellation_type Ct = asio::cancellation_type::all,
         awaitable<detail::transactable_coroutine_promise<>> ... Promise>
auto race(Promise && ... p) -> detail::race_variadic_impl<Ct, Promise ...>
{
  return detail::race_variadic_impl<Ct, Promise ...>(static_cast<Promise&&>(p)...);
}


template<asio::cancellation_type Ct = asio::cancellation_type::all, typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>,
                     detail::transactable_coroutine_promise<>>
auto race(PromiseRange && p)
{
  if (std::empty(p))
    throw_exception(std::invalid_argument("empty range raceed"));

  return detail::race_ranged_impl<Ct, PromiseRange>{static_cast<PromiseRange&&>(p)};
}




}

#endif //BOOST_ASYNC_RACEa_HPP
