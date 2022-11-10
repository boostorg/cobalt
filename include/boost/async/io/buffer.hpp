//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOTS_ASYNC_IO_BUFFER_HPP
#define BOOTS_ASYNC_IO_BUFFER_HPP

#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/flat_static_buffer.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/core/static_buffer.hpp>

#include <boost/container/pmr/polymorphic_allocator.hpp>

#include <boost/asio/buffer.hpp>
#include <boost/asio/detail/consuming_buffers.hpp>

namespace boost::async::io
{


using prepared_buffers =
    asio::detail::prepared_buffers<
        asio::const_buffer,
        asio::detail::buffer_sequence_adapter_base::max_buffers> ;

using asio::buffer;
using asio::mutable_buffer;
using asio::const_buffer;
using beast::flat_static_buffer;
using beast::static_buffer;

using beast::flat_static_buffer_base;
using beast::static_buffer_base;

using flat_buffer  = beast::basic_flat_buffer <container::pmr::polymorphic_allocator<char>>;
using multi_buffer = beast::basic_multi_buffer<container::pmr::polymorphic_allocator<char>>;

using streambuf = boost::asio::basic_streambuf<container::pmr::polymorphic_allocator<char>>;

}

#endif //BOOTS_ASYNC_IO_BUFFER_HPP
