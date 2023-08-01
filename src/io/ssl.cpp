//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/ssl.hpp>
#include <boost/async/io/detail/duplicate.hpp>
#include <boost/async/io/stream_socket.hpp>

namespace boost::async::io
{
/*
system::result<ssl_stream> ssl_stream::duplicate()
{
  SSL_dup(ssl_stream_.native_handle());
  auto res = detail::io::duplicate_handle(ssl_stream_.native_handle());
  if (!res)
    return res.error();

  return {system::in_place_value, ssl_stream(*res)};
}*/

static asio::ssl::context & get_ssl_context()
{
  thread_local static asio::ssl::context ctx{asio::ssl::context_base::tlsv13};
  return ctx;
}


ssl_stream::ssl_stream()
  : ssl_stream_base(this_thread::get_executor(), get_ssl_context()), socket(ssl_stream_.next_layer())
{
}

ssl_stream::ssl_stream(ssl_stream && lhs)
    : ssl_stream_base(std::move(lhs.ssl_stream_)), socket(ssl_stream_.next_layer())
{
}

ssl_stream::ssl_stream(stream_socket && socket_)
    : ssl_stream_base(std::move(socket_.stream_socket_), get_ssl_context()), socket(ssl_stream_.next_layer()) {}

ssl_stream::ssl_stream(asio::ssl::context & ctx)
    : ssl_stream_base(this_thread::get_executor(), ctx), socket(ssl_stream_.next_layer()) {}

ssl_stream::ssl_stream(asio::ssl::context & ctx, stream_socket && socket_)
    : ssl_stream_base(std::move(socket_.stream_socket_), ctx), socket(ssl_stream_.next_layer()) {}



void ssl_stream::async_read_some_impl_(
    buffers::mutable_buffer_subspan buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  ssl_stream_.async_read_some(buffer, std::move(h));
}

void ssl_stream::async_write_some_impl_(
    buffers::const_buffer_subspan buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  ssl_stream_.async_write_some(buffer, std::move(h));
}

system::result<void> ssl_stream::close() { return socket::close(); }
system::result<void> ssl_stream::cancel() { return socket::cancel(); }
bool ssl_stream::is_open() const {return socket::is_open();}

ssl_stream::handshake_op_         ssl_stream::async_handshake(handshake_type ht)
{
  return handshake_op_{*this, ht};
}
ssl_stream::handshake_op_buf_     ssl_stream::async_handshake(handshake_type ht, buffers::const_buffer buf)
{
  return handshake_op_buf_{*this, ht, buf};
}
ssl_stream::handshake_op_buf_seq_ ssl_stream::async_handshake(handshake_type ht, buffers::const_buffer_span buf)
{
  return handshake_op_buf_seq_{*this, ht, buffers::const_buffer_subspan{buf}};
}
ssl_stream::handshake_op_buf_seq_ ssl_stream::async_handshake(handshake_type ht, buffers::const_buffer_subspan buf)
{
  return handshake_op_buf_seq_{*this, ht, buf};
}

ssl_stream::shutdown_op_ ssl_stream::async_shutdown()
{
  return shutdown_op_{*this};
}

void ssl_stream::adopt_endpoint_(endpoint & ep)
{

  switch (ep.protocol().family())
  {
    case BOOST_ASIO_OS_DEF(AF_INET): BOOST_FALLTHROUGH;
    case BOOST_ASIO_OS_DEF(AF_INET6):
      if (ep.protocol().protocol() == BOOST_ASIO_OS_DEF(IPPROTO_IP))
        ep.set_protocol(BOOST_ASIO_OS_DEF(IPPROTO_TCP));
    case AF_UNIX:
      if (ep.protocol().type() == 0)
        ep.set_type(BOOST_ASIO_OS_DEF(SOCK_STREAM));
  }
}

void ssl_stream::handshake_op_::initiate(completion_handler<system::error_code> handler)
{
  stream.ssl_stream_.async_handshake(ht, std::move(handler));

}

void ssl_stream::handshake_op_buf_::initiate(completion_handler<system::error_code, std::size_t> handler)
{
  stream.ssl_stream_.async_handshake(ht, io::buffers::const_buffer_subspan{&buf, 1u}, std::move(handler));
}

void ssl_stream::handshake_op_buf_seq_::initiate(completion_handler<system::error_code, std::size_t> handler)
{
  stream.ssl_stream_.async_handshake(ht, buf, std::move(handler));
}

void ssl_stream::shutdown_op_::initiate(completion_handler<system::error_code> handler)
{
  stream.ssl_stream_.async_shutdown(std::move(handler));
}




}