//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_READ_AT_HPP
#define BOOST_ASYNC_IO_READ_AT_HPP

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

using read_at_handler = boost::async::detail::completion_handler<system::error_code, std::size_t>;

void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, mutable_buffer buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, mutable_buffer buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, flat_static_buffer_base &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, static_buffer_base &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, static_buffer_base &buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, flat_buffer &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, flat_buffer &buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, multi_buffer &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, multi_buffer &buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, std::string &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, std::string &buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, std::vector<unsigned char> &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, streambuf &buffer, detail::read_at_handler rh);
void read_at_impl(concepts::read_stream & pipe, std::uint64_t offset, streambuf &buffer, detail::completion_condition cond, detail::read_at_handler rh);


template<typename Buffer>
struct read_at_op
{
  concepts::read_stream & stream;
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
      read_at_impl(stream, offset, buffer, {h, result});
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
struct read_at_ec_op
{
  concepts::read_stream & stream;
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
      read_at_impl(stream, offset, buffer, {h, result});
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
struct read_at_completion_op final : completion_condition_base
{
  concepts::read_stream & stream;
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
      read_at_impl(stream, offset, buffer, completion_condition{this}, {h, result});
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
struct read_at_completion_ec_op final : completion_condition_base
{
  concepts::read_stream & stream;
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
      read_at_impl(stream, offset, buffer, completion_condition{this}, {h, result});
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


inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, mutable_buffer              buffer) -> detail::read_at_op<mutable_buffer>              {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_static_buffer_base    &buffer) -> detail::read_at_op<flat_static_buffer_base &>   {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, static_buffer_base         &buffer) -> detail::read_at_op<static_buffer_base &>        {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_buffer                &buffer) -> detail::read_at_op<flat_buffer &>               {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, multi_buffer               &buffer) -> detail::read_at_op<multi_buffer &>              {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::string                &buffer) -> detail::read_at_op<std::string&>                {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::vector<unsigned char> &buffer) -> detail::read_at_op<std::vector<unsigned char>&> {return {stream, offset, buffer};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, streambuf                  &buffer) -> detail::read_at_op<streambuf&>                  {return {stream, offset, buffer};}

inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, mutable_buffer              buffer, system::error_code & ec) -> detail::read_at_ec_op<mutable_buffer>              {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_static_buffer_base    &buffer, system::error_code & ec) -> detail::read_at_ec_op<flat_static_buffer_base &>   {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, static_buffer_base         &buffer, system::error_code & ec) -> detail::read_at_ec_op<static_buffer_base &>        {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_buffer                &buffer, system::error_code & ec) -> detail::read_at_ec_op<flat_buffer &>               {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, multi_buffer               &buffer, system::error_code & ec) -> detail::read_at_ec_op<multi_buffer &>              {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::string                &buffer, system::error_code & ec) -> detail::read_at_ec_op<std::string&>                {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::vector<unsigned char> &buffer, system::error_code & ec) -> detail::read_at_ec_op<std::vector<unsigned char>&> {return {stream, offset, buffer, ec};}
inline auto read_at(concepts::read_stream & stream, std::uint64_t offset, streambuf                  &buffer, system::error_code & ec) -> detail::read_at_ec_op<streambuf&>                  {return {stream, offset, buffer, ec};}

template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, mutable_buffer              buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<mutable_buffer, CompletionCondition>              {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_static_buffer_base    &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<flat_static_buffer_base &, CompletionCondition>   {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, static_buffer_base         &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<static_buffer_base &, CompletionCondition>        {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_buffer                &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<flat_buffer &, CompletionCondition>               {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, multi_buffer               &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<multi_buffer &, CompletionCondition>              {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::string                &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<std::string&, CompletionCondition>                {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::vector<unsigned char> &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<std::vector<unsigned char>&, CompletionCondition> {return {stream, offset, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, streambuf                  &buffer, CompletionCondition completion_condition) -> detail::read_at_completion_op<streambuf&, CompletionCondition>                  {return {stream, offset, buffer, std::move(completion_condition)};}

template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, mutable_buffer              buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<mutable_buffer, CompletionCondition>              {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_static_buffer_base    &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<flat_static_buffer_base &, CompletionCondition>   {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, static_buffer_base         &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<static_buffer_base &, CompletionCondition>        {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, flat_buffer                &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<flat_buffer &, CompletionCondition>               {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, multi_buffer               &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<multi_buffer &, CompletionCondition>              {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::string                &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<std::string&, CompletionCondition>                {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, std::vector<unsigned char> &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<std::vector<unsigned char>&, CompletionCondition> {return {stream, offset, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read_at(concepts::read_stream & stream, std::uint64_t offset, streambuf                  &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_at_completion_ec_op<streambuf&, CompletionCondition>                  {return {stream, offset, buffer, std::move(completion_condition), ec};}


}

#endif //BOOST_ASYNC_IO_READ_AT_HPP
