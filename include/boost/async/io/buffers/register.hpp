//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_BUFFERS_REGISTER_HPP
#define BOOST_ASYNC_IO_BUFFERS_REGISTER_HPP

#include <boost/async/io/buffers/mutable_buffer_span.hpp>

#include <boost/asio/buffer_registration.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

namespace boost::async::io::buffers
{

using buffer_registration = asio::buffer_registration<buffers::mutable_buffer_subspan,
                                                      container::pmr::polymorphic_allocator<void>>;

BOOST_ASYNC_DECL buffer_registration register_(buffers::mutable_buffer mutable_buffer);
BOOST_ASYNC_DECL buffer_registration register_(buffers::mutable_buffer_span mutable_buffers);
BOOST_ASYNC_DECL buffer_registration register_(buffers::mutable_buffer_subspan mutable_buffers);

}

#endif //BOOST_ASYNC_IO_BUFFERS_REGISTER_HPP
