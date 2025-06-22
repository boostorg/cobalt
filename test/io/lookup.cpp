//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/resolver.hpp>

#include <boost/asio/buffer_registration.hpp>
#include <boost/asio/io_context.hpp>

#include "../test.hpp"

BOOST_AUTO_TEST_SUITE(io);
BOOST_AUTO_TEST_SUITE(lookup);

CO_TEST_CASE(boost_org)
{
  auto ep = co_await boost::cobalt::io::lookup("boost.org", "https");
  BOOST_REQUIRE(ep.size() >= 1u);
  BOOST_CHECK(boost::cobalt::io::get<boost::cobalt::io::tcp>(ep[0]).port() == 443);

}



BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
