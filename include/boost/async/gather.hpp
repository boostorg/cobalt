//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_GATHER_HPP
#define BOOST_ASYNC_GATHER_HPP

#include <boost/async/concepts.hpp>
#include <boost/async/detail/gather.hpp>

namespace boost::async
{


template<awaitable<detail::immediate_coroutine_promise<>> ... Promise>
auto gather(Promise && ... p)
{
  return detail::gather_variadic_impl<Promise ...>(
      static_cast<Promise&&>(p)...);
}


template<typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>,
                     detail::immediate_coroutine_promise<>>
auto gather(PromiseRange && p)
{
  return detail::gather_ranged_impl<PromiseRange>{static_cast<PromiseRange&&>(p)};
}



}


#endif //BOOST_ASYNC_GATHER_HPP
