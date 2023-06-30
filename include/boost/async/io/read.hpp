//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_READ_HPP
#define BOOST_ASYNC_IO_READ_HPP

#include <boost/async/promise.hpp>
#include <boost/async/io/stream.hpp>
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer_span.hpp>
#include <boost/container/pmr/vector.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<transfer_result> read(stream & source, buffers::mutable_buffer buffer);
BOOST_ASYNC_DECL promise<transfer_result> read(stream & source, buffers::mutable_buffer_span buffer);
BOOST_ASYNC_DECL promise<transfer_result> read(stream & source, buffers::mutable_buffer_subspan buffer);
BOOST_ASYNC_DECL promise<transfer_result> read(stream & source, buffers::any_dynamic_buffer & buffer,
                                               std::size_t chunk_size = 4096);

template<buffers::mutable_buffer_sequence MutableBufferSequence>
  requires (!std::convertible_to<buffers::mutable_buffer_span, MutableBufferSequence>)
promise<transfer_result> read(stream & source, MutableBufferSequence && buffer)
{
  buffers::mutable_buffer buf[32];
  container::pmr::monotonic_buffer_resource res{buf, sizeof(buf), this_thread::get_default_resource()};
  container::pmr::vector<buffers::mutable_buffer> buf_span{buffer.begin(), buffer.end(), &res};
  co_return co_await read(source, buffers::mutable_buffer_span{buf_span});
}

template<buffers::dynamic_buffer DynamicBuffer>
promise<transfer_result> read(stream & source, DynamicBuffer && buffer, std::size_t chunk_size = 4096)
{
  auto any = buffers::make_any(std::forward<DynamicBuffer>(buffer));
  buffers::any_dynamic_buffer & ab = any;
  co_return co_await read(source, ab, chunk_size);
}


}

#endif //BOOST_ASYNC_IO_READ_HPP
