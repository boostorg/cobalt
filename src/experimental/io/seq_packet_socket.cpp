//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/seq_packet_socket.hpp>
#include <boost/cobalt/experimental/io/initiate_templates.hpp>

namespace boost::cobalt::experimental::io
{

seq_packet_socket::seq_packet_socket(const cobalt::executor & exec)
    : socket(seq_packet_socket_), seq_packet_socket_(exec)
{
}

seq_packet_socket::seq_packet_socket(native_handle_type h, protocol_type protocol, const cobalt::executor & exec)
    : socket(seq_packet_socket_), seq_packet_socket_(exec, protocol, h)
{
}

seq_packet_socket::seq_packet_socket(seq_packet_socket && lhs)
    : socket(seq_packet_socket_), seq_packet_socket_(std::move(lhs.seq_packet_socket_))
{
}
seq_packet_socket::seq_packet_socket(endpoint ep, const cobalt::executor & exec)
    : socket(seq_packet_socket_), seq_packet_socket_(exec, ep)
{
}



void seq_packet_socket::adopt_endpoint_(endpoint & ep)
{

  switch (ep.protocol().family())
  {
#if defined(IPPROTO_SCTP)
    case BOOST_ASIO_OS_DEF(AF_INET): BOOST_FALLTHROUGH;
    case BOOST_ASIO_OS_DEF(AF_INET6):
      if (ep.protocol().protocol() == BOOST_ASIO_OS_DEF(IPPROTO_IP))
        ep.set_protocol(BOOST_ASIO_OS_DEF(IPPROTO_UDP));
      BOOST_FALLTHROUGH;
#endif
    case AF_UNIX:
      if (ep.protocol().type() == 0)
        ep.set_type(BOOST_ASIO_OS_DEF(SOCK_DGRAM));

  }
}

void seq_packet_socket::initiate_receive_(void *this_ , message_flags in, message_flags  * out, mutable_buffer_sequence seq, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  auto & s = static_cast<seq_packet_socket*>(this_)->seq_packet_socket_;
  if (out != nullptr)
  {
    if (seq.buffer_count() > 0u)
      s.async_receive(seq, in, *out, std::move(handler));
    else if (seq.is_registered())
      s.async_receive(seq.registered, in, *out, std::move(handler));
    else
      s.async_receive(seq.head, in, *out, std::move(handler));
  }
  else
  {
    if (seq.buffer_count() > 0u)
      s.async_receive(seq, in, std::move(handler));
    else if (seq.is_registered())
      s.async_receive(seq.registered, in, std::move(handler));
    else
      s.async_receive(seq.head, in, std::move(handler));
  }
}

void seq_packet_socket::initiate_send_   (void * this_, message_flags in, const_buffer_sequence seq, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  auto & s = static_cast<seq_packet_socket*>(this_)->seq_packet_socket_;
  if (seq.buffer_count() > 0u)
    s.async_send(seq, in, std::move(handler));
  else if (seq.is_registered())
    s.async_send(seq.registered, in, std::move(handler));
  else
    s.async_send(seq.head, in,std::move(handler));
}


}