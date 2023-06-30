//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/read_at.hpp>
#include <boost/async/io/buffers/algorithm.hpp>

namespace boost::async::io
{

promise<transfer_result> read_at(random_access_device & source, std::uint64_t offset,buffers::mutable_buffer buffer)
{
  transfer_result tr;

  do
  {
    auto rd = co_await source.read_some_at(offset, buffer);
    tr.transferred += rd.transferred;
    offset += rd.transferred;
    tr.error = rd.error;
    buffer += rd.transferred;
  }
  while (buffer.size() > 0 && !tr.has_error());
  co_return tr;
}

promise<transfer_result> read_at(random_access_device & source, std::uint64_t offset, buffers::mutable_buffer_subspan buffer)
{
  transfer_result tr;
  do
  {
    auto rd = co_await source.read_some_at(offset, buffer);
    tr.transferred += rd.transferred;
    offset += rd.transferred;
    tr.error = rd.error;
    buffer = buffers::sans_prefix(buffer, rd.transferred);
  }
  while (buffers::buffer_size(buffer) > 0 && !tr.has_error());
  co_return tr;
}

promise<transfer_result> read_at(random_access_device & source, std::uint64_t offset, buffers::mutable_buffer_span buffer)
{
  return read_at(source, offset, buffers::mutable_buffer_subspan(buffer));
}


}