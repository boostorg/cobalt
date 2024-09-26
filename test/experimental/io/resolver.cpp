//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../../test.hpp"
#include <boost/cobalt/experimental/io/resolver.hpp>
#include <boost/asio.hpp>

CO_TEST_CASE(resolver_)
{
  using namespace boost;

  auto t = co_await cobalt::experimental::io::lookup("boost.org", "http");

  BOOST_REQUIRE(t.size() > 0u);
  for (auto & ep : t)
    BOOST_CHECK(ep.protocol() == cobalt::experimental::io::ip_v4);


}
