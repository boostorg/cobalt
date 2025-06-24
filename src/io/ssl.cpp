//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/ssl.hpp>
#include <boost/cobalt/io/stream_socket.hpp>

namespace boost::cobalt::io::ssl
{

stream::stream(context & ctx, const cobalt::executor & exec)
    : stream_impl{.stream_socket_={exec, ctx}}, socket(stream_socket_.lowest_layer())
{
}

stream::stream(context & ctx, native_handle_type h,
                       protocol_type protocol, const cobalt::executor & exec)
    : stream_impl{.stream_socket_ = {asio::basic_stream_socket<protocol_type, executor>(exec, protocol, h), ctx}},
      socket(stream_socket_.lowest_layer())

{
}

stream::stream(context & ctx, endpoint ep, const cobalt::executor & exec)
    : stream_impl{.stream_socket_={asio::basic_stream_socket<protocol_type, executor>(exec, ep), ctx}},
      socket(stream_socket_.lowest_layer())
{}

stream::stream(context & ctx, stream_socket && sock)
    : stream_impl{.stream_socket_={std::move(sock.stream_socket_), ctx}},
      socket(stream_socket_.lowest_layer())
{}


void stream::adopt_endpoint_(endpoint & ep)
{
  switch (ep.protocol().family())
  {
    case BOOST_ASIO_OS_DEF(AF_INET): BOOST_FALLTHROUGH;
    case BOOST_ASIO_OS_DEF(AF_INET6):
      if (ep.protocol().protocol() == BOOST_ASIO_OS_DEF(IPPROTO_IP))
        ep.set_protocol(BOOST_ASIO_OS_DEF(IPPROTO_TCP));
          BOOST_FALLTHROUGH;
    case AF_UNIX:
      if (ep.protocol().type() == 0)
        ep.set_type(BOOST_ASIO_OS_DEF(SOCK_STREAM));

  }
}


void stream::initiate_read_some_(void *this_ , mutable_buffer_sequence buffer,
                                     boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf)
  {
    auto t = static_cast<stream*>(this_);
    if (t->upgraded_)
      t->stream_socket_.async_read_some(buf, std::move(handler))  ;
    else
      t->stream_socket_.next_layer().async_read_some(buf, std::move(handler));
  });

}
void stream::initiate_write_some_(void * this_, const_buffer_sequence buffer,
                                      boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf)
  {
    auto t = static_cast<stream*>(this_);
    if (t->upgraded_)
      t->stream_socket_.async_write_some(buf, std::move(handler))  ;
    else
      t->stream_socket_.next_layer().async_write_some(buf, std::move(handler));
  });
}

void stream::handshake_op_::ready(handler<system::error_code> handler)
{
  if (upgraded_)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    handler(system::error_code{asio::error::already_started});
  }
}

void stream::handshake_op_::initiate(completion_handler<system::error_code> handler)
{
  stream_socket_.async_handshake(type_, std::move(handler));
}

void stream::handshake_buffer_op_::ready(handler<system::error_code, std::size_t> handler)
{
  if (upgraded_)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    handler(system::error_code{asio::error::already_started}, 0ull);
  }
}

void stream::handshake_buffer_op_::initiate(completion_handler<system::error_code, std::size_t> handler)
{
  stream_socket_.async_handshake(type_, buffer_, std::move(handler));
}

void stream::shutdown_op_::ready(handler<system::error_code> handler)
{
  if (!upgraded_)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    handler(system::error_code{asio::error::no_protocol_option});
  }
}

void stream::shutdown_op_::initiate(completion_handler<system::error_code> handler)
{
  stream_socket_.async_shutdown(std::move(handler));
}


system::result<void> stream::set_verify_depth(int depth)
{
  system::error_code ec;
  stream_socket_.set_verify_depth(depth, ec);
  return ec ? ec : system::result<void>();
}


system::result<void> stream::set_verify_mode(verify mode)
{
  system::error_code ec;
  stream_socket_.set_verify_mode(static_cast<asio::ssl::verify_mode>(mode), ec);
  return ec ? ec : system::result<void>();
}
}