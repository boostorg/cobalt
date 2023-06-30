//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_READ_AT_HPP
#define BOOST_ASYNC_READ_AT_HPP

#include <boost/async/promise.hpp>
#include <boost/async/io/random_access_device.hpp>
#include <boost/async/io/buffers/mutable_buffer_span.hpp>
#include <boost/container/pmr/vector.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<transfer_result> read_at(random_access_device & source, std::uint64_t offset, buffers::mutable_buffer buffer);
BOOST_ASYNC_DECL promise<transfer_result> read_at(random_access_device & source, std::uint64_t offset, buffers::mutable_buffer_span buffer);
BOOST_ASYNC_DECL promise<transfer_result> read_at(random_access_device & source, std::uint64_t offset, buffers::mutable_buffer_subspan buffer);
template<buffers::mutable_buffer_sequence MutableBufferSequence>
    requires (!std::convertible_to<buffers::mutable_buffer_span, MutableBufferSequence>)
promise<transfer_result> read(random_access_device & source, std::uint64_t offset, MutableBufferSequence && buffer)
{
  buffers::mutable_buffer buf[32];
  container::pmr::monotonic_buffer_resource res{buf, sizeof(buf), this_thread::get_default_resource()};
  container::pmr::vector<buffers::mutable_buffer> buf_span{buffer.begin(), buffer.end(), &res};
  co_return co_await read_at(source, offset, buffers::mutable_buffer_span{buf_span});
}


}


#endif //BOOST_ASYNC_READ_AT_HPP
