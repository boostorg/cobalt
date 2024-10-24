//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_SEQ_PACKET_SOCKET_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_SEQ_PACKET_SOCKET_HPP

#include <boost/cobalt/experimental/io/endpoint.hpp>
#include <boost/cobalt/experimental/io/socket.hpp>

#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/basic_seq_packet_socket.hpp>

namespace boost::cobalt::experimental::io
{

struct [[nodiscard]] seq_packet_socket final : socket
{
  // duplicate onto another thread
  system::result<seq_packet_socket> duplicate(const cobalt::executor & executor = this_thread::get_executor());

  BOOST_COBALT_DECL seq_packet_socket(const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL seq_packet_socket(seq_packet_socket && lhs);
  BOOST_COBALT_DECL seq_packet_socket(native_handle_type h, protocol_type protocol = protocol_type(),
                  const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL seq_packet_socket(endpoint ep,
                  const cobalt::executor & executor = this_thread::get_executor());


  struct send_op
  {
    message_flags in_flags;
    const_buffer_sequence buffer;

    void *this_;
    void (*implementation)(void * this_, message_flags, const_buffer_sequence,
                           boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

    basic_awaitable<send_op, std::tuple<message_flags, const_buffer_sequence>, boost::system::error_code, std::size_t>
        operator co_await()
    {
      return {this, in_flags, buffer};
    }
  };

  struct receive_op
  {
    message_flags in_flags, *out_flags;
    mutable_buffer_sequence buffer;

    void *this_;
    void (*implementation)(void * this_, message_flags, message_flags*, mutable_buffer_sequence,
                           boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

    basic_awaitable<receive_op, std::tuple<message_flags, message_flags*, mutable_buffer_sequence>, boost::system::error_code, std::size_t>
        operator co_await()
    {
      return {this, in_flags, out_flags, buffer};
    }
  };

  receive_op receive(message_flags in_flags, message_flags& out_flags, mutable_buffer_sequence buffer)
  {
    return {in_flags, &out_flags, buffer, this, initiate_receive_};
  }

  send_op send(message_flags in_flags, const_buffer_sequence buffer)
  {
    return {in_flags, buffer, this, initiate_send_};
  }
  receive_op receive(message_flags in_flags, mutable_buffer_sequence buffer)
  {
    return {in_flags, nullptr, buffer, this, initiate_receive_};
  }

 public:
  BOOST_COBALT_DECL void adopt_endpoint_(endpoint & ep) override;

  BOOST_COBALT_DECL static void initiate_receive_(void *, message_flags, message_flags*, mutable_buffer_sequence, boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);
  BOOST_COBALT_DECL static void initiate_send_   (void *, message_flags, const_buffer_sequence, boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

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

#endif //BOOST_COBALT_EXPERIMENTAL_IO_SEQ_PACKET_SOCKET_HPP
