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
auto spawn(promise<T> && t, CompletionToken&& token)
{
  return asio::async_initiate<CompletionToken, void(std::exception_ptr, T)>(
      detail::async_initiate{}, token, std::move(t));
}

template<typename CompletionToken>
auto spawn(promise<void> && t, CompletionToken&& token)
{
  return asio::async_initiate<CompletionToken, void(std::exception_ptr)>(
      detail::async_initiate{}, token, std::move(t));
}


template<typename ExecutionContext, typename T, typename CompletionToken>
requires (std::is_convertible<ExecutionContext&, asio::execution_context&>::value)
auto spawn(ExecutionContext & context,
promise<T> && t,
    CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(typename ExecutionContext::executor_type))
{
return spawn(std::move(t), asio::bind_executor(context.get_executor(), std::forward<CompletionToken>(token)));
}

template<typename Executor, typename T, typename CompletionToken>
requires (asio::is_executor<Executor>::value || asio::execution::is_executor<Executor>::value)
auto spawn(Executor executor,
           promise<T> && t,
           CompletionToken&& token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(Executor))
{
  return spawn(std::move(t), asio::bind_executor(executor, std::forward<CompletionToken>(token)));
}

}

#endif //BOOST_ASYNC_SPAWN_HPP
