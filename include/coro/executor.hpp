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
auto pick_executor(CompletionHandler &&completion_handler,
                   const First &first,
                   IoObjectsOrExecutors &&... io_objects_or_executors)
-> typename std::enable_if<
        asio::is_executor<First>::value || asio::execution::is_executor<First>::value, First>::type
{
    return first;
}


template<typename CompletionHandler,
        typename First,
        typename ... IoObjectsOrExecutors>
auto pick_executor(CompletionHandler &&completion_handler,
                   First &first,
                   IoObjectsOrExecutors &&... io_objects_or_executors)
-> typename First::executor_type
{
    return first.get_executor();
}


template<typename CompletionHandler,
        typename First,
        typename ... IoObjectsOrExecutors>
auto pick_executor(CompletionHandler &&completion_handler,
                   First &&,
                   IoObjectsOrExecutors &&... io_objects_or_executors)
{
    return pick_executor(std::forward<CompletionHandler>(completion_handler),
                         std::forward<IoObjectsOrExecutors>(io_objects_or_executors)...);
}

}

}

#endif //CORO_EXECUTOR_HPP
