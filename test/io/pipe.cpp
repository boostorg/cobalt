//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include "../test.hpp"

#include <boost/cobalt/io/pipe.hpp>
#include <boost/cobalt/io/read.hpp>
#include <boost/cobalt/io/write.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/promise.hpp>

using namespace boost;

BOOST_AUTO_TEST_SUITE(pipe_);

CO_TEST_CASE(rw_some)
{
  auto p = cobalt::io::pipe();
  BOOST_REQUIRE(p);
  auto &[r, w] = *p;

  std::string input;
  input.resize(14);

  std::array<char[7], 2u> output;
  std::memcpy(output[0], "Hello, ", 7);
  std::memcpy(output[1], " World!", 7);
  std::array<cobalt::io::const_buffer, 2u> outbuf = {cobalt::io::buffer(output[0]), cobalt::io::buffer(output[1])};
  BOOST_CHECK_EQUAL(cobalt::io::buffer_size(outbuf), 14u);
  cobalt::io::const_buffer_sequence cbs(outbuf); 
  BOOST_CHECK(!cbs.is_registered());
  auto itr = cbs.begin();
  BOOST_CHECK(itr == cbs.begin());
  BOOST_CHECK(itr->data() == &output[0]);
  BOOST_CHECK_EQUAL(itr->size(), 7u);
  itr++;
  BOOST_CHECK(itr->data() == &output[1]);
  BOOST_CHECK_EQUAL(itr->size(), 7u);
  itr++;
  BOOST_CHECK(itr == cbs.end());
  BOOST_CHECK_EQUAL(cbs.buffer_count(), 2u);

  cobalt::io::const_buffer_sequence seq{outbuf};
  while (asio::buffer_size(seq) > 0)
    seq += co_await w.write_some(seq);

  auto bf = cobalt::io::buffer(input);
  while (asio::buffer_size(bf) > 0)
    bf += co_await r.read_some(bf);

  BOOST_CHECK_EQUAL(input, "Hello,  World!");
  //BOOST_CHECK_EQUAL(rs, 14);
  //BOOST_CHECK_EQUAL(ws, 14);
}

BOOST_AUTO_TEST_CASE(rw_sync)
{
  asio::io_context ctx;
  cobalt::executor exec{ctx.get_executor()};
  auto p = cobalt::io::pipe(exec);
  BOOST_REQUIRE(p);
  auto & [r, w] = *p;

  std::string input;
  input.resize(14);

  std::array<char[7], 2u> output;
  std::memcpy(output[0], "Hello, ", 7);
  std::memcpy(output[1], " World!", 7);
  std::array<cobalt::io::const_buffer, 2u> outbuf = {cobalt::io::buffer(output[0]), cobalt::io::buffer(output[1])};
  BOOST_CHECK_EQUAL(cobalt::io::buffer_size(outbuf), 14u);
  BOOST_CHECK(!cobalt::io::const_buffer_sequence(outbuf).is_registered());
  BOOST_CHECK_EQUAL(cobalt::io::const_buffer_sequence(outbuf).begin()->data(), &output[0]);
  BOOST_CHECK_EQUAL(cobalt::io::const_buffer_sequence(outbuf).begin()->size(),7u);
  BOOST_CHECK_EQUAL(std::next(cobalt::io::const_buffer_sequence(outbuf).begin())->data(), &output[1]);
  BOOST_CHECK_EQUAL(std::next(cobalt::io::const_buffer_sequence(outbuf).begin())->size(),7u);
  BOOST_CHECK_EQUAL(cobalt::io::const_buffer_sequence(outbuf).buffer_count(), 2u);

  auto w_op = cobalt::io::write(w, outbuf);
  BOOST_CHECK(!w_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(w_op.step.buffer.buffer_count(), 2u);
  auto itr = w_op.step.buffer.begin();
  BOOST_CHECK_EQUAL(itr->data(), &output[0]);
  BOOST_CHECK_EQUAL(itr->size(),7u);
  itr++;
  BOOST_CHECK_EQUAL(itr->data(), &output[1]);
  BOOST_CHECK_EQUAL(itr->size(), 7u);
  itr++;
  auto end = w_op.step.buffer.end();
  BOOST_CHECK(itr == end);
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(w_op.step.buffer), 14u);


  auto bb = w_op.step.buffer;
  BOOST_CHECK(!w_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(bb.buffer_count(), 2u);
  BOOST_CHECK_EQUAL(bb.begin()->data(), &output[0]);
  BOOST_CHECK_EQUAL(bb.begin()->size(),7u);
  BOOST_CHECK_EQUAL(std::next(bb.begin())->data(), &output[1]);
  BOOST_CHECK_EQUAL(std::next(bb.begin())->size(),7u);
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(bb), 14u);


  auto r_op = cobalt::io::read(r, cobalt::io::buffer(input));

  BOOST_CHECK(!r_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(r_op.step.buffer.buffer_count(), 1u);
  BOOST_CHECK_EQUAL(r_op.step.buffer.begin()->data(), input.data());
  BOOST_CHECK_EQUAL(r_op.step.buffer.begin()->size(), input.size());
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(r_op.step.buffer), 14u);
}


