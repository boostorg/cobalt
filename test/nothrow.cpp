//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/nothrow.hpp>

#include <boost/cobalt/task.hpp>

#include <boost/test/unit_test.hpp>
#include "test.hpp"

boost::cobalt::nothrow<boost::cobalt::task<void>> p1() {co_return ;}
boost::cobalt::nothrow<boost::cobalt::task<void>> p2()
{
  volatile char data[1000000000000ull];
  co_return ;
}

BOOST_AUTO_TEST_CASE(nothrow_)
{
  BOOST_CHECK(p1().has_value());
  BOOST_CHECK(p2().has_error());
}

