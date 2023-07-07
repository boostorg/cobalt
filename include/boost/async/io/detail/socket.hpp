//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_SOCKET_HPP
#define BOOST_ASYNC_IO_DETAIL_SOCKET_HPP

#include <boost/async/io/socket.hpp>
#include <boost/async/io/result.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost::async::io
{

struct socket::wait_op_ : result_op<void>
{
  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code> handler);

  wait_op_(asio::basic_socket<protocol_type, executor> & socket,
           asio::socket_base::wait_type wt) : socket_(socket), wt_(wt) {}
 private:
  asio::basic_socket<protocol_type, executor>  & socket_;
  asio::socket_base::wait_type wt_;
};


struct socket::connect_op_ : result_op<void>
{
  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code> handler);

  connect_op_(socket* socket,
              endpoint ep) : socket_(socket), ep_(ep) {}
 private:
  socket * socket_;
  endpoint ep_;
};


}

#endif //BOOST_ASYNC_IO_SOCKET_HPP