CO_TEST_CASE(rw)
{
  auto p = cobalt::io::pipe();
  BOOST_REQUIRE(p);
  auto & [r, w] = *p;

  std::string input;
  input.resize(14);

  std::array<char[7], 2u> output;
  std::memcpy(output[0], "Hello, ", 7);
  std::memcpy(output[1], " World!", 7);
  std::array<cobalt::io::const_buffer, 2u> outbuf = {cobalt::io::buffer(output[0]), cobalt::io::buffer(output[1])};
  BOOST_CHECK_EQUAL(cobalt::io::buffer_size(outbuf), 14u);
  BOOST_CHECK(!cobalt::io::const_buffer_sequence(outbuf).is_registered());
  BOOST_CHECK_EQUAL(cobalt::io::const_buffer_sequence(outbuf).begin()->data(), &output[0]);
  BOOST_CHECK_EQUAL(cobalt::io::const_buffer_sequence(outbuf).begin()->size(),7u);
  BOOST_CHECK_EQUAL(std::next(cobalt::io::const_buffer_sequence(outbuf).begin())->data(), &output[1]);
  BOOST_CHECK_EQUAL(std::next(cobalt::io::const_buffer_sequence(outbuf).begin())->size(),7u);
  BOOST_CHECK_EQUAL(cobalt::io::const_buffer_sequence(outbuf).buffer_count(), 2u);

  auto w_op = cobalt::io::write(w, outbuf);
  BOOST_CHECK(!w_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(w_op.step.buffer.buffer_count(), 2u);
  auto itr = w_op.step.buffer.begin();
  BOOST_CHECK_EQUAL(itr->data(), &output[0]);
  BOOST_CHECK_EQUAL(itr->size(),7u);
  itr++;
  BOOST_CHECK_EQUAL(itr->data(), &output[1]);
  BOOST_CHECK_EQUAL(itr->size(), 7u);
  itr++;
  auto end = w_op.step.buffer.end();
  BOOST_CHECK(itr == end);
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(w_op.step.buffer), 14u);

  auto r_op = cobalt::io::read(r, cobalt::io::buffer(input));

  BOOST_CHECK(!r_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(r_op.step.buffer.buffer_count(), 1u);
  BOOST_CHECK_EQUAL(r_op.step.buffer.begin()->data(), input.data());
  BOOST_CHECK_EQUAL(r_op.step.buffer.begin()->size(), input.size());
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(r_op.step.buffer), 14u);

  BOOST_TEST_CHECKPOINT("Preparing await");
  const auto ws = co_await w_op;
  const auto rs = co_await r_op;
  BOOST_TEST_CHECKPOINT("Performed await");

  BOOST_CHECK_EQUAL(input, "Hello,  World!");
  BOOST_CHECK_EQUAL(rs, 14);
  BOOST_CHECK_EQUAL(ws, 14);
}

CO_TEST_CASE(rw_join)
{
  auto p = cobalt::io::pipe();
  BOOST_REQUIRE(p);
  auto & [r, w] = *p;

  std::string input;
  input.resize(14);

  std::array<char[7], 2u> output;
  std::memcpy(output[0], "Hello, ", 7);
  std::memcpy(output[1], " World!", 7);
  std::array<cobalt::io::const_buffer, 2u> outbuf = {cobalt::io::buffer(output[0]), cobalt::io::buffer(output[1])};


  auto w_op = cobalt::io::write(w, outbuf);
  BOOST_CHECK(!w_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(w_op.step.buffer.buffer_count(), 2u);
  auto itr = w_op.step.buffer.begin();
  BOOST_CHECK_EQUAL(itr->data(), &output[0]);
  BOOST_CHECK_EQUAL(itr->size(),7u);
  itr++;
  BOOST_CHECK_EQUAL(itr->data(), &output[1]);
  BOOST_CHECK_EQUAL(itr->size(), 7u);
  itr++;
  auto end = w_op.step.buffer.end();
  BOOST_CHECK(itr == end);
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(w_op.step.buffer), 14u);


  auto r_op = cobalt::io::read(r, cobalt::io::buffer(input));

  BOOST_CHECK(!r_op.step.buffer.is_registered());
  BOOST_CHECK_EQUAL(r_op.step.buffer.buffer_count(), 1u);
  BOOST_CHECK_EQUAL(r_op.step.buffer.begin()->data(), input.data());
  BOOST_CHECK_EQUAL(r_op.step.buffer.begin()->size(), input.size());
  BOOST_CHECK_EQUAL(boost::cobalt::io::buffer_size(r_op.step.buffer), 14u);

  auto res = co_await cobalt::join(w_op, r_op);

  auto & [rs, ws] = res;

  BOOST_CHECK_EQUAL(input, "Hello,  World!");
  BOOST_CHECK_EQUAL(rs, 14);
  BOOST_CHECK_EQUAL(ws, 14);
}


BOOST_AUTO_TEST_SUITE_END();
