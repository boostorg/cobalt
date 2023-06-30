//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SOCKET_HPP
#define BOOST_ASYNC_IO_SOCKET_HPP

#include <boost/async/io/endpoint.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/basic_socket.hpp>

namespace boost::async::io
{

struct socket
{
  [[nodiscard]] system::result<void> open(protocol_type prot = protocol_type {});
  [[nodiscard]] system::result<void> close();
  [[nodiscard]] system::result<void> cancel();
  [[nodiscard]] bool is_open() const;

  // asio acceptor compatibility
  template<typename T>
  struct rebind_executor {using other = socket;};

  using shutdown_type      = asio::socket_base::shutdown_type;
  using wait_type          = asio::socket_base::wait_type;
  using message_flags      = asio::socket_base::message_flags;
  constexpr static int message_peek          = asio::socket_base::message_peek;
  constexpr static int message_out_of_band   = asio::socket_base::message_out_of_band;
  constexpr static int message_do_not_route  = asio::socket_base::message_do_not_route;
  constexpr static int message_end_of_record = asio::socket_base::message_end_of_record;

  using native_handle_type = asio::basic_socket<protocol_type, executor_type>::native_handle_type;
  native_handle_type native_handle();

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> shutdown(shutdown_type = shutdown_type::shutdown_both);

  [[nodiscard]] BOOST_ASYNC_DECL system::result<endpoint> local_endpoint() const;
  [[nodiscard]] BOOST_ASYNC_DECL system::result<endpoint> remote_endpoint() const;


  BOOST_ASYNC_DECL system::result<void> assign(protocol_type protocol, native_handle_type native_handle);
  BOOST_ASYNC_DECL system::result<native_handle_type> release();

  /// copied from what asio does
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::size_t> bytes_readable();

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> set_debug(bool debug);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> get_debug() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> set_do_not_route(bool do_not_route);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> get_do_not_route() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> set_enable_connection_aborted(bool enable_connection_aborted);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> get_enable_connection_aborted() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> set_keep_alive(bool keep_alive);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> get_keep_alive() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> set_linger(bool linger, int timeout);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::pair<bool, int>> get_linger() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>        set_receive_buffer_size(std::size_t receive_buffer_size);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::size_t> get_receive_buffer_size() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>        set_send_buffer_size(std::size_t send_buffer_size);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::size_t> get_send_buffer_size() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>        set_receive_low_watermark(std::size_t receive_low_watermark);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::size_t> get_receive_low_watermark() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>        set_send_low_watermark(std::size_t send_low_watermark);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::size_t> get_send_low_watermark() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> set_reuse_address(bool reuse_address);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> get_reuse_address() const;

 private:

  virtual void adopt_endpoint_(endpoint & ) {}
  struct connect_op_;
  struct wait_op_;
  friend struct acceptor;
  asio::basic_socket<protocol_type, executor_type> & socket_;

 public:
  socket(asio::basic_socket<protocol_type, executor_type> & socket) : socket_(socket) {}

  BOOST_ASYNC_DECL connect_op_  connect(endpoint ep);
  BOOST_ASYNC_DECL wait_op_     wait(wait_type = wait_type::wait_read);
  BOOST_ASYNC_DECL wait_op_ operator co_await(); // co_await sock;
};

BOOST_ASYNC_DECL system::result<void> connect_pair(protocol_type protocol, socket & socket1, socket & socket2);

}

#include <boost/async/io/detail/socket.hpp>

#endif //BOOST_ASYNC_IO_SOCKET_HPP
