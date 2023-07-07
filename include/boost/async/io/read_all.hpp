//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_READ_ALL_HPP
#define BOOST_ASYNC_IO_READ_ALL_HPP

#include <boost/async/promise.hpp>
#include <boost/async/io/stream.hpp>
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/container/pmr/vector.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<transfer_result> read_all(stream & source, buffers::any_dynamic_buffer & buffer,
                                                   std::size_t chunk_size = 4096);

template<buffers::dynamic_buffer DynamicBuffer>
promise<transfer_result> read_all(stream & source, DynamicBuffer &&buffer, std::size_t chunk_size = 4096)
{
  auto any = buffers::make_any(std::forward<DynamicBuffer>(buffer));
  buffers::any_dynamic_buffer & ab = any;
  co_return co_await read_all(source, ab, chunk_size);
}


}

#endif //BOOST_ASYNC_IO_READ_ALL_HPP
