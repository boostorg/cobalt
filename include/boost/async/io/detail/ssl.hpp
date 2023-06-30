//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_SSL_HPP
#define BOOST_ASYNC_SSL_HPP

#include <boost/async/io/ssl.hpp>
#include <boost/async/io/buffers.hpp>

namespace boost::async::io
{
struct ssl_stream::accept_op_         : detail::deferred_op_resource_base
{
  constexpr static bool await_ready() { return false; }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      stream.ssl_stream_.async_handshake(ht, completion_handler<system::error_code>{h, result_, get_resource(h)});
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
    auto ec = std::get<0>(result_.value_or(std::make_tuple(system::error_code{})));
    return ec ? system::result<void>(ec) : system::in_place_value;
  }

  accept_op_(ssl_stream & stream, handshake_type ht) : stream(stream), ht(ht) {}
 private:

  ssl_stream & stream;
  handshake_type ht;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code>> result_;
};

struct ssl_stream::accept_op_buf_     : detail::deferred_op_resource_base
{
  constexpr static bool await_ready() { return false; }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      stream.ssl_stream_.async_handshake(
          ht,
          io::buffers::const_buffer_span{&buf, 1u},
          completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
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

  accept_op_buf_(ssl_stream & stream,
                 handshake_type ht,
                 buffers::const_buffer buf) : stream(stream), ht(ht), buf(buf) {}
 private:
  ssl_stream & stream;
  handshake_type ht;
  buffers::const_buffer buf;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};

struct ssl_stream::accept_op_buf_seq_ : detail::deferred_op_resource_base
{
  constexpr static bool await_ready() { return false; }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      stream.ssl_stream_.async_handshake(ht, buf, completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
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

  accept_op_buf_seq_(ssl_stream & stream, handshake_type ht,
                     buffers::const_buffer_span buf) : stream(stream), ht(ht), buf(buf) {}
 private:
  ssl_stream & stream;
  handshake_type ht;
  buffers::const_buffer_span buf;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};

struct ssl_stream::shutdown_op_ : detail::deferred_op_resource_base
{
  constexpr static bool await_ready() { return false; }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      stream.ssl_stream_.template async_shutdown(completion_handler<system::error_code>{h, result_, get_resource(h)});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  [[nodiscard]] shutdown_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    auto ec = std::get<0>(result_.value_or(std::make_tuple(system::error_code{})));
    return ec ? shutdown_result(ec) : system::in_place_value;
  }

  shutdown_op_(ssl_stream & stream) : stream(stream) {}
 private:

  ssl_stream & stream;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code>> result_;
};

}

#endif //BOOST_ASYNC_SSL_HPP
