//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/pipe.hpp>
#include <boost/async/io/read_until.hpp>
#include <boost/async/io/write.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;

CO_TEST_CASE("pipe-test")
{
  std::string res;

  auto [r, w] = async::io::connect_pipe();

  auto ro = async::io::read_until(r, res, '\n');

  auto sz = co_await async::io::write(w, async::io::buffer("test-data\n", 10));
  CHECK(sz == 10);

  sz = co_await ro;
  CHECK(sz == 10);
  CHECK(res.size () == 10);
  CHECK(res == "test-data\n");
}