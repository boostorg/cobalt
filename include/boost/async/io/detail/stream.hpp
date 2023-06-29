//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_STREAM_HPP
#define BOOST_ASYNC_IO_DETAIL_STREAM_HPP

#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct stream::read_some_op_
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(
          buffer, sizeof(buffer),
          asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      rstream_.async_read_some_impl_({&buffer_, 1u},
                                     completion_handler<system::error_code, std::size_t>{h, result_, &res});
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

  read_some_op_(stream & rs, buffers::mutable_buffer buffer)
      : rstream_(rs), buffer_(buffer) {}

 private:
  stream & rstream_;
  buffers::mutable_buffer buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
};


struct stream::read_some_op_seq_
{
  constexpr bool await_ready() noexcept {return false;};


  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(
          buffer, sizeof(buffer),
          asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      rstream_.async_read_some_impl_(buffer_, completion_handler<system::error_code, std::size_t>{h, result_, &res});
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

  read_some_op_seq_(stream & rs, buffers::mutable_buffer_span buffer)
      : rstream_(rs), buffer_(buffer) {}

 private:
  stream & rstream_;
  buffers::mutable_buffer_span buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
};
struct stream::write_some_op_
{
  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(
          buffer, sizeof(buffer),
          asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      rstream_.async_write_some_impl_({&buffer_, 1},
                                      completion_handler<system::error_code, std::size_t>{h, result_, &res});
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

  write_some_op_(stream & rs, buffers::const_buffer buffer)
      : rstream_(rs), buffer_(buffer) {}

 private:
  stream & rstream_;
  buffers::const_buffer buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
};


struct stream::write_some_op_seq_
{
  constexpr bool await_writey() noexcept {return false;};


  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(
          buffer, sizeof(buffer),
          asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      rstream_.async_write_some_impl_(buffer_, completion_handler<system::error_code, std::size_t>{h, result_, &res});
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

  write_some_op_seq_(stream & rs, buffers::const_buffer_span buffer)
      : rstream_(rs), buffer_(buffer) {}

 private:
  stream & rstream_;
  buffers::const_buffer_span buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_STREAM_HPP
