//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_COPY_N_HPP
#define BOOST_ASYNC_IO_COPY_N_HPP

#include <boost/async/io/stream.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<std::pair<transfer_result, transfer_result>>
copy_n(stream & source, stream & sink, std::size_t n);

BOOST_ASYNC_DECL promise<std::pair<transfer_result, transfer_result>>
copy_n(stream & source, stream & sink, buffers::any_dynamic_buffer & buffer,
       std::size_t n, std::size_t chunk_size = 4096);


template<buffers::dynamic_buffer DynamicBuffer>
promise<transfer_result> copy_n(stream & source, stream & sink, DynamicBuffer & buffer,
                                std::size_t n, std::size_t chunk_size = 4096)
{
  auto any = buffers::make_any(buffer);
  buffers::any_dynamic_buffer & ab = any;
  co_return co_await copy_n(source, sink,  ab, n, chunk_size);
}

}

#endif //BOOST_ASYNC_IO_COPY_N_HPP
