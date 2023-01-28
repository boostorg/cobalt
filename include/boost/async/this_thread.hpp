//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_THIS_THREAD_HPP
#define BOOST_ASYNC_THIS_THREAD_HPP

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <optional>

namespace boost::async::this_thread
{


namespace detail
{
extern thread_local container::pmr::memory_resource * default_coro_memory_resource;
extern thread_local std::optional<asio::io_context::executor_type> executor;
}



inline container::pmr::memory_resource* get_default_resource() noexcept
{
  return detail::default_coro_memory_resource;
}

inline container::pmr::memory_resource* set_default_resource(container::pmr::memory_resource* r) noexcept
{
  auto pre = get_default_resource();
  detail::default_coro_memory_resource = r;
  return pre;
}

inline container::pmr::polymorphic_allocator<void> get_allocator()
{
  return container::pmr::polymorphic_allocator<void>(get_default_resource());
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

inline void reset_executor() noexcept
{
  detail::executor.reset();
}


}

#endif //BOOST_ASYNC_THIS_THREAD_HPP
