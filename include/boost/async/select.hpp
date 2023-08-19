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
#include <boost/async/detail/wrapper.hpp>
#include <random>

namespace boost::async
{

namespace detail
{

inline std::mt19937 &random_device()
{
  thread_local static std::random_device rd;
  thread_local static std::mt19937 g(rd());
  return g;
}

}
// tag::concept[]
template<typename G>
  concept uniform_random_bit_generator =
    requires ( G & g)
    {
      {typename std::decay_t<G>::result_type() } -> std::unsigned_integral; // is an unsigned integer type
      // T	Returns the smallest value that G's operator() may return. The value is strictly less than G::max(). The function must be constexpr.
      {std::decay_t<G>::min()} -> std::same_as<typename std::decay_t<G>::result_type>;
      // T	Returns the largest value that G's operator() may return. The value is strictly greater than G::min(). The function must be constexpr.
      {std::decay_t<G>::max()} -> std::same_as<typename std::decay_t<G>::result_type>;
      {g()} -> std::same_as<typename std::decay_t<G>::result_type>;
    } && (std::decay_t<G>::max() > std::decay_t<G>::min());

// end::concept[]


template<asio::cancellation_type Ct = asio::cancellation_type::all,
    uniform_random_bit_generator URBG,
    awaitable<detail::fork::promise_type> ... Promise>
auto select(URBG && g, Promise && ... p) -> detail::select_variadic_impl<Ct, URBG, Promise ...>
{
  return detail::select_variadic_impl<Ct, URBG, Promise ...>(std::forward<URBG>(g), static_cast<Promise&&>(p)...);
}


template<asio::cancellation_type Ct = asio::cancellation_type::all,
    uniform_random_bit_generator URBG,
    typename PromiseRange>
requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>,
    detail::fork::promise_type>
auto select(URBG && g, PromiseRange && p) -> detail::select_ranged_impl<Ct, URBG, PromiseRange>
{
  if (std::empty(p))
    throw_exception(std::invalid_argument("empty range selected"));

  return detail::select_ranged_impl<Ct, URBG, PromiseRange>{std::forward<URBG>(g), static_cast<PromiseRange&&>(p)};
}

template<asio::cancellation_type Ct = asio::cancellation_type::all,
         awaitable<detail::fork::promise_type> ... Promise>
auto select(Promise && ... p) -> detail::select_variadic_impl<Ct, std::mt19937&, Promise ...>
{
  return select<Ct>(detail::random_device(), static_cast<Promise&&>(p)...);
}


template<asio::cancellation_type Ct = asio::cancellation_type::all, typename PromiseRange>
  requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>,
      detail::fork::promise_type>
auto select(PromiseRange && p) -> detail::select_ranged_impl<Ct, std::mt19937&, PromiseRange>
{
  if (std::empty(p))
    throw_exception(std::invalid_argument("empty range selected"));

  return select<Ct>(detail::random_device(), static_cast<PromiseRange&&>(p));
}

template<asio::cancellation_type Ct = asio::cancellation_type::all,
    awaitable<detail::fork::promise_type> ... Promise>
auto left_select(Promise && ... p) -> detail::select_variadic_impl<Ct, detail::left_select_tag, Promise ...>
{
  return detail::select_variadic_impl<Ct, detail::left_select_tag, Promise ...>(
      detail::left_select_tag{}, static_cast<Promise&&>(p)...);
}


template<asio::cancellation_type Ct = asio::cancellation_type::all, typename PromiseRange>
requires awaitable<std::decay_t<decltype(*std::declval<PromiseRange>().begin())>,
    detail::fork::promise_type>
auto left_select(PromiseRange && p)  -> detail::select_ranged_impl<Ct, detail::left_select_tag, PromiseRange>
{
  if (std::empty(p))
    throw_exception(std::invalid_argument("empty range left_selected"));

  return detail::select_ranged_impl<Ct, detail::left_select_tag, PromiseRange>{
      detail::left_select_tag{}, static_cast<PromiseRange&&>(p)};
}




}

#endif //BOOST_ASYNC_SELECT_HPP
