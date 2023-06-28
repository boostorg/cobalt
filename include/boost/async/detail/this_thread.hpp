//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_THIS_THREAD_HPP
#define BOOST_ASYNC_DETAIL_THIS_THREAD_HPP

#include <boost/async/this_thread.hpp>

#include <boost/asio/uses_executor.hpp>
#include <boost/mp11/algorithm.hpp>

namespace boost::async::detail
{

inline executor_type
extract_executor(executor_type exec) { return exec; }

executor_type
extract_executor(asio::any_io_executor exec);


template<typename ... Args>
executor_type get_executor_from_args(Args &&... args)
{
  using args_type = mp11::mp_list<std::decay_t<Args>...>;
  constexpr static auto I = mp11::mp_find<args_type, asio::executor_arg_t>::value;
  if constexpr (sizeof...(Args) == I)
    return this_thread::get_executor();
  else  //
    return extract_executor(std::get<I + 1u>(std::tie(args...)));
}

template<typename ... Args>
container::pmr::memory_resource * get_memory_resource_from_args(Args &&... args)
{
  using args_type = mp11::mp_list<std::decay_t<Args>...>;
  constexpr static auto I = mp11::mp_find<args_type, std::allocator_arg_t>::value;
  if constexpr (sizeof...(Args) == I)
    return this_thread::get_default_resource();
  else  //
    return std::get<I + 1u>(std::tie(args...)).resource();
}

}

#endif //BOOST_ASYNC_DETAIL_THIS_THREAD_HPP
