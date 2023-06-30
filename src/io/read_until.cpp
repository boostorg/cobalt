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

promise<transfer_result> read_until(stream & source, buffers::any_dynamic_buffer & buffer,
                                    char delim, std::size_t chunk_size)
{
  transfer_result tr{};

  bool matched = false;
  do
  {
    auto buf = buffer.prepare(chunk_size);
    auto rd = co_await source.read_some(buf);

    auto begin = asio::buffers_begin(buf);
    auto end = std::next(asio::buffers_begin(buf), rd.transferred);
    matched  = std::find(begin, end, delim) != end;
    tr.transferred += rd.transferred;
    tr.error = rd.error;
    buffer.commit(rd.transferred);
  }
  while (buffer.max_size() > buffer.size() && !tr.has_error() && !matched);
  co_return tr;
}

promise<transfer_result> read_until(stream & source, buffers::any_dynamic_buffer & buffer,
                                    core::string_view delim, std::size_t chunk_size)
{
  transfer_result tr;
  if (delim.empty())
    co_return tr;

  std::size_t offset = 0;

  bool matched = false;
  do
  {
    auto buf = buffer.prepare(chunk_size);
    auto rd = co_await source.read_some(buf);
    tr.transferred += rd.transferred;
    tr.error = rd.error;
    buffer.commit(rd.transferred);

    auto relevant_memory = (std::min)(delim.size() + rd.transferred, buffers::buffer_size(buf));
    auto begin = std::prev(asio::buffers_end(buffer.data()), relevant_memory);
    auto end = asio::buffers_end(buffer.data());
    auto itr = std::search(begin, end, delim.begin(), delim.end());
    matched = (itr != end);
  }
  while (buffer.size() > 0 && !tr.has_error() && !matched);
  co_return tr;
}



}