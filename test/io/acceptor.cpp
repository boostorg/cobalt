//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/acceptor.hpp>
#include <boost/async/join.hpp>
#include <boost/async/io/buffers.hpp>

#include <boost/asio.hpp>

CO_TEST_CASE("acceptor")
{
  using namespace boost::async;
  io::endpoint ep{io::tcp, "127.0.0.1", 8080};
  io::acceptor acceptor{ep};
  io::stream_socket ss{};

  auto [accepted, connected] =
      co_await join(acceptor.accept(),
                    ss.connect(ep));

  CHECK(!accepted.has_error());
  CHECK(!connected.has_error());

  CHECK(accepted->remote_endpoint()->protocol() == ss.local_endpoint()->protocol());
  CHECK(accepted->local_endpoint()->protocol() == ss.remote_endpoint()->protocol());

  CHECK(get<io::tcp_v4>(*accepted->remote_endpoint()).port() == get<io::tcp_v4>(*ss.local_endpoint()) .port());
  CHECK(get<io::tcp_v4>(*accepted->local_endpoint()) .port() == get<io::tcp_v4>(*ss.remote_endpoint()).port());
  CHECK(get<io::tcp_v4>(*accepted->remote_endpoint()).addr() == get<io::tcp_v4>(*ss.local_endpoint()) .addr());
  CHECK(get<io::tcp_v4>(*accepted->local_endpoint()) .addr() == get<io::tcp_v4>(*ss.remote_endpoint()).addr());
}
