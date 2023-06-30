//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_SOCKET_HPP
#define BOOST_ASYNC_IO_DETAIL_SOCKET_HPP

#include <boost/async/io/socket.hpp>
#include <boost/async/op.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace boost::async::io
{

struct socket::wait_op_ : detail::deferred_op_resource_base
{
  constexpr static bool await_ready() { return false; }

  BOOST_ASYNC_DECL void init_op(completion_handler<system::error_code> handler);

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      init_op(completion_handler<system::error_code>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  [[nodiscard]] system::result<void> await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    auto [ec] = result_.value_or(std::make_tuple(system::error_code{}));
    if (ec)
      return ec;
    else
      return {};
  }
  wait_op_(asio::basic_socket<protocol_type, executor_type> & socket,
           asio::socket_base::wait_type wt) : socket_(socket), wt_(wt) {}
 private:
  asio::basic_socket<protocol_type, executor_type>  & socket_;
  asio::socket_base::wait_type wt_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code>> result_;
};


struct socket::connect_op_ : detail::deferred_op_resource_base
{
  constexpr static bool await_ready() { return false; }

  BOOST_ASYNC_DECL void init_op(completion_handler<system::error_code> handler);

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      init_op(completion_handler<system::error_code>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  [[nodiscard]] system::result<void> await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    auto [ec] = result_.value_or(std::make_tuple(system::error_code{}));
    if (ec)
      return ec;
    else
      return {};
  }
  connect_op_(socket* socket,
              endpoint ep) : socket_(socket), ep_(ep) {}
 private:
  socket * socket_;
  endpoint ep_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code>> result_;
};


}

#endif //BOOST_ASYNC_IO_SOCKET_HPP
