// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/cobalt/experimental/fiber.hpp>

#include "../test.hpp"

BOOST_AUTO_TEST_SUITE(fiber);

BOOST_AUTO_TEST_CASE(basics)
{
  boost::cobalt::experimental::detail::fiber_frame ff;

  using pro = boost::cobalt::experimental::detail::fiber_promise;
  auto hh = std::coroutine_handle<pro>::from_address(&ff);
  BOOST_CHECK(!hh.done());
  ff.resume_ = nullptr;
  BOOST_CHECK(hh.done());

  BOOST_CHECK(&ff.promise == &hh.promise());
  BOOST_CHECK(std::coroutine_handle<pro>::from_promise(ff.promise).address() == &ff);
}


BOOST_AUTO_TEST_SUITE_END();