//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_THIS_THREAD_HPP
#define BOOST_ASYNC_THIS_THREAD_HPP

#include <boost/config.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/asio/io_context.hpp>

namespace boost::async::this_thread
{
BOOST_SYMBOL_EXPORT container::pmr::memory_resource* get_default_resource() noexcept;
BOOST_SYMBOL_EXPORT container::pmr::memory_resource* set_default_resource(container::pmr::memory_resource* r) noexcept;
BOOST_SYMBOL_EXPORT container::pmr::polymorphic_allocator<void> get_allocator();

BOOST_SYMBOL_EXPORT
typename asio::io_context::executor_type & get_executor(
    const boost::source_location & loc = BOOST_CURRENT_LOCATION);
BOOST_SYMBOL_EXPORT bool has_executor();
BOOST_SYMBOL_EXPORT void set_executor(asio::io_context::executor_type exec) noexcept;

}

#endif //BOOST_ASYNC_THIS_THREAD_HPP
