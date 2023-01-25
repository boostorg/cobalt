//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/channel.hpp>

#include "test.hpp"
#include "doctest.h"
#include "boost/async/promise.hpp"

namespace async = boost::async;

async::promise<void> do_write(async::channel<void> &chn, std::vector<int> & seq)
{
  seq.push_back(0);
  co_await chn.write(); seq.push_back(1);
  co_await chn.write(); seq.push_back(2);
  co_await chn.write(); seq.push_back(3);
  co_await chn.write(); seq.push_back(4);
  co_await chn.write(); seq.push_back(5);
  co_await chn.write(); seq.push_back(6);
  co_await chn.write(); seq.push_back(7);
}

async::promise<void> do_read(async::channel<void> &chn, std::vector<int> & seq)
{
  seq.push_back(10);
  co_await chn.read(); seq.push_back(11);
  co_await chn.read(); seq.push_back(12);
  co_await chn.read(); seq.push_back(13);
  co_await chn.read(); seq.push_back(14);
  co_await chn.read(); seq.push_back(15);
  co_await chn.read(); seq.push_back(16);
  co_await chn.read(); seq.push_back(17);
}

TEST_SUITE_BEGIN("channel");

CO_TEST_CASE("void")
{
  async::channel<void> chn{2u, co_await async::this_coro::executor};

  std::vector<int> seq;
  auto r = do_read(chn, seq);
  auto w = do_write(chn, seq);

  co_await r;
  co_await w;
  REQUIRE(seq.size() == 16);
  CHECK(seq[0] == 10);
  CHECK(seq[1] == 0);
  CHECK(seq[2] == 1);
  CHECK(seq[3] == 2);
  CHECK(seq[4] == 11);
  CHECK(seq[5] == 12);
  CHECK(seq[6] == 3);
  CHECK(seq[7] == 4);
  CHECK(seq[8] == 13);
  CHECK(seq[9] == 14);
  CHECK(seq[10] == 5);
  CHECK(seq[11] == 6);
  CHECK(seq[12] == 15);
  CHECK(seq[13] == 16);
  CHECK(seq[14] == 7);
  CHECK(seq[15] == 17);
}

async::promise<void> do_write(async::channel<int> &chn, std::vector<int> & seq)
{
  seq.push_back(0);
  co_await chn.write(1); seq.push_back(1);
  co_await chn.write(2); seq.push_back(2);
  co_await chn.write(3); seq.push_back(3);
  co_await chn.write(4); seq.push_back(4);
  co_await chn.write(5); seq.push_back(5);
  co_await chn.write(6); seq.push_back(6);
  co_await chn.write(7); seq.push_back(7);
}

async::promise<void> do_read(async::channel<int> &chn, std::vector<int> & seq)
{
  seq.push_back(10);
  CHECK(1 == co_await chn.read()); seq.push_back(11);
  CHECK(2 == co_await chn.read()); seq.push_back(12);
  CHECK(3 == co_await chn.read()); seq.push_back(13);
  CHECK(4 == co_await chn.read()); seq.push_back(14);
  CHECK(5 == co_await chn.read()); seq.push_back(15);
  CHECK(6 == co_await chn.read()); seq.push_back(16);
  CHECK(7 == co_await chn.read()); seq.push_back(17);
}


CO_TEST_CASE("int")
{
  async::channel<int> chn{2u, co_await async::this_coro::executor};



  std::vector<int> seq;
  auto r = do_read(chn, seq);
  auto w = do_write(chn, seq);

  co_await r;
  co_await w;
  REQUIRE(seq.size() == 16);
  CHECK(seq[0] == 10);
  CHECK(seq[1] == 0);
  CHECK(seq[2] == 1);
  CHECK(seq[3] == 2);
  CHECK(seq[4] == 11);
  CHECK(seq[5] == 12);
  CHECK(seq[6] == 3);
  CHECK(seq[7] == 4);
  CHECK(seq[8] == 13);
  CHECK(seq[9] == 14);
  CHECK(seq[10] == 5);
  CHECK(seq[11] == 6);
  CHECK(seq[12] == 15);
  CHECK(seq[13] == 16);
  CHECK(seq[14] == 7);
  CHECK(seq[15] == 17);
}


TEST_SUITE_END();