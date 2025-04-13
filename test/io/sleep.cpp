//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../test.hpp"

#include <boost/cobalt/io/sleep.hpp>

using namespace boost;

BOOST_AUTO_TEST_SUITE(sleep_);

CO_TEST_CASE(sleep_duration)
{
  auto pre = std::chrono::steady_clock::now();
  co_await cobalt::io::sleep(std::chrono::milliseconds(0));
  auto post = std::chrono::steady_clock::now();
  BOOST_CHECK((post - pre) < std::chrono::milliseconds(50));
  co_await cobalt::io::sleep(std::chrono::milliseconds(50));
  post = std::chrono::steady_clock::now();
  BOOST_CHECK((post - pre) >= std::chrono::milliseconds(50));
}


CO_TEST_CASE(system_time)
{
  auto pre = std::chrono::system_clock::now();
  co_await cobalt::io::sleep(std::chrono::milliseconds(0) + std::chrono::system_clock::now());
  auto post = std::chrono::system_clock::now();
  BOOST_CHECK((post - pre) < std::chrono::milliseconds(50));
  co_await cobalt::io::sleep(std::chrono::milliseconds(50) + std::chrono::system_clock::now());
  post = std::chrono::system_clock::now();
  BOOST_CHECK((post - pre) >= std::chrono::milliseconds(50));
}

CO_TEST_CASE(steady_time)
{
  auto pre = std::chrono::steady_clock::now();
  co_await cobalt::io::sleep(std::chrono::milliseconds(0) + std::chrono::steady_clock::now());
  auto post = std::chrono::steady_clock::now();
  BOOST_CHECK((post - pre) < std::chrono::milliseconds(50));
  co_await cobalt::io::sleep(std::chrono::milliseconds(50) + std::chrono::steady_clock::now());
  post = std::chrono::steady_clock::now();
  BOOST_CHECK((post - pre) >= std::chrono::milliseconds(50));
}


BOOST_AUTO_TEST_SUITE_END();
