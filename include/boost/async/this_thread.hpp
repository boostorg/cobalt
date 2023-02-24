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
#include <boost/asio/io_context.hpp>

namespace boost::async::this_thread
{
container::pmr::memory_resource* get_default_resource() noexcept;
container::pmr::memory_resource* set_default_resource(container::pmr::memory_resource* r) noexcept;
container::pmr::polymorphic_allocator<void> get_allocator();

typename asio::io_context::executor_type & get_executor(
    const boost::source_location & loc = BOOST_CURRENT_LOCATION);
bool has_executor();
void set_executor(asio::io_context::executor_type exec) noexcept;

}

#endif //BOOST_ASYNC_THIS_THREAD_HPP
