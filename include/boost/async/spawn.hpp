//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_SPAWN_HPP
#define BOOST_ASYNC_SPAWN_HPP

#include <boost/async/detail/spawn.hpp>

namespace boost::async
{

template<typename T, typename CompletionToken>
auto spawn(asio::io_context & context,
           task<T> && t,
           CompletionToken&& token)
{
    return asio::async_initiate<CompletionToken, void(std::exception_ptr, T)>(
            detail::async_initiate{}, token, std::move(t), context.get_executor());
}

template<std::derived_from<executor> Executor, typename T, typename CompletionToken>
auto spawn(Executor executor, task<T> && t,
           CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(Executor))
{
    return asio::async_initiate<CompletionToken, void(std::exception_ptr, T)>(
            detail::async_initiate{}, token, std::move(t), executor);
}

template<typename CompletionToken>
auto spawn(asio::io_context & context,
           task<void> && t,
           CompletionToken&& token)
{
    return asio::async_initiate<CompletionToken, void(std::exception_ptr)>(
            detail::async_initiate{}, token, std::move(t), context.get_executor());
}

template<std::derived_from<executor> Executor, typename CompletionToken>
auto spawn(Executor executor, task<void> && t,
           CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(Executor))
{
    return asio::async_initiate<CompletionToken, void(std::exception_ptr)>(
            detail::async_initiate{}, token, std::move(t), executor);

}

}

#endif //BOOST_ASYNC_SPAWN_HPP
