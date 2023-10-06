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
#include <boost/async/io/buffers/const_buffer_span.hpp>
#include <boost/async/io/buffers/range.hpp>
#include <boost/container/pmr/vector.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<transfer_result> write(stream & source, buffers::const_buffer buffer);
BOOST_ASYNC_DECL promise<transfer_result> write(stream & source, buffers::const_buffer_span buffer);
BOOST_ASYNC_DECL promise<transfer_result> write(stream & source, buffers::const_buffer_subspan buffer);

template<buffers::const_buffer_sequence ConstBufferSequence>
  requires (!std::convertible_to<buffers::const_buffer_span, ConstBufferSequence>)
promise<transfer_result> write(stream & source, ConstBufferSequence && buffer)
{
  buffers::const_buffer buf[asio::detail::max_iov_len];

  transfer_result tr{};

  for (auto itr = buffers::begin(buffer), end = buffers::end(buffer); itr != end;)
  {
    auto ie = (std::min)(end, std::next(itr, asio::detail::max_iov_len));
    auto oe = std::copy(itr, ie, buf);

    buffers::const_buffer_span cbs{buf, std::distance(buf, oe)};
    tr += co_await write(source, cbs);
    itr = ie;
  }
  co_return tr;
}

}

#endif //BOOST_ASYNC_IO_WRITE_HPP
