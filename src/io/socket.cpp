//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/socket.hpp>
#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>

namespace boost::cobalt::io
{
using boost::system::result;
using boost::system::error_code;


result<void> socket::open(protocol_type prot)
{
  error_code ec;
  socket_.open(prot, ec);
  return ec ? ec : result<void>{};
}

result<void> socket::close()
{
  error_code ec;
  socket_.close(ec);
  return ec ? ec : result<void>{};
}

result<void> socket::cancel()
{
  error_code ec;
  socket_.cancel(ec);
  return ec ? ec : result<void>{};
}

bool socket::is_open() const
{
  return socket_.is_open();
}

auto socket::native_handle() -> native_handle_type { return socket_.native_handle(); }

result<void> socket::shutdown(shutdown_type st)
{
  error_code ec;
  socket_.shutdown(st, ec);
  return ec ? ec : result<void>{};
}

result<endpoint> socket::local_endpoint() const
{
  error_code ec;
  auto res = socket_.local_endpoint(ec);
  return ec ? ec : result<endpoint>(res);
}

result<endpoint> socket::remote_endpoint() const
{
  error_code ec;
  auto res = socket_.remote_endpoint(ec);
  return ec ? ec : result<endpoint>(res);
}


result<void> socket::assign(protocol_type protocol, native_handle_type native_handle)
{
  error_code ec;
  socket_.assign(protocol, native_handle, ec);
  return ec ? ec : result<void>{};
}



auto socket::release() -> result<native_handle_type>
{
  error_code ec;
  auto h = socket_.release(ec);
  return ec ? ec : result<native_handle_type>(h);
}

result <std::size_t> socket::bytes_readable()
{
  error_code ec;
  asio::socket_base::bytes_readable opt;
  socket_.io_control(opt, ec);
  return ec ? ec : result<std::size_t>(opt.get());
}

#define DEFINE_OPTION(Name, Type)                                                     \
result<void> socket::set_##Name(Type value)                                           \
{                                                                                     \
  error_code ec;                                                                      \
  socket_.set_option(asio::socket_base::Name(value), ec);                             \
  return ec ? ec : result<void>{};                                                    \
}                                                                                     \
                                                                                      \
result<Type> socket::get_##Name() const                                               \
{                                                                                     \
  error_code ec;                                                                      \
  asio::socket_base::Name opt;                                                        \
  socket_.get_option(opt, ec);                                                        \
  return ec                                                                           \
       ? result<Type>(boost::system::in_place_error, ec)                              \
       : result<Type>(boost::system::in_place_value, static_cast<Type>(opt.value())); \
}

DEFINE_OPTION(debug, bool);
DEFINE_OPTION(do_not_route, bool);
DEFINE_OPTION(enable_connection_aborted , bool);
DEFINE_OPTION(keep_alive , bool);
DEFINE_OPTION(receive_buffer_size,   int);
DEFINE_OPTION(send_buffer_size,      int);
DEFINE_OPTION(receive_low_watermark, int);
DEFINE_OPTION(send_low_watermark,    int);
DEFINE_OPTION(reuse_address, bool);

#define DEFINE_TCP_OPTION(Name, Type)                                          \
result<void> socket::set_##Name(Type value)                                    \
{                                                                              \
  error_code ec;                                                               \
  socket_.set_option(asio::ip::tcp::Name(value), ec);                          \
  return ec ? ec : result<void>{};                                             \
}                                                                              \
                                                                               \
result<Type> socket::get_##Name() const                                        \
{                                                                              \
  error_code ec;                                                               \
  asio::ip::tcp::Name opt;                                                     \
  socket_.get_option(opt, ec);                                                 \
  return ec                                                                    \
       ? result<Type>(boost::system::in_place_error, ec)                       \
       : result<Type>(boost::system::in_place_value, opt.value());             \
}

DEFINE_TCP_OPTION(no_delay, bool);

result<void> socket::set_linger(bool linger, int timeout)
{
  error_code ec;
  socket_.set_option(asio::socket_base::linger(linger, timeout), ec);
  return ec ? ec : result<void>{};
}

result<std::pair<bool, int>> socket::get_linger() const
{
  error_code ec;
  asio::socket_base::linger opt;
  socket_.get_option(opt, ec);
  return ec ? ec : result<std::pair<bool, int>>(opt.enabled(), opt.timeout());
}


result<void> connect_pair(protocol_type protocol, socket & socket1, socket & socket2)
{
  error_code ec;
  asio::detail::socket_type sv[2];
  if (asio::detail::socket_ops::socketpair(protocol.family(),
                                                  protocol.type(), protocol.protocol(), sv, ec)
      == asio::detail::socket_error_retval)
    return ec;

  auto res = socket1.assign(protocol, sv[0]);
  if (!res)
  {
    error_code temp_ec;
    asio::detail::socket_ops::state_type state[2] = { 0, 0 };
    asio::detail::socket_ops::close(sv[0], state[0], true, temp_ec);
    asio::detail::socket_ops::close(sv[1], state[1], true, temp_ec);
    return res;
  }

  res = socket2.assign(protocol, sv[1]);
  if (!res)
  {
    error_code temp_ec;
    (void)socket1.close();
    asio::detail::socket_ops::state_type state = 0;
    asio::detail::socket_ops::close(sv[1], state, true, temp_ec);
    return res;
  }

  return {};
}

void socket::wait_op::ready(handler<error_code> h)
{
  if (wt == wait_type::wait_read)
  {
    auto br = sock_.bytes_readable();
    if (br && *br > 0u)
      h({});
  }
}

void socket::wait_op::initiate(completion_handler<error_code> handler)
{
  sock_.socket_.async_wait(wt, std::move(handler));
}

void socket::connect_op::initiate(boost::cobalt::completion_handler<system::error_code> handler)
{
  sock_.adopt_endpoint_(endpoint);
  sock_.socket_.async_connect(endpoint, std::move(handler));
}

void socket::ranged_connect_op::initiate(completion_handler<error_code, endpoint> handler)
{
  for (auto & ep : endpoints)
    sock_.adopt_endpoint_(ep);
  boost::asio::async_connect(
      sock_.socket_, endpoints, std::move(handler));
}


}
