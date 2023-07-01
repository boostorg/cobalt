//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SEQ_PACKET_SOCKET_HPP
#define BOOST_ASYNC_IO_SEQ_PACKET_SOCKET_HPP

#include <boost/async/io/socket.hpp>
#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/basic_seq_packet_socket.hpp>
#include <boost/async/io/buffers.hpp>

namespace boost::async::io
{

struct [[nodiscard]] seq_packet_socket final : socket
{
  // duplicate onto another thread
  system::result<seq_packet_socket> duplicate();

  using message_flags = boost::asio::socket_base::message_flags;
  /// Peek at incoming data without removing it from the input queue.
  static constexpr int message_peek = boost::asio::socket_base::message_peek;

  /// Process out-of-band data.
  static constexpr int message_out_of_band = boost::asio::socket_base::message_out_of_band;

  /// Specify that the data should not be subject to routing.
  static constexpr int message_do_not_route = boost::asio::socket_base::message_do_not_route;

  /// Specifies that the data marks the end of a record.
  static constexpr int message_end_of_record = boost::asio::socket_base::message_end_of_record;

  BOOST_ASYNC_DECL seq_packet_socket();
  BOOST_ASYNC_DECL seq_packet_socket(seq_packet_socket && lhs);
  BOOST_ASYNC_DECL seq_packet_socket(native_handle_type h, protocol_type protocol = protocol_type());
  BOOST_ASYNC_DECL seq_packet_socket(endpoint ep);
 private:
  struct receive_op_;
  struct receive_op_seq_;
  struct send_op_;
  struct send_op_seq_;
  BOOST_ASYNC_DECL void adopt_endpoint_(endpoint & ep) override;
 public:

  [[nodiscard]] BOOST_ASYNC_DECL receive_op_seq_ receive(buffers::mutable_buffer_subspan buffers, message_flags & out_flags);
  [[nodiscard]] BOOST_ASYNC_DECL receive_op_seq_ receive(buffers::mutable_buffer_span    buffers, message_flags & out_flags);
  [[nodiscard]] BOOST_ASYNC_DECL receive_op_     receive(buffers::mutable_buffer         buffer,  message_flags & out_flags);
  [[nodiscard]] BOOST_ASYNC_DECL send_op_seq_    send(buffers::const_buffer_subspan buffers, message_flags out_flags);
  [[nodiscard]] BOOST_ASYNC_DECL send_op_seq_    send(buffers::const_buffer_span    buffers, message_flags out_flags);
  [[nodiscard]] BOOST_ASYNC_DECL send_op_        send(buffers::const_buffer         buffer,  message_flags out_flags);

  asio::basic_seq_packet_socket<protocol_type, executor> seq_packet_socket_;
};


inline system::result<std::pair<seq_packet_socket, seq_packet_socket>> make_pair(decltype(local_seqpacket) protocol)
{
  std::pair<seq_packet_socket, seq_packet_socket> res;
  auto c = connect_pair(protocol, res.first, res.second);
  if (c)
    return res;
  else
    return c.error();
}

}

#include <boost/async/io/detail/seq_packet_socket.hpp>

#endif //BOOST_ASYNC_IO_SEQ_PACKET_SOCKET_HPP
