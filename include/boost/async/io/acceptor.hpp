//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_ACCEPTOR_HPP
#define BOOST_ASYNC_IO_ACCEPTOR_HPP

#include <boost/async/io/result.hpp>
#include <boost/async/io/stream_socket.hpp>
#include <boost/async/io/seq_packet_socket.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>


namespace boost::async::io
{

struct acceptor
{
  BOOST_ASYNC_DECL acceptor(const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL acceptor(endpoint ep, const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL system::result<void> bind(endpoint ep);
  BOOST_ASYNC_DECL system::result<void> listen(int backlog = asio::socket_base::max_listen_connections); // int backlog = asio::max_backlog()
  BOOST_ASYNC_DECL endpoint local_endpoint();

 private:

  struct accept_op_ final : result_op<void>
  {
    accept_op_(asio::basic_socket_acceptor<protocol_type, executor> & acceptor,
               socket &socket)
      : acceptor_(acceptor), socket_(socket) {}
    BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code> h) override;

   private:
    asio::basic_socket_acceptor<protocol_type, executor> & acceptor_;
    socket &socket_;
  };

 public:
  [[nodiscard]] BOOST_ASYNC_DECL accept_op_     accept(socket & sock);
 private:
  asio::basic_socket_acceptor<protocol_type, executor> acceptor_;
};

}

#endif //BOOST_ASYNC_IO_ACCEPTOR_HPP
