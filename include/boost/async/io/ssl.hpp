//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SSL_HPP
#define BOOST_ASYNC_IO_SSL_HPP

#include <boost/async/io/stream.hpp>
#include <boost/async/io/socket.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/basic_stream_socket.hpp>

namespace boost::async::detail
{

struct ssl_stream_base
{

  template<typename ... Args>
  ssl_stream_base(Args && ...args) : ssl_stream_(std::forward<Args>(args)...) {}

  asio::ssl::stream<asio::basic_stream_socket<io::protocol_type, executor>> ssl_stream_;
};

}

namespace boost::async::io
{

struct ssl_stream : private detail::ssl_stream_base, stream, socket
{
  using shutdown_result = system::result<void>;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> close() override;
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> cancel() override;
  [[nodiscard]] BOOST_ASYNC_DECL bool is_open() const override;

  BOOST_ASYNC_DECL ssl_stream(const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL ssl_stream(ssl_stream && steam);
  BOOST_ASYNC_DECL ssl_stream(stream_socket && socket);

  BOOST_ASYNC_DECL ssl_stream(asio::ssl::context & ctx, const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL ssl_stream(asio::ssl::context & ctx, stream_socket && socket);

 private:
  BOOST_ASYNC_DECL void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  BOOST_ASYNC_DECL void async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;

  struct handshake_op_;
  struct handshake_op_buf_;
  struct handshake_op_buf_seq_;
  struct shutdown_op_;
  void adopt_endpoint_(endpoint & ep) override;
 public:
  using handshake_type = boost::asio::ssl::stream_base::handshake_type;
  BOOST_ASYNC_DECL handshake_op_         async_handshake(handshake_type ht);
  BOOST_ASYNC_DECL handshake_op_buf_     async_handshake(handshake_type ht, buffers::const_buffer buf);
  BOOST_ASYNC_DECL handshake_op_buf_seq_ async_handshake(handshake_type ht, buffers::const_buffer_subspan buf);
  BOOST_ASYNC_DECL handshake_op_buf_seq_ async_handshake(handshake_type ht, buffers::const_buffer_span buf);
  BOOST_ASYNC_DECL shutdown_op_ async_shutdown();
 private:

  // duplicate onto another thread
  system::result<ssl_stream> duplicate() = delete;
};

}

#include <boost/async/io/detail/ssl.hpp>


#endif //BOOST_ASYNC_IO_SSL_HPP
