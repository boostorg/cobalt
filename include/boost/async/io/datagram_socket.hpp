//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DATAGRAM_SOCKET_HPP
#define BOOST_ASYNC_IO_DATAGRAM_SOCKET_HPP

#include <boost/async/io/socket.hpp>
#include <boost/async/io/buffers.hpp>

#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/basic_datagram_socket.hpp>

namespace boost::async::io
{

struct [[nodiscard]] datagram_socket final : socket
{
  // duplicate onto another thread
  system::result<datagram_socket> duplicate();

  datagram_socket();
  datagram_socket(datagram_socket && lhs);
  datagram_socket(native_handle_type h, protocol_type protocol = protocol_type());
  datagram_socket(endpoint ep);
 private:
  struct receive_op_;
  struct receive_op_seq_;
  struct receive_from_op_;
  struct receive_from_op_seq_;
  struct send_op_;
  struct send_op_seq_;
  struct send_to_op_;
  struct send_to_op_seq_;
  BOOST_ASYNC_DECL void adopt_endpoint_(endpoint & ep) override;

 public:

  [[nodiscard]] receive_op_seq_ receive(buffers::mutable_buffer_span buffers);
  [[nodiscard]] receive_op_     receive(buffers::mutable_buffer      buffer);
  [[nodiscard]] receive_from_op_seq_ receive_from(buffers::mutable_buffer_span buffers, endpoint & ep);
  [[nodiscard]] receive_from_op_     receive_from(buffers::mutable_buffer      buffer,  endpoint & ep);
  [[nodiscard]] send_op_seq_    send(buffers::const_buffer_span buffers);
  [[nodiscard]] send_op_        send(buffers::const_buffer      buffer);
  [[nodiscard]] send_to_op_seq_ send_to(buffers::const_buffer_span buffers, const endpoint & target);
  [[nodiscard]] send_to_op_     send_to(buffers::const_buffer      buffer,  const endpoint & target);

  asio::basic_datagram_socket<protocol_type, executor_type> datagram_socket_;
};

inline system::result<std::pair<datagram_socket, datagram_socket>> make_pair(decltype(local_datagram) protocol)
{
  std::pair<datagram_socket, datagram_socket> res;
  auto c = connect_pair(protocol, res.first, res.second);
  if (c)
    return res;
  else
    return c.error();
}


}

#include <boost/async/io/detail/datagram_socket.hpp>

#endif //BOOST_ASYNC_IO_DATAGRAM_SOCKET_HPP
