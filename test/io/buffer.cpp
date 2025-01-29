//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/buffer.hpp>

#include <boost/asio/buffer_registration.hpp>
#include <boost/asio/io_context.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(io);
BOOST_AUTO_TEST_SUITE(buffer);

BOOST_AUTO_TEST_CASE(mutable_)
{
  using namespace boost;

  std::string s1 = "foo", s2 = "bar";

  std::array<asio::mutable_buffer, 2u> buf = {asio::buffer(s1), asio::buffer(s2)};

  cobalt::io::mutable_buffer_sequence mbs = buf;
  BOOST_CHECK_EQUAL(mbs.is_registered(), false);

  BOOST_CHECK_EQUAL(mbs.buffer_count(), 2u);
  BOOST_CHECK_EQUAL(cobalt::io::buffer_size(mbs), 6u);

  std::string res;
  res.resize(6u);

  BOOST_CHECK_EQUAL(cobalt::io::buffer_copy(cobalt::io::buffer(res), mbs), 6u);
  BOOST_CHECK_EQUAL(res, "foobar");


  res = "123456";
  BOOST_CHECK_EQUAL(cobalt::io::buffer_copy(mbs, cobalt::io::buffer(res)), 6u);
  BOOST_CHECK_EQUAL(s1, "123");
  BOOST_CHECK_EQUAL(s2, "456");


  asio::io_context ctx;
  auto reg = boost::asio::register_buffers(ctx, buf);

  mbs = reg[0];

  res = "blabla";
  BOOST_CHECK_EQUAL(cobalt::io::buffer_copy(cobalt::io::buffer(res), mbs), 3u);
  BOOST_CHECK_EQUAL(res, "123bla");
}


BOOST_AUTO_TEST_CASE(const_)
{
  using namespace boost;

  std::string s1 = "foo", s2 = "bar";

  std::array<asio::mutable_buffer, 2u> buf = {asio::buffer(s1), asio::buffer(s2)};
  std::array<asio::const_buffer,   2u> bu2 = {asio::buffer(s1), asio::buffer(s2)};

  cobalt::io::const_buffer_sequence cb1;
  BOOST_CHECK_EQUAL(cb1.is_registered(), false);
  BOOST_CHECK_EQUAL(cb1.buffer_count(), 1u);
  BOOST_CHECK_EQUAL(cobalt::io::buffer_size(cb1), 0u);

  cobalt::io::const_buffer_sequence cbs = bu2;
  BOOST_CHECK_EQUAL(cbs.is_registered(), false);
  BOOST_CHECK_EQUAL(cbs.buffer_count(), 2u);
  BOOST_CHECK_EQUAL(cobalt::io::buffer_size(cbs), 6u);

  std::string res;
  res.resize(6u);

  BOOST_CHECK_EQUAL(cobalt::io::buffer_copy(cobalt::io::buffer(res), cbs), 6u);
  BOOST_CHECK_EQUAL(res, "foobar");

  asio::io_context ctx;

  auto reg = boost::asio::register_buffers(ctx, buf);

  cbs = reg[0];

  res = "blabla";
  BOOST_CHECK_EQUAL(cobalt::io::buffer_copy(cobalt::io::buffer(res), cbs), 3u);
  BOOST_CHECK_EQUAL(res, "foobla");
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
