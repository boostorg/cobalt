//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_OPS_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_OPS_HPP

#include <boost/cobalt/concepts.hpp>
#include <boost/cobalt/detail/await_result_helper.hpp>

namespace boost::cobalt::experimental::io
{

template<typename Awaitable>
concept wait_op =
    awaitable<Awaitable> &&
    std::same_as<
        detail::co_await_result_t<as_tuple_t<decltype(detail::get_awaitable_type(std::declval<Awaitable>()))>>,
        std::tuple<system::error_code>>;


template<typename Awaitable>
concept transfer_op =
    awaitable<Awaitable> &&
    std::same_as<
        detail::co_await_result_t<as_tuple_t<decltype(detail::get_awaitable_type(std::declval<Awaitable>()))>>,
        std::tuple<system::error_code, std::size_t>>;

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_OPS_HPP
