//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/copy.hpp>
#include <boost/async/join.hpp>
#include <boost/async/io/buffers/circular_buffer.hpp>
#include <boost/async/io/buffers/buffer_size.hpp>

namespace boost::async::io
{

promise<std::pair<transfer_result, transfer_result>>
copy(stream & source, stream & sink)
{
  constexpr std::size_t chunk_size = 4096;
  char mem[chunk_size * 2];
  buffers::circular_buffer buf{mem, sizeof(mem)};

  transfer_result r = co_await source.read_some(buf.prepare(chunk_size)),
                  w = {};

  buf.commit(r.transferred);

  while (!r.has_error() && !w.has_error())
  {
    auto [r2, w2] = co_await join(
        source.read_some(buf.prepare(chunk_size)),
        sink.write_some(buf.data()));
    buf.commit(r2.transferred);
    buf.consume(w2.transferred);
    r.transferred += r2.transferred;
    w.transferred += w2.transferred;
    r.error = r2.error;
    w.error = w2.error;
  }
  // remaining readable stuff
  while (r.has_error() && !w.has_error() && buffers::buffer_size(buf.data()) > 0u)
  {
    auto w2 = co_await sink.write_some(buffers::const_buffer_span(buf.data()));
    buf.consume(w2.transferred);
    w.transferred = w2.transferred;
    w.error       = w2.error;
  }
  co_return {r, w};
}

promise<std::pair<transfer_result, transfer_result>>
copy(stream & source, stream & sink, buffers::dynamic_buffer_view buf, std::size_t chunk_size)
{
  transfer_result r = co_await source.read_some(buffers::mutable_buffer_span(buf.prepare(chunk_size))),
      w = {};

  buf.commit(r.transferred);

  while (!r.has_error() && !w.has_error())
  {
    auto [r2, w2] = co_await join(
        source.read_some(buffers::mutable_buffer_span(buf.prepare(chunk_size))),
        sink.write_some(buffers::const_buffer_span(buf.data())));
    buf.commit(r2.transferred);
    buf.consume(w2.transferred);
    r.transferred += r2.transferred;
    w.transferred += w2.transferred;
    r.error = r2.error;
    w.error = w2.error;
  }
  // remaining readable stuff
  while (r.has_error() && !w.has_error() && buffers::buffer_size(buf.data()) > 0u)
  {
    auto w2 = co_await sink.write_some(buffers::const_buffer_span(buf.data()));
    buf.consume(w2.transferred);
    w.transferred = w2.transferred;
    w.error       = w2.error;
  }
  co_return {r, w};
}


}
