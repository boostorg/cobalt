//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/ssl.hpp>

#include <boost/cobalt/experimental/io/initiate_templates.hpp>
#include <boost/cobalt/experimental/io/socket.hpp>
#include <boost/cobalt/experimental/io/stream_socket.hpp>


namespace boost::cobalt::experimental::io
{

static asio::ssl::context & get_ssl_context()
{
  thread_local static asio::ssl::context ctx{asio::ssl::context_base::tlsv13};
  return ctx;
}


ssl_stream::ssl_stream(const cobalt::executor & exec)
    : ssl_stream_base(exec, get_ssl_context()), socket(ssl_stream_.next_layer())
{
}

ssl_stream::ssl_stream(ssl_stream && lhs)
    : ssl_stream_base(std::move(lhs.ssl_stream_)), socket(ssl_stream_.next_layer())
{
}

ssl_stream::ssl_stream(stream_socket && socket_)
    : ssl_stream_base(std::move(socket_.stream_socket_), get_ssl_context()), socket(ssl_stream_.next_layer()) {}

ssl_stream::ssl_stream(asio::ssl::context & ctx, const cobalt::executor & exec)
    : ssl_stream_base(exec, ctx), socket(ssl_stream_.next_layer()) {}

ssl_stream::ssl_stream(asio::ssl::context & ctx, stream_socket && socket_)
    : ssl_stream_base(std::move(socket_.stream_socket_), ctx), socket(ssl_stream_.next_layer()) {}



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

void ssl_stream::initiate_read_some_(void *this_ , mutable_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  initiate_async_read_some(static_cast<ssl_stream*>(this_)->ssl_stream_, buffer, std::move(handler));

}
void ssl_stream::initiate_write_some_   (void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  initiate_async_write_some(static_cast<ssl_stream*>(this_)->ssl_stream_, buffer, std::move(handler));

}
void ssl_stream::initiate_shutdown_(void * this_, boost::cobalt::completion_handler<boost::system::error_code> handler)
{
  static_cast<ssl_stream*>(this_)->ssl_stream_.async_shutdown(std::move(handler));
}
void ssl_stream::initiate_handshake_(void *this_, handshake_type ht, boost::cobalt::completion_handler<boost::system::error_code> handler)
{
  static_cast<ssl_stream*>(this_)->ssl_stream_.async_handshake(ht, std::move(handler));
}

void ssl_stream::initiate_buffered_handshake_(void * this_, handshake_type ht, const_buffer_sequence seq,
                                              boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  auto & str = static_cast<ssl_stream*>(this_)->ssl_stream_;
  if (seq.buffer_count() > 0u)
    str.async_handshake(ht, seq, std::move(handler));
  else
    str.async_handshake(ht, seq.head, std::move(handler));
}



}
