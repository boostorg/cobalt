//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/ssl.hpp>
#include "../doctest.h"
#include "../test.hpp"

#include <boost/async/io/resolver.hpp>
#include <boost/asio.hpp>

CO_TEST_CASE("ssl")
{
  using namespace boost;

  auto t = (co_await async::io::lookup("boost.org", "https")).value();
  REQUIRE(!t.empty());

  async::io::ssl_stream ss{};

  auto conn = co_await ss.connect(t.front());
  CHECK_MESSAGE(conn, conn.error().message());

  CHECK(co_await ss.async_handshake(async::io::ssl_stream::handshake_type::client)
        == system::in_place_value);

  CHECK(co_await ss.async_shutdown() == system::in_place_value);
}