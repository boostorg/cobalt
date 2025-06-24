//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#ifndef BOOST_COBALT_SSL_HPP
#define BOOST_COBALT_SSL_HPP

#include <boost/cobalt/io/socket.hpp>
#include <boost/cobalt/io/stream.hpp>

#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/ssl/stream.hpp>


namespace boost::cobalt::io::ssl
{

enum class verify
{
  none = asio::ssl::verify_none,
  peer = asio::ssl::verify_peer,
  fail_if_no_peer_cert = asio::ssl::verify_fail_if_no_peer_cert,
  client_once = asio::ssl::verify_client_once
};

using context = asio::ssl::context;
using verify_mode = asio::ssl::verify_mode;

namespace detail
{

struct stream_impl
{
  asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>> stream_socket_;
};

}

struct BOOST_SYMBOL_VISIBLE stream final : private detail::stream_impl, socket, cobalt::io::stream, asio::ssl::stream_base
{
  BOOST_COBALT_SSL_DECL stream(context & ctx, const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_SSL_DECL stream(context & ctx, stream_socket && sock);
  BOOST_COBALT_SSL_DECL stream(context & ctx, native_handle_type h, protocol_type protocol = protocol_type(),
                                     const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_SSL_DECL stream(context & ctx, endpoint ep,
                                     const cobalt::executor & executor = this_thread::get_executor());

  write_op write_some(const_buffer_sequence buffer) override
  {
    return {buffer, this, initiate_write_some_};
  }
  read_op read_some(mutable_buffer_sequence buffer) override
  {
    return {buffer, this, initiate_read_some_};
  }

  [[nodiscard]] bool secure() const {return upgraded_;}

  template<typename VerifyCallback>
    requires requires (const VerifyCallback & cb, context & ctx) {{cb(true, ctx)} -> std::same_as<bool>;}
  system::result<void> set_verify_callback(VerifyCallback vc)
  {
    system::error_code ec;
    stream_socket_.set_verify_callback(std::move(vc), ec);
    return ec ? ec : system::result<void>();
  }

  BOOST_COBALT_SSL_DECL
  system::result<void> set_verify_depth(int depth);


  BOOST_COBALT_SSL_DECL
  system::result<void> set_verify_mode(verify depth);

 private:

  struct [[nodiscard]] handshake_op_ final : cobalt::op<system::error_code>
  {
    BOOST_COBALT_SSL_DECL
    void ready(handler<system::error_code> h) final;
    BOOST_COBALT_SSL_DECL
    void initiate(completion_handler<system::error_code> h) final;
    handshake_op_(handshake_type type, bool upgraded, asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>>  & stream_socket)
        : type_(type), upgraded_(upgraded), stream_socket_(stream_socket) {}
   private:
    handshake_type type_;
    bool upgraded_;
    asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>> &stream_socket_;
  };


  struct [[nodiscard]] handshake_buffer_op_ final : cobalt::op<system::error_code, std::size_t>
  {
    BOOST_COBALT_SSL_DECL
    void ready(handler<system::error_code, std::size_t> h) final;
    BOOST_COBALT_SSL_DECL
    void initiate(completion_handler<system::error_code, std::size_t> h) final;
    handshake_buffer_op_(handshake_type type, bool upgraded, const_buffer_sequence buffer_,
                         asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>>  & stream_socket)
        : type_(type), upgraded_(upgraded), buffer_(buffer_), stream_socket_(stream_socket) {}
   private:
    handshake_type type_;
    bool upgraded_;
    const_buffer_sequence buffer_;
    asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>> &stream_socket_;
  };


  struct [[nodiscard]] shutdown_op_ final : cobalt::op<system::error_code>
  {
    BOOST_COBALT_SSL_DECL
    void ready(handler<system::error_code> h) final;
    BOOST_COBALT_SSL_DECL
    void initiate(completion_handler<system::error_code> h) final;
    shutdown_op_(bool upgraded, asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>>  & stream_socket)
        : upgraded_(upgraded), stream_socket_(stream_socket) {}
   private:
    bool upgraded_;
    asio::ssl::stream<asio::basic_stream_socket<protocol_type, executor>> &stream_socket_;
  };
 public:
  [[nodiscard]] auto handshake(handshake_type type) { return handshake_op_{type, upgraded_, stream_socket_}; }
  [[nodiscard]] auto handshake(handshake_type type, const_buffer_sequence buffer)
  {
    return handshake_buffer_op_{type, upgraded_, buffer, stream_socket_};
  }
  [[nodiscard]] auto shutdown() { return shutdown_op_{upgraded_, stream_socket_}; }


 private:

  BOOST_COBALT_SSL_DECL void adopt_endpoint_(endpoint & ep) override;

  BOOST_COBALT_SSL_DECL static void initiate_read_some_ (void *, mutable_buffer_sequence, cobalt::completion_handler<system::error_code, std::size_t>);
  BOOST_COBALT_SSL_DECL static void initiate_write_some_(void *,   const_buffer_sequence, cobalt::completion_handler<system::error_code, std::size_t>);


  bool upgraded_ = false;
};

}

#endif //BOOST_COBALT_SSL_HPP
