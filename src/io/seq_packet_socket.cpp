//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/seq_packet_socket.hpp>
#include <boost/async/io/detail/duplicate.hpp>

namespace boost::async::io
{

system::result<seq_packet_socket> seq_packet_socket::duplicate()
{
  auto res = detail::duplicate_socket(seq_packet_socket_.native_handle());
  if (!res)
    return res.error();

  return {system::in_place_value, seq_packet_socket(*res)};
}



seq_packet_socket::seq_packet_socket()
  : socket(seq_packet_socket_), seq_packet_socket_(this_thread::get_executor())
{
}

seq_packet_socket::seq_packet_socket(native_handle_type h, protocol_type protocol)
    : socket(seq_packet_socket_), seq_packet_socket_(this_thread::get_executor(), protocol, h)
{
}

seq_packet_socket::seq_packet_socket(seq_packet_socket && lhs)
  : socket(seq_packet_socket_), seq_packet_socket_(std::move(lhs.seq_packet_socket_))
{
}
seq_packet_socket::seq_packet_socket(endpoint ep)
  : socket(seq_packet_socket_), seq_packet_socket_(this_thread::get_executor(), ep)
{
}

auto seq_packet_socket::receive(buffers::mutable_buffer_span buffers, message_flags &out_flags) -> receive_op_seq_
{
  return receive_op_seq_{seq_packet_socket_, buffers, out_flags};
}
auto seq_packet_socket::receive(buffers::mutable_buffer      buffer, message_flags &out_flags) -> receive_op_
{
  return receive_op_{seq_packet_socket_, buffer, out_flags};
}
auto seq_packet_socket::send(buffers::const_buffer_span buffers, message_flags out_flags) -> send_op_seq_
{
  return send_op_seq_{seq_packet_socket_, buffers, out_flags};
}
auto seq_packet_socket::send(buffers::const_buffer      buffer,message_flags out_flags) -> send_op_
{
  return send_op_{seq_packet_socket_, buffer, out_flags};
}

void seq_packet_socket::receive_op_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->seq_packet_socket_.async_receive(buffers::mutable_buffer_span{&buffer_, 1u}, out_flags_, std::move(h));
}

void seq_packet_socket::receive_op_seq_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->seq_packet_socket_.async_receive(buffer_, out_flags_, std::move(h));
}

void seq_packet_socket::send_op_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->seq_packet_socket_.async_send(buffers::const_buffer_span{&buffer_, 1u}, out_flags_, std::move(h));
}

void seq_packet_socket::send_op_seq_::initiate_(async::completion_handler<system::error_code, std::size_t> h)
{
  this->seq_packet_socket_.async_send(buffer_, out_flags_, std::move(h));
}


}