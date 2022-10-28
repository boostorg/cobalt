// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_EXECUTOR_HPP
#define CORO_EXECUTOR_HPP

#include <asio/associated_executor.hpp>
#include <asio/executor.hpp>

namespace coro
{


namespace detail
{

template<typename CompletionHandler>
auto pick_executor(CompletionHandler &&completion_handler)
{
    return asio::get_associated_executor(completion_handler);
}

template<typename CompletionHandler,
        typename First,
        typename ... IoObjectsOrExecutors>
auto pick_executor_impl(rank<1>,
                        CompletionHandler &completion_handler,
                        First &first,
                        IoObjectsOrExecutors &... io_objects_or_executors)
        -> asio::associated_executor_t<std::decay_t<CompletionHandler>, typename First::executor_type>
{
    return asio::get_associated_executor(completion_handler, first.get_executor());
}


template<typename CompletionHandler,
        typename First,
        typename ... IoObjectsOrExecutors>
    requires (asio::is_executor<First>::value && asio::execution::is_executor_v<First>)
auto pick_executor_impl(rank<1>,
                        CompletionHandler &completion_handler,
                        First &first,
                        IoObjectsOrExecutors &... io_objects_or_executors)
    -> asio::associated_executor_t<std::decay_t<CompletionHandler>,  First>
{
    return asio::get_associated_executor(completion_handler, first);
}

template<typename CompletionHandler,
        typename First,
        typename ... IoObjectsOrExecutors>
auto pick_executor_impl(rank<0>,
                        CompletionHandler &completion_handler,
                        First &,
                        IoObjectsOrExecutors &... io_objects_or_executors)
{
    return pick_executor(rank<1>{},
                         completion_handler,
                         io_objects_or_executors...);
}


template<typename CompletionHandler,
        typename ... IoObjectsOrExecutors>
auto pick_executor(CompletionHandler &completion_handler,
                   IoObjectsOrExecutors &... io_objects_or_executors)
{
    if constexpr (asio::detail::has_executor_type<std::decay_t<CompletionHandler>>::value)
        return asio::get_associated_executor(completion_handler);
    else
        return pick_executor_impl(rank<1>{}, completion_handler, io_objects_or_executors...);
}




}

}

#endif //CORO_EXECUTOR_HPP
