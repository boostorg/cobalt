//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/stream_socket.hpp>

namespace boost::cobalt::io
{

stream_socket::stream_socket(const cobalt::executor & exec)
    : socket(stream_socket_), stream_socket_(exec)
{
}

stream_socket::stream_socket(native_handle_type h, protocol_type protocol, const cobalt::executor & exec)
    : socket(stream_socket_), stream_socket_(exec, protocol, h)
{
}

stream_socket::stream_socket(stream_socket && lhs)
    : socket(stream_socket_), stream_socket_(std::move(lhs.stream_socket_))
{
}
stream_socket::stream_socket(endpoint ep, const cobalt::executor & exec)
    : socket(stream_socket_), stream_socket_(exec, ep)
{
}



void stream_socket::adopt_endpoint_(endpoint & ep)
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

void stream_socket::initiate_read_some_(void *this_ , mutable_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf)
  {
    static_cast<stream_socket*>(this_)->stream_socket_.async_read_some(buf, std::move(handler))  ;
  });

}
void stream_socket::initiate_write_some_   (void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf)
  {
    static_cast<stream_socket*>(this_)->stream_socket_.async_write_some(buf, std::move(handler))  ;
  });
}


}