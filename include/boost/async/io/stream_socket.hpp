//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_STREAM_SOCKET_HPP
#define BOOST_ASYNC_IO_STREAM_SOCKET_HPP

#include <boost/async/io/socket.hpp>
#include <boost/async/io/stream.hpp>
#include <boost/asio/generic/stream_protocol.hpp>
#include <boost/asio/basic_stream_socket.hpp>

namespace boost::async::io
{

struct [[nodiscard]] stream_socket final : stream, socket
{
  // duplicate onto another thread
  BOOST_ASYNC_DECL system::result<stream_socket> duplicate(const async::executor & executor = this_thread::get_executor());

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> close() override;
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> cancel() override;
  [[nodiscard]] BOOST_ASYNC_DECL bool is_open() const override;

  BOOST_ASYNC_DECL stream_socket(const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL stream_socket(stream_socket && lhs);
  BOOST_ASYNC_DECL stream_socket(native_handle_type h, protocol_type protocol = protocol_type(),
                                 const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL stream_socket(endpoint ep, const async::executor & executor = this_thread::get_executor());
 private:
  BOOST_ASYNC_DECL void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  BOOST_ASYNC_DECL void async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  asio::basic_stream_socket<protocol_type, executor> stream_socket_;
  friend struct ssl_stream;
  BOOST_ASYNC_DECL void adopt_endpoint_(endpoint & ep) override;
};

inline system::result<std::pair<stream_socket, stream_socket>> make_pair(decltype(local_stream) protocol)
{
  std::pair<stream_socket, stream_socket> res;
  auto c = connect_pair(protocol, res.first, res.second);
  if (c)
    return res;
  else
    return c.error();
}

}

#endif //BOOST_ASYNC_IO_STREAM_SOCKET_HPP
