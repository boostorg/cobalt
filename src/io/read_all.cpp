//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/read_until.hpp>
#include <boost/async/io/buffers/algorithm.hpp>
#include <algorithm>
#include <ranges>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/range/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace boost::async::io
{

promise<transfer_result> read_all(stream & source, buffers::dynamic_buffer_view buffer, std::size_t chunk_size)
{
  transfer_result tr;

  do
  {
    auto rd = co_await source.read_some(buffer.prepare((std::min)(chunk_size, buffer.max_size() - buffer.size())));
    tr.transferred += rd.transferred;
    tr.error = rd.error;
    buffer.commit(rd.transferred);
  }
  while (buffer.max_size() > buffer.size() && !tr.has_error());
  co_return tr;
}




}