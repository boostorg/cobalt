//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/stream_socket.hpp>

#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/buffers.hpp>
#include <boost/async/join.hpp>
#include <boost/asio.hpp>

CO_TEST_CASE("local_stream")
{
  using namespace boost::async;
  auto [r, w] = io::make_pair(io::local_stream).value();

  char data[7] = "nodata";

  auto [written, read] =
      co_await join(w.write_some(io::buffers::buffer("foobar", 6)),
                    r.read_some(io::buffers::buffer(data)));

  CHECK(written.transferred == 6u);
  CHECK(read.transferred == 6u);
  CHECK(data == std::string("foobar"));
}
