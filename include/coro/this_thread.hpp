//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CORO_THIS_THREAD_HPP
#define CORO_THIS_THREAD_HPP

#include <memory_resource>
#include <asio/executor.hpp>
#include <asio/io_context.hpp>
#include <optional>

namespace coro::this_thread
{


namespace detail
{
inline static thread_local std::pmr::memory_resource * default_coro_memory_resource = std::pmr::get_default_resource();
}



inline std::pmr::memory_resource* get_default_resource() noexcept
{
  return detail::default_coro_memory_resource;
}

inline std::pmr::memory_resource* set_default_resource(std::pmr::memory_resource* r) noexcept
{
  auto pre = get_default_resource();
  detail::default_coro_memory_resource = r;
  return pre;
}

inline std::pmr::polymorphic_allocator<void> get_allocator()
{
  return std::pmr::polymorphic_allocator<void>(get_default_resource());
}

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

#endif //CORO_THIS_THREAD_HPP
