//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/seq_packet_socket.hpp>

namespace boost::cobalt::io
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
        ep.set_protocol(IPPROTO_SCTP);
      BOOST_FALLTHROUGH;
#endif
    case AF_UNIX:
      if (ep.protocol().type() == 0)
        ep.set_type(BOOST_ASIO_OS_DEF(SOCK_SEQPACKET));

  }
}

void seq_packet_socket::receive_op::initiate(boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  if (out_flags != nullptr)
    visit(buffer,
          [&](auto buf)
          {
            socket_.async_receive(buf, in_flags, *out_flags, std::move(handler));
          });
  else
    visit(buffer,
          [&](auto buf)
          {
            socket_.async_receive(buf, in_flags, std::move(handler));
          });
}

void seq_packet_socket::send_op::initiate(boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buf)
        {
          socket_.async_send(buf, in_flags, std::move(handler));
        });
}


}
