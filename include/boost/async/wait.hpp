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


template<awaitable ... Promise>
auto wait(Promise && ... p)
{
  return detail::wait_variadic_impl<Promise ...>(
      std::forward<Promise>(p)...);
}


template<typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>>
auto wait(PromiseRange && p)
{
  using type = std::decay_t<decltype(*std::begin(std::declval<PromiseRange>()))>;
  return detail::wait_ranged_impl<PromiseRange>(std::forward<PromiseRange>(p));
}



}


#endif //BOOST_ASYNC_WAIT_HPP
