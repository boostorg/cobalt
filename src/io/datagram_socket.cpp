//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/datagram_socket.hpp>
#include <boost/async/io/detail/duplicate.hpp>

namespace boost::async::io
{

system::result<datagram_socket> datagram_socket::duplicate()
{
  auto res = detail::duplicate_socket(datagram_socket_.native_handle());
  if (!res)
    return res.error();

  return {system::in_place_value, datagram_socket(*res)};
}



datagram_socket::datagram_socket()
  : socket(datagram_socket_), datagram_socket_(this_thread::get_executor())
{
}

datagram_socket::datagram_socket(native_handle_type h, protocol_type protocol)
    : socket(datagram_socket_), datagram_socket_(this_thread::get_executor(), protocol, h)
{
}

datagram_socket::datagram_socket(datagram_socket && lhs)
  : socket(datagram_socket_), datagram_socket_(std::move(lhs.datagram_socket_))
{
}
datagram_socket::datagram_socket(endpoint ep)
  : socket(datagram_socket_), datagram_socket_(this_thread::get_executor(), ep)
{
}

auto datagram_socket::receive(buffers::mutable_buffer_span buffers) -> receive_op_seq_
{
  return receive_op_seq_{datagram_socket_, buffers};
}
auto datagram_socket::receive(buffers::mutable_buffer      buffer) -> receive_op_
{
  return receive_op_{datagram_socket_, buffer};
}
auto datagram_socket::receive_from(buffers::mutable_buffer_span buffers, endpoint & ep) -> receive_from_op_seq_
{
  return receive_from_op_seq_{datagram_socket_, buffers, ep};
}
auto datagram_socket::receive_from(buffers::mutable_buffer      buffer,  endpoint & ep) -> receive_from_op_
{
  return receive_from_op_{datagram_socket_, buffer, ep};
}
auto datagram_socket::send(buffers::const_buffer_span buffers) -> send_op_seq_
{
  return send_op_seq_{datagram_socket_, buffers};
}
auto datagram_socket::send(buffers::const_buffer      buffer) -> send_op_
{
  return send_op_{datagram_socket_, buffer};
}
auto datagram_socket::send_to(buffers::const_buffer_span buffers, const endpoint & target) -> send_to_op_seq_
{
  return send_to_op_seq_{datagram_socket_, buffers, target};
}
auto datagram_socket::send_to(buffers::const_buffer      buffer,  const endpoint & target) -> send_to_op_
{
  return send_to_op_{datagram_socket_, buffer, target};
}

void datagram_socket::receive_op_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_receive(buffers::mutable_buffer_span{&buffer_, 1u}, std::move(h));
}

void datagram_socket::receive_op_seq_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_receive(buffer_, std::move(h));
}

void datagram_socket::receive_from_op_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_receive_from(buffers::mutable_buffer_span{&buffer_, 1u}, ep_, std::move(h));
}

void datagram_socket::receive_from_op_seq_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_receive_from(buffer_, ep_, std::move(h));
}

void datagram_socket::send_op_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_send(buffers::const_buffer_span{&buffer_, 1u}, std::move(h));
}

void datagram_socket::send_op_seq_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_send(buffer_, std::move(h));
}

void datagram_socket::send_to_op_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_send_to(buffers::const_buffer_span{&buffer_, 1u}, ep_, std::move(h));
}

void datagram_socket::send_to_op_seq_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->datagram_socket_.async_send_to(buffer_, ep_, std::move(h));
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


}