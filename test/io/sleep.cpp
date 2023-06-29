//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../test.hpp"

#include <boost/async/io/sleep.hpp>

TEST_SUITE_BEGIN("sleep");

CO_TEST_CASE_TEMPLATE("sleep-timepoint", Clock, std::chrono::steady_clock, std::chrono::system_clock)
{
  auto pre = Clock::now();
  (co_await boost::async::io::sleep(pre)).value();
  auto post = Clock::now();
  CHECK((post - pre) < std::chrono::milliseconds(50));
  (co_await boost::async::io::sleep(pre + std::chrono::milliseconds(50))).value();
  post = Clock::now();
  CHECK((post - pre) >= std::chrono::milliseconds(50));
}

CO_TEST_CASE("sleep-duration")
{
  auto pre = std::chrono::steady_clock::now();
  (co_await boost::async::io::sleep(std::chrono::milliseconds(0))).value();
  auto post = std::chrono::steady_clock::now();
  CHECK((post - pre) < std::chrono::milliseconds(50));
  (co_await boost::async::io::sleep(std::chrono::milliseconds(50))).value();
  post = std::chrono::steady_clock::now();
  CHECK((post - pre) >= std::chrono::milliseconds(50));
}


TEST_SUITE_END();