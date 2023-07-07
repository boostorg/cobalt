//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_SSL_HPP
#define BOOST_ASYNC_SSL_HPP

#include <boost/async/io/ssl.hpp>
#include <boost/async/io/buffers.hpp>

namespace boost::async::io
{
struct ssl_stream::handshake_op_ final : result_op<void>
{
  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code> handler) override;
  handshake_op_(ssl_stream & stream, handshake_type ht) : stream(stream), ht(ht) {}
 private:

  ssl_stream & stream;
  handshake_type ht;
};

struct ssl_stream::handshake_op_buf_ final : transfer_op
{
  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, std::size_t> handler) override;
  handshake_op_buf_(ssl_stream & stream,
                    handshake_type ht,
                    buffers::const_buffer buf) : stream(stream), ht(ht), buf(buf) {}
 private:
  ssl_stream & stream;
  handshake_type ht;
  buffers::const_buffer buf;
};

struct ssl_stream::handshake_op_buf_seq_ final : transfer_op
{
  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, std::size_t> handler) override;
  handshake_op_buf_seq_(ssl_stream & stream, handshake_type ht,
                     buffers::const_buffer_subspan buf) : stream(stream), ht(ht), buf(buf) {}
 private:
  ssl_stream & stream;
  handshake_type ht;
  buffers::const_buffer_subspan buf;
};

struct ssl_stream::shutdown_op_ final : result_op<void>
{
  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code> handler) override;
  shutdown_op_(ssl_stream & stream) : stream(stream) {}
 private:
  ssl_stream & stream;
};

}

#endif //BOOST_ASYNC_SSL_HPP
