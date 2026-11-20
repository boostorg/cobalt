//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/ring_buffer.hpp>
#include <boost/cobalt/promise.hpp>


#include "test.hpp"

namespace cobalt = boost::cobalt;

cobalt::promise<void> do_write(cobalt::ring_buffer<void> &chn, std::vector<int> & seq)
{
  seq.push_back(0);
  co_await chn.write(); seq.push_back(1);
  co_await chn.write(); seq.push_back(2);
  (co_await cobalt::as_result(chn.write())).value(); seq.push_back(3);
  co_await cobalt::as_tuple(chn.write()); seq.push_back(4);
  co_await chn.write(); seq.push_back(5);
  co_await boost::asio::post(cobalt::use_op);
  co_await chn.write(); seq.push_back(6);
  co_await chn.write(); seq.push_back(7);
}

cobalt::promise<void> do_read(cobalt::ring_buffer<void> &chn, std::vector<int> & seq)
{
  seq.push_back(10);
  co_await chn.read(); seq.push_back(11);
  co_await chn.read(); seq.push_back(12);
  (co_await cobalt::as_result(chn.read())).value(); seq.push_back(13);
  co_await cobalt::as_tuple(chn.read()); seq.push_back(14);
  co_await chn.read(); seq.push_back(15);
  co_await boost::asio::post(cobalt::use_op);
  co_await chn.read(); seq.push_back(16);
  co_await chn.read(); seq.push_back(17);
}

BOOST_AUTO_TEST_SUITE(ring_buffer);

CO_TEST_CASE(void_)
{
  cobalt::ring_buffer<void> chn{2u, co_await cobalt::this_coro::executor};

  std::vector<int> seq;
  auto r = do_read(chn, seq);
  auto w = do_write(chn, seq);

  co_await r;
  co_await w;
  BOOST_REQUIRE(seq.size() == 16);
  BOOST_CHECK_EQUAL(seq[0], 10);
  BOOST_CHECK_EQUAL(seq[1], 0);
  BOOST_CHECK_EQUAL(seq[2], 11);
  BOOST_CHECK_EQUAL(seq[3], 1);
  BOOST_CHECK_EQUAL(seq[4], 12);
  BOOST_CHECK_EQUAL(seq[5], 2);
  BOOST_CHECK_EQUAL(seq[6], 13);
  BOOST_CHECK_EQUAL(seq[7], 3);
  BOOST_CHECK_EQUAL(seq[8], 14);
  BOOST_CHECK_EQUAL(seq[9], 4);
  BOOST_CHECK_EQUAL(seq[10], 15);
  BOOST_CHECK_EQUAL(seq[11], 5);
  BOOST_CHECK_EQUAL(seq[12], 16);
  BOOST_CHECK_EQUAL(seq[13], 6);
  BOOST_CHECK_EQUAL(seq[14], 17);
  BOOST_CHECK_EQUAL(seq[15], 7);
}

CO_TEST_CASE(void_0)
{
  cobalt::ring_buffer<void> chn{0u, co_await cobalt::this_coro::executor};

  co_await chn.write();
  co_await chn.write();
  co_await chn.write();
  auto r = [](cobalt::ring_buffer<void> & rb) -> cobalt::promise<void> { co_await rb.read();}(chn);

  BOOST_CHECK(!r.ready());
  co_await chn.write();
  BOOST_CHECK(r.ready());
}

CO_TEST_CASE(int_0)
{
  cobalt::ring_buffer<int> chn{0u, co_await cobalt::this_coro::executor};
  co_await chn.write(0);
  co_await chn.write(1);
  co_await chn.write(2);
  auto r = [](cobalt::ring_buffer<int> & rb) -> cobalt::promise<int> { co_return co_await rb.read();}(chn);

  BOOST_CHECK(!r.ready());
  co_await chn.write(3);
  BOOST_CHECK(r.ready());
  BOOST_CHECK(3 == co_await r);
}


CO_TEST_CASE(int_2)
{
  cobalt::ring_buffer<int> chn{2u, co_await cobalt::this_coro::executor};
  co_await chn.write(0);
  co_await chn.write(1);
  co_await chn.write(2);
  auto r = [](cobalt::ring_buffer<int> & rb) -> cobalt::promise<int> { co_return co_await rb.read();}(chn);

  BOOST_CHECK(r.ready());
  co_await chn.write(3);
  BOOST_CHECK(r.ready());
  BOOST_CHECK_EQUAL(co_await r, 1);
  BOOST_CHECK_EQUAL(co_await chn.read(), 2);
  BOOST_CHECK_EQUAL(co_await chn.read(), 3);

}


}
