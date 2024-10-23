//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/datagram_socket.hpp>
#include <boost/cobalt/experimental/io/initiate_templates.hpp>

namespace boost::cobalt::experimental::io
{

datagram_socket::datagram_socket(const cobalt::executor & exec)
    : socket(datagram_socket_), datagram_socket_(exec)
{
}

datagram_socket::datagram_socket(native_handle_type h, protocol_type protocol, const cobalt::executor & exec)
    : socket(datagram_socket_), datagram_socket_(exec, protocol, h)
{
}

datagram_socket::datagram_socket(datagram_socket && lhs)
    : socket(datagram_socket_), datagram_socket_(std::move(lhs.datagram_socket_))
{
}
datagram_socket::datagram_socket(endpoint ep, const cobalt::executor & exec)
    : socket(datagram_socket_), datagram_socket_(exec, ep)
{
}



void datagram_socket::adopt_endpoint_(endpoint & ep)
{

  switch (ep.protocol().family())
  {
    case BOOST_ASIO_OS_DEF(AF_INET): BOOST_FALLTHROUGH;
    case BOOST_ASIO_OS_DEF(AF_INET6):
      if (ep.protocol().protocol() == BOOST_ASIO_OS_DEF(IPPROTO_IP))
        ep.set_protocol(BOOST_ASIO_OS_DEF(IPPROTO_UDP));
      BOOST_FALLTHROUGH;
    case AF_UNIX:
      if (ep.protocol().type() == 0)
        ep.set_type(BOOST_ASIO_OS_DEF(SOCK_DGRAM));

  }
}

void datagram_socket::initiate_receive_(void *this_ , mutable_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
    initiate_async_receive(static_cast<datagram_socket*>(this_)->datagram_socket_, buffer, std::move(handler));

}
void datagram_socket::initiate_send_   (void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
    initiate_async_send(static_cast<datagram_socket*>(this_)->datagram_socket_, buffer, std::move(handler));

}


}