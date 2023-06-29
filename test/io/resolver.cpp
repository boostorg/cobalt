//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/resolver.hpp>
#include <boost/asio.hpp>



CO_TEST_CASE("resolver")
{
  using namespace boost;

  auto t = co_await async::io::lookup("boost.org", "http").value();

  CHECK(t.error() == system::error_code{});
  REQUIRE(t.value().size() > 0u);
  for (auto & ep : *t)
    CHECK(ep.protocol() == async::io::ip_v4);


}