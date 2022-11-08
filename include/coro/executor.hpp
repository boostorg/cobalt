// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_EXECUTOR_HPP
#define CORO_EXECUTOR_HPP

#include <asio/associated_executor.hpp>
#include <asio/executor.hpp>
#include <asio/io_context.hpp>
#include <optional>

namespace coro::this_thread
{

namespace detail
{

inline static std::optional<asio::io_context::executor_type> executor;

}

inline typename asio::io_context::executor_type & get_executor()
{
  if (!detail::executor)
    throw asio::bad_executor();
  return *detail::executor;
}

inline void set_executor(asio::io_context::executor_type exec) noexcept
{
  detail::executor = std::move(exec);
}


}

#endif //CORO_EXECUTOR_HPP
