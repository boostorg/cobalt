//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/seq_packet_socket.hpp>

#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/buffers.hpp>
#include <boost/async/join.hpp>
#include <boost/asio.hpp>

CO_TEST_CASE("local_seq_packet")
{
  using namespace boost::async;
  auto [r, w] = io::make_pair(io::local_seqpacket).value();

  char data[7] = "nodata";

  int flags = -1;
  auto [written, read] =
      co_await join(w.send(io::buffers::buffer("foobar", 6), 0),
                    r.receive(io::buffers::buffer(data), flags));

  CHECK(flags == 0);
  CHECK(written.transferred == 6u);
  CHECK(read.transferred == 6u);
  CHECK(data == std::string("foobar"));
}
