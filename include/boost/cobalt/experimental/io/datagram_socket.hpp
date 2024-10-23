//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_DATAGRAM_SOCKET_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_DATAGRAM_SOCKET_HPP

#include <boost/cobalt/experimental/io/endpoint.hpp>
#include <boost/cobalt/experimental/io/socket.hpp>

#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/basic_datagram_socket.hpp>

namespace boost::cobalt::experimental::io
{

struct [[nodiscard]] datagram_socket final : socket
{
  BOOST_COBALT_DECL datagram_socket(const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL datagram_socket(datagram_socket && lhs);
  BOOST_COBALT_DECL datagram_socket(native_handle_type h, protocol_type protocol = protocol_type(),
                  const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL datagram_socket(endpoint ep,
                  const cobalt::executor & executor = this_thread::get_executor());

  write_op send(const_buffer_sequence buffer)
  {
    return {buffer, this, initiate_send_};
  }
  read_op receive(mutable_buffer_sequence buffer)
  {
    return {buffer, this, initiate_receive_};
  }

 public:
  BOOST_COBALT_DECL void adopt_endpoint_(endpoint & ep) override;

  BOOST_COBALT_DECL static void initiate_receive_(void *, mutable_buffer_sequence, boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);
  BOOST_COBALT_DECL static void initiate_send_   (void *, const_buffer_sequence, boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  asio::basic_datagram_socket<protocol_type, executor> datagram_socket_;
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

#endif //BOOST_COBALT_EXPERIMENTAL_IO_DATAGRAM_SOCKET_HPP
