//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_READ_UNTIL_HPP
#define BOOST_ASYNC_IO_READ_UNTIL_HPP

#include <boost/async/promise.hpp>
#include <boost/async/io/stream.hpp>
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/container/pmr/vector.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<transfer_result> read_until(stream & source, buffers::any_dynamic_buffer & buffer,
                                                     char delim, std::size_t chunk_size = 4096);
BOOST_ASYNC_DECL promise<transfer_result> read_until(stream & source, buffers::any_dynamic_buffer & buffer,
                                                     core::string_view delim, std::size_t chunk_size = 4096);


template<buffers::dynamic_buffer DynamicBuffer>
promise<transfer_result> read_until(stream & source, DynamicBuffer &&buffer,
                                    char delim, std::size_t chunk_size = 4096)
{
  auto any = buffers::make_any(std::forward<DynamicBuffer>(buffer));
  buffers::any_dynamic_buffer & ab = any;
  co_return co_await read_until(source, ab, delim, chunk_size);
}

template<buffers::dynamic_buffer DynamicBuffer>
promise<transfer_result> read_until(stream & source, DynamicBuffer &&buffer,
                                    core::string_view delim, std::size_t chunk_size = 4096)
{
  auto any = buffers::make_any(std::forward<DynamicBuffer>(buffer));
  buffers::any_dynamic_buffer & ab = any;
  co_return co_await read_until(source, ab, delim, chunk_size);
}

}

#endif //BOOST_ASYNC_IO_READ_UNTIL_HPP
