//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_DATAGRAM_SOCKET_HPP
#define BOOST_ASYNC_IO_DETAIL_DATAGRAM_SOCKET_HPP

#include <boost/async/io/datagram_socket.hpp>
#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct datagram_socket::receive_op_ final : transfer_op
{
  receive_op_(asio::basic_datagram_socket<protocol_type, executor> & rs,
              buffers::mutable_buffer buffer)
    : datagram_socket_(rs), buffer_(buffer) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h) override;
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::mutable_buffer buffer_;
};


struct datagram_socket::receive_op_seq_ final : transfer_op
{
  receive_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                  buffers::mutable_buffer_subspan buffer)
      : datagram_socket_(rs), buffer_(buffer) {}

  receive_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                  buffers::mutable_buffer_span buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h) override;
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::mutable_buffer_subspan buffer_;
};


struct datagram_socket::receive_from_op_ final : transfer_op
{
  receive_from_op_(asio::basic_datagram_socket<protocol_type, executor> & rs,
              buffers::mutable_buffer buffer, endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h);
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::mutable_buffer buffer_;
  endpoint &ep_;
};


struct datagram_socket::receive_from_op_seq_ final : transfer_op
{
  receive_from_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                       buffers::mutable_buffer_subspan buffer, endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
  receive_from_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                       buffers::mutable_buffer_span buffer, endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h);
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::mutable_buffer_subspan buffer_;
  endpoint & ep_;
};

struct datagram_socket::send_op_ final : transfer_op
{
  send_op_(asio::basic_datagram_socket<protocol_type, executor> & rs,
           buffers::const_buffer buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h);
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::const_buffer buffer_;
};


struct datagram_socket::send_op_seq_ final : transfer_op
{
  send_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                  buffers::const_buffer_subspan buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
  send_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
               buffers::const_buffer_span buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h);
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::const_buffer_subspan buffer_;
};


struct datagram_socket::send_to_op_ final : transfer_op
{
  send_to_op_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                   buffers::const_buffer buffer, const endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h);
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::const_buffer buffer_;
  const endpoint &ep_;
};


struct datagram_socket::send_to_op_seq_ final : transfer_op
{
  send_to_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                       buffers::const_buffer_subspan buffer, const endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}

  send_to_op_seq_(asio::basic_datagram_socket<protocol_type, executor> & rs,
                  buffers::const_buffer_span buffer, const endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
  void initiate(async::completion_handler<system::error_code, std::size_t> h);
 private:
  asio::basic_datagram_socket<protocol_type, executor> &datagram_socket_;
  buffers::const_buffer_subspan buffer_;
  const endpoint & ep_;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_DATAGRAM_SOCKET_HPP
