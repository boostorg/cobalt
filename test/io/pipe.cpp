//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/pipe.hpp>
#include <boost/async/io/buffers.hpp>
#include <boost/async/join.hpp>
#include <boost/asio.hpp>

CO_TEST_CASE("pipe")
{
  using namespace boost::async;
  auto [r, w] = io::make_pipe().value();

  {
    io::stream & rs = r;
    io::stream & ws = w;
    char buf[4096];
    auto ec = co_await ws. read_some(io::buffers::buffer(buf));
    CHECK(co_await ws. read_some(io::buffers::buffer(buf)) == boost::asio::error::operation_not_supported);
    CHECK(co_await rs.write_some(io::buffers::buffer(buf)) == boost::asio::error::operation_not_supported);
  }

  char data[7] = "nodata";

  auto [written, read] =
      co_await join(w.write_some("foobar"),
                    r.read_some(data));

  CHECK(written.transferred == 6u);
  CHECK(read.transferred == 6u);
  CHECK(data == std::string("foobar"));

}

