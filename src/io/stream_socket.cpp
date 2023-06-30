//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/stream_socket.hpp>
#include <boost/async/io/detail/duplicate.hpp>

namespace boost::async::io
{

system::result<stream_socket> stream_socket::duplicate()
{
  auto res = detail::duplicate_socket(stream_socket_.native_handle());
  if (!res)
    return res.error();

  return {system::in_place_value, stream_socket(*res)};
}



stream_socket::stream_socket()
  : socket(stream_socket_), stream_socket_(this_thread::get_executor())
{
}

stream_socket::stream_socket(native_handle_type h, protocol_type protocol)
    : socket(stream_socket_), stream_socket_(this_thread::get_executor(), protocol, h)
{
}

stream_socket::stream_socket(stream_socket && lhs)
  : socket(stream_socket_), stream_socket_(std::move(lhs.stream_socket_))
{
}
stream_socket::stream_socket(endpoint ep)
  : socket(stream_socket_), stream_socket_(this_thread::get_executor(), ep)
{
}

void stream_socket::async_read_some_impl_(
    buffers::mutable_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  stream_socket_.async_read_some(buffers::mutable_buffer_span(buffer), std::move(h));
}

void stream_socket::async_write_some_impl_(
    buffers::const_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  stream_socket_.async_write_some(buffer, std::move(h));
}

system::result<void> stream_socket::close() { return socket::close(); }
system::result<void> stream_socket::cancel() { return socket::cancel(); }
bool stream_socket::is_open() const {return socket::is_open();}

void stream_socket::adopt_endpoint_(endpoint & ep)
{

  switch (ep.protocol().family())
  {
    case BOOST_ASIO_OS_DEF(AF_INET): BOOST_FALLTHROUGH;
    case BOOST_ASIO_OS_DEF(AF_INET6):
      if (ep.protocol().protocol() == BOOST_ASIO_OS_DEF(IPPROTO_IP))
        ep.set_protocol(BOOST_ASIO_OS_DEF(IPPROTO_TCP));
    case AF_UNIX:
      if (ep.protocol().type() == 0)
        ep.set_type(BOOST_ASIO_OS_DEF(SOCK_STREAM));
  }
}

}