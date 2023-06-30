//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_COPY_HPP
#define BOOST_ASYNC_IO_COPY_HPP

#include <boost/async/io/stream.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<std::pair<transfer_result, transfer_result>>
copy(stream & source, stream & sink);

BOOST_ASYNC_DECL promise<std::pair<transfer_result, transfer_result>>
copy(stream & source, stream & sink, buffers::any_dynamic_buffer & buffer, std::size_t chunk_size = 4096);


template<buffers::dynamic_buffer DynamicBuffer>
promise<transfer_result> read(stream & source, stream & sink, DynamicBuffer & buffer, std::size_t chunk_size = 4096)
{
  auto any = buffers::make_any(buffer);
  buffers::any_dynamic_buffer & ab = any;
  co_return co_await copy(source, sink,  ab, chunk_size);
}

}

#endif //BOOST_ASYNC_IO_COPY_HPP
