//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_SEQ_PACKET_SOCKET_HPP
#define BOOST_ASYNC_IO_DETAIL_SEQ_PACKET_SOCKET_HPP

#include <boost/async/io/seq_packet_socket.hpp>
#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct seq_packet_socket::receive_op_ final : transfer_op
{
  receive_op_(asio::basic_seq_packet_socket<protocol_type, executor> & rs,
              buffers::mutable_buffer buffer, message_flags &out_flags)
    : seq_packet_socket_(rs), buffer_(buffer), out_flags_(out_flags) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h) override;
 private:
  asio::basic_seq_packet_socket<protocol_type, executor> &seq_packet_socket_;
  buffers::mutable_buffer buffer_;
  message_flags &out_flags_;
};


struct seq_packet_socket::receive_op_seq_ final : transfer_op
{
  receive_op_seq_(asio::basic_seq_packet_socket<protocol_type, executor> & rs,
                  buffers::mutable_buffer_subspan buffer, message_flags &out_flags)
      : seq_packet_socket_(rs), buffer_(buffer), out_flags_(out_flags) {}
  receive_op_seq_(asio::basic_seq_packet_socket<protocol_type, executor> & rs,
                  buffers::mutable_buffer_span buffer, message_flags &out_flags)
      : seq_packet_socket_(rs), buffer_(buffer), out_flags_(out_flags) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h) override;
 private:
  asio::basic_seq_packet_socket<protocol_type, executor> &seq_packet_socket_;
  buffers::mutable_buffer_subspan buffer_;
  message_flags &out_flags_;
};

struct seq_packet_socket::send_op_ final : transfer_op
{
  send_op_(asio::basic_seq_packet_socket<protocol_type, executor> & rs,
           buffers::const_buffer buffer, message_flags out_flags)
      : seq_packet_socket_(rs), buffer_(buffer), out_flags_(out_flags) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h) override;
 private:
  asio::basic_seq_packet_socket<protocol_type, executor> &seq_packet_socket_;
  buffers::const_buffer buffer_;
  message_flags out_flags_;
};


struct seq_packet_socket::send_op_seq_ final : transfer_op
{
  send_op_seq_(asio::basic_seq_packet_socket<protocol_type, executor> & rs,
               buffers::const_buffer_subspan buffer, message_flags out_flags)
      : seq_packet_socket_(rs), buffer_(buffer), out_flags_(out_flags) {}
  send_op_seq_(asio::basic_seq_packet_socket<protocol_type, executor> & rs,
               buffers::const_buffer_span buffer, message_flags out_flags)
      : seq_packet_socket_(rs), buffer_(buffer), out_flags_(out_flags) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h) override;
 private:
  asio::basic_seq_packet_socket<protocol_type, executor> &seq_packet_socket_;
  buffers::const_buffer_subspan buffer_;
  message_flags out_flags_;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_SEQ_PACKET_SOCKET_HPP
