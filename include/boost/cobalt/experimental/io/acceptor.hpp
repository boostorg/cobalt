//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_ACCEPTOR_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_ACCEPTOR_HPP


#include <boost/cobalt/experimental/io/ops.hpp>
#include <boost/cobalt/experimental/io/stream_socket.hpp>
#include <boost/cobalt/experimental/io/seq_packet_socket.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>

namespace boost::cobalt::experimental::io
{

struct acceptor : asio::socket_base
{
  BOOST_COBALT_DECL acceptor(const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL acceptor(endpoint ep, const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL system::result<void> bind(endpoint ep);
  BOOST_COBALT_DECL system::result<void> listen(int backlog = max_listen_connections); // int backlog = asio::max_backlog()
  BOOST_COBALT_DECL endpoint local_endpoint();

  struct accept_op
  {
    socket * sock;

    void *this_;
    void (*implementation)(void * this_, socket *,
                           boost::cobalt::completion_handler<boost::system::error_code>);

    op_awaitable<accept_op, std::tuple<socket *>, boost::system::error_code>
        operator co_await()
    {
      return {this, sock};
    }
  };

  accept_op accept(socket & sock)
  {
    return {&sock, this, initiate_accept_};
  }

  struct wait_op
  {
    wait_type wt;

    void *this_;
    void (*implementation)(void * this_, wait_type wt,
                           boost::cobalt::completion_handler<boost::system::error_code>);

    op_awaitable<wait_op, std::tuple<wait_type>, boost::system::error_code>
        operator co_await()
    {
      return {this, wt};
    }
  };
  wait_op     wait(wait_type wt = wait_type::wait_read)
  {
    return {wt, this, initiate_wait_};
  }

 private:
  BOOST_COBALT_DECL static void initiate_accept_ (void *, socket *, completion_handler<system::error_code>);
  BOOST_COBALT_DECL static void initiate_wait_(void *, wait_type, completion_handler<boost::system::error_code>);

  asio::basic_socket_acceptor<protocol_type, executor> acceptor_;
};

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_ACCEPTOR_HPP
