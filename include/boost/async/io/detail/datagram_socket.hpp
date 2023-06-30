//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_DATAGRAM_SOCKET_HPP
#define BOOST_ASYNC_IO_DETAIL_DATAGRAM_SOCKET_HPP

#include <boost/async/io/datagram_socket.hpp>
#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct datagram_socket::receive_op_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  receive_op_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
              buffers::mutable_buffer buffer)
    : datagram_socket_(rs), buffer_(buffer) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::mutable_buffer buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct datagram_socket::receive_op_seq_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  receive_op_seq_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
                  buffers::mutable_buffer_span buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::mutable_buffer_span buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct datagram_socket::receive_from_op_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  receive_from_op_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
              buffers::mutable_buffer buffer, endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::mutable_buffer buffer_;
  endpoint &ep_;

  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct datagram_socket::receive_from_op_seq_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  receive_from_op_seq_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
                  buffers::mutable_buffer_span buffer, endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::mutable_buffer_span buffer_;
  endpoint & ep_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};

struct datagram_socket::send_op_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  send_op_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
           buffers::const_buffer buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::const_buffer buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct datagram_socket::send_op_seq_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  send_op_seq_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
                  buffers::const_buffer_span buffer)
      : datagram_socket_(rs), buffer_(buffer) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::const_buffer_span buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct datagram_socket::send_to_op_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  send_to_op_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
                   buffers::const_buffer buffer, const endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::const_buffer buffer_;
  const endpoint &ep_;

  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct datagram_socket::send_to_op_seq_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      initiate_(completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }
  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    const auto & [ec, n] = *result_;
    return transfer_result{ec, n};
  }
  send_to_op_seq_(asio::basic_datagram_socket<protocol_type, executor_type> & rs,
                       buffers::const_buffer_span buffer, const endpoint & ep)
      : datagram_socket_(rs), buffer_(buffer), ep_(ep) {}
 private:
  void initiate_(async::completion_handler<system::error_code, std::size_t> h);
  asio::basic_datagram_socket<protocol_type, executor_type> &datagram_socket_;
  buffers::const_buffer_span buffer_;
  const endpoint & ep_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_DATAGRAM_SOCKET_HPP
