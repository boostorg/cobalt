//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_WRITE_AT_HPP
#define BOOST_ASYNC_IO_WRITE_AT_HPP

#include <boost/async/io/buffer.hpp>
#include "boost/async/io/detail/completion_condition.hpp"
#include <boost/async/io/concepts.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/system/error_code.hpp>

#include <concepts>

namespace boost::async::io
{

namespace detail
{

using write_at_handler = boost::async::completion_handler<system::error_code, std::size_t>;


void write_at_impl(concepts::random_access_write_device & pipe, std::uint64_t offset, const_buffer buffer, detail::write_at_handler rh);
void write_at_impl(concepts::random_access_write_device & pipe, std::uint64_t offset, const_buffer buffer, detail::completion_condition cond, detail::write_at_handler rh);
void write_at_impl(concepts::random_access_write_device & pipe, std::uint64_t offset, streambuf &buffer, detail::write_at_handler rh);
void write_at_impl(concepts::random_access_write_device & pipe, std::uint64_t offset, streambuf &buffer, detail::completion_condition cond, detail::write_at_handler rh);

template<typename Buffer>
struct [[nodiscard]] write_at_op
{
  concepts::random_access_write_device & stream;
  std::uint64_t offset;
  Buffer buffer;

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      write_at_impl(stream, offset, buffer, {h, result});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  std::size_t await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));

    if (std::get<0>(*result))
      throw system::system_error(std::get<0>(*result));

    return std::get<1>(*result);
  }
};


template<typename Buffer>
struct [[nodiscard]] write_at_ec_op
{
  concepts::random_access_write_device & stream;
  std::uint64_t offset;
  Buffer buffer;
  system::error_code & ec;
  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      write_at_impl(stream, offset, buffer, {h, result});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  std::size_t await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));

    ec = std::get<0>(*result);
    return std::get<1>(*result);
  }
};


template<typename Buffer, typename CompletionCondition>
struct [[nodiscard]] write_at_completion_op final : completion_condition_base
{
  concepts::random_access_write_device & stream;
  std::uint64_t offset;
  Buffer buffer;
  CompletionCondition completion_condition_;

  std::size_t operator()(const system::error_code & ec, std::size_t bytes_transferred) final
  {
    return completion_condition_(ec, bytes_transferred);
  }

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      write_at_impl(stream, offset, buffer, completion_condition{this}, {h, result});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  std::size_t await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));

    if (std::get<0>(*result))
      throw system::system_error(std::get<0>(*result));

    return std::get<1>(*result);
  }
};


template<typename Buffer, typename CompletionCondition>
struct [[nodiscard]] write_at_completion_ec_op final : completion_condition_base
{
  concepts::random_access_write_device & stream;
  std::uint64_t offset;
  Buffer buffer;
  CompletionCondition completion_condition_;

  std::size_t operator()(const system::error_code & ec, std::size_t bytes_transferred) final
  {
    return completion_condition_(ec, bytes_transferred);
  }

  system::error_code & ec;
  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      write_at_impl(stream, offset, buffer, completion_condition{this}, {h, result});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  std::size_t await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));

    ec = std::get<0>(*result);
    return std::get<1>(*result);
  }
};

}


inline auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, const_buffer                buffer) -> detail::write_at_op<const_buffer>                {return {stream, offset, buffer};}
inline auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, streambuf                  &buffer) -> detail::write_at_op<streambuf&>                  {return {stream, offset, buffer};}

inline auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, const_buffer                buffer, system::error_code & ec) -> detail::write_at_ec_op<const_buffer>                {return {stream, offset, buffer, ec};}
inline auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, streambuf                  &buffer, system::error_code & ec) -> detail::write_at_ec_op<streambuf&>                  {return {stream, offset, buffer, ec};}

template<typename CompletionCondition> auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, const_buffer                buffer, CompletionCondition completion_condition) -> detail::write_at_completion_op<const_buffer, CompletionCondition>                {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, streambuf                  &buffer, CompletionCondition completion_condition) -> detail::write_at_completion_op<streambuf&, CompletionCondition>                  {return {stream, offset, buffer, std::move(completion_condition)};}

template<typename CompletionCondition> auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, const_buffer                buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_at_completion_ec_op<const_buffer, CompletionCondition>                {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto write_at(concepts::random_access_write_device & stream, std::uint64_t offset, streambuf                  &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_at_completion_ec_op<streambuf&, CompletionCondition>                  {return {stream, offset, buffer, std::move(completion_condition), ec};}


}

#endif //BOOST_ASYNC_IO_WRITE_AT_HPP
