//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_WRITE_HPP
#define BOOST_ASYNC_IO_WRITE_HPP

#include <boost/async/promise.hpp>
#include <boost/async/io/stream.hpp>
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>
#include <boost/async/io/buffers/const_buffer_span.hpp>
#include <boost/container/pmr/vector.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<transfer_result> write(stream & source, buffers::const_buffer buffer);
BOOST_ASYNC_DECL promise<transfer_result> write(stream & source, buffers::const_buffer_span buffer);
BOOST_ASYNC_DECL promise<transfer_result> write(stream & source, buffers::const_buffer_subspan buffer);

template<buffers::const_buffer_sequence MutableBufferSequence>
  requires (!std::convertible_to<buffers::const_buffer_span, MutableBufferSequence>)
promise<transfer_result> write(stream & source, MutableBufferSequence && buffer)
{
  buffers::const_buffer buf[32];
  container::pmr::monotonic_buffer_resource res{buf, sizeof(buf), this_thread::get_default_resource()};
  container::pmr::vector<buffers::const_buffer> buf_span{buffer.begin(), buffer.end(), &res};
  co_return co_await write(source, buffers::const_buffer_span{buf_span});
}

}

#endif //BOOST_ASYNC_IO_WRITE_HPP
