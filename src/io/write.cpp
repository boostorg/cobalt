//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/write.hpp>
#include <boost/async/io/buffers/algorithm.hpp>

namespace boost::async::io
{

promise<transfer_result> write(stream & source, buffers::const_buffer buffer)
{
  transfer_result tr{};

  do
  {
    auto rd = co_await source.write_some(buffer);
    tr += rd;
    buffer += rd.transferred;
  }
  while (buffer.size() > 0 && !tr.has_error());
  co_return tr;
}

promise<transfer_result> write(stream & source, buffers::const_buffer_subspan buffer)
{
  transfer_result tr;
  do
  {
    auto rd = co_await source.write_some(buffer);
    tr.transferred += rd.transferred;
    tr.error = rd.error;
    buffer = buffers::sans_prefix(buffer, rd.transferred);
  }
  while (buffers::buffer_size(buffer) > 0 && !tr.has_error());
  co_return tr;
}

promise<transfer_result> write(stream & source, buffers::const_buffer_span buffer)
{
  return write(source, buffers::const_buffer_subspan(buffer));
}


}