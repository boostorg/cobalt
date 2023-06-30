//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_RANDOM_ACCESS_DEVICE_HPP
#define BOOST_ASYNC_IO_DETAIL_RANDOM_ACCESS_DEVICE_HPP

#include <boost/async/io/random_access_device.hpp>

namespace boost::async::io
{

struct random_access_device::read_some_at_op_ : detail::deferred_op_resource_base
{
  read_some_at_op_(read_some_at_op_ && ) noexcept = default;

  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      random_access_device_.async_read_some_at_impl_(offset_, {&buffer_, 1u},
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

  read_some_at_op_(random_access_device & rs, std::uint64_t offset, buffers::mutable_buffer buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::mutable_buffer buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct random_access_device::read_some_at_op_seq_ : detail::deferred_op_resource_base
{
  constexpr bool await_ready() noexcept {return false;};


  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      random_access_device_.async_read_some_at_impl_(offset_, buffer_, completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
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

  read_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::mutable_buffer_subspan buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}
  read_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::mutable_buffer_span buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::mutable_buffer_subspan buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};
struct random_access_device::write_some_at_op_ : detail::deferred_op_resource_base
{
  write_some_at_op_(write_some_at_op_ && ) noexcept = default;

  constexpr bool await_ready() noexcept {return false;};

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      random_access_device_.async_write_some_at_impl_(offset_, {&buffer_, 1},
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

  write_some_at_op_(random_access_device & rs, std::uint64_t offset, buffers::const_buffer buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::const_buffer buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};


struct random_access_device::write_some_at_op_seq_ : detail::deferred_op_resource_base
{
  constexpr bool await_writey() noexcept {return false;};


  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      random_access_device_.async_write_some_at_impl_(offset_, buffer_, completion_handler<system::error_code, std::size_t>{h, result_, get_resource(h)});
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

  write_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::const_buffer_subspan buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}
  write_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::const_buffer_span buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::const_buffer_subspan buffer_;
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result_;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_RANDOM_ACCESS_DEVICE_HPP
