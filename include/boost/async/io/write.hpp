//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_WRITE_HPP
#define BOOST_ASYNC_IO_WRITE_HPP

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

using write_handler = boost::async::detail::completion_handler<system::error_code, std::size_t>;

template<typename WritableStream>
struct write_impl
{
  static void call(WritableStream & pipe, const_buffer buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, const_buffer buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, flat_static_buffer_base &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, static_buffer_base &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, static_buffer_base &buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, flat_buffer &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, flat_buffer &buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, multi_buffer &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, multi_buffer &buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, std::string &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, std::string &buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, std::vector<unsigned char> &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::write_handler rh);
  static void call(WritableStream & pipe, streambuf &buffer, detail::write_handler rh);
  static void call(WritableStream & pipe, streambuf &buffer, detail::completion_condition cond, detail::write_handler rh);
};

template<typename WritableStream, typename Buffer>
struct write_op
{
  WritableStream & stream;
  Buffer buffer;

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      write_impl<WritableStream>::call(stream, buffer, {h, result});
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


template<typename WritableStream, typename Buffer>
struct write_ec_op
{
  WritableStream & stream;
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
      write_impl<WritableStream>::call(stream, buffer, {h, result});
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


template<typename WritableStream, typename Buffer, typename CompletionCondition>
struct write_completion_op final : completion_condition_base
{
  WritableStream & stream;
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
      write_impl<WritableStream>::call(stream, buffer, completion_condition{this}, {h, result});
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


template<typename WritableStream, typename Buffer, typename CompletionCondition>
struct write_completion_ec_op final : completion_condition_base
{
  WritableStream & stream;
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
      write_impl<WritableStream>::call(stream, buffer, completion_condition{this}, {h, result});
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


template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, const_buffer                buffer) -> detail::write_op<Stream, const_buffer>                {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, flat_static_buffer_base    &buffer) -> detail::write_op<Stream, flat_static_buffer_base &>   {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, static_buffer_base         &buffer) -> detail::write_op<Stream, static_buffer_base &>        {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, flat_buffer                &buffer) -> detail::write_op<Stream, flat_buffer &>               {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, multi_buffer               &buffer) -> detail::write_op<Stream, multi_buffer &>              {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, std::string                &buffer) -> detail::write_op<Stream, std::string&>                {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, std::vector<unsigned char> &buffer) -> detail::write_op<Stream, std::vector<unsigned char>&> {return {stream, buffer};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, streambuf                  &buffer) -> detail::write_op<Stream, streambuf&>                  {return {stream, buffer};}

template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, const_buffer                buffer, system::error_code & ec) -> detail::write_ec_op<Stream, const_buffer>                {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, flat_static_buffer_base    &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, flat_static_buffer_base &>   {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, static_buffer_base         &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, static_buffer_base &>        {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, flat_buffer                &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, flat_buffer &>               {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, multi_buffer               &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, multi_buffer &>              {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, std::string                &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, std::string&>                {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, std::vector<unsigned char> &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, std::vector<unsigned char>&> {return {stream, buffer, ec};}
template<std::derived_from<concepts::write_stream> Stream> auto write(Stream & stream, streambuf                  &buffer, system::error_code & ec) -> detail::write_ec_op<Stream, streambuf&>                  {return {stream, buffer, ec};}

template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, const_buffer                buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, const_buffer, CompletionCondition>                {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, flat_static_buffer_base    &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, flat_static_buffer_base &, CompletionCondition>   {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, static_buffer_base         &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, static_buffer_base &, CompletionCondition>        {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, flat_buffer                &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, flat_buffer &, CompletionCondition>               {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, multi_buffer               &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, multi_buffer &, CompletionCondition>              {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, std::string                &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, std::string&, CompletionCondition>                {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, std::vector<unsigned char> &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, std::vector<unsigned char>&, CompletionCondition> {return {stream, buffer, std::move(completion_condition)};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, streambuf                  &buffer, CompletionCondition completion_condition) -> detail::write_completion_op<Stream, streambuf&, CompletionCondition>                  {return {stream, buffer, std::move(completion_condition)};}

template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, const_buffer                buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, const_buffer, CompletionCondition>                {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, flat_static_buffer_base    &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, flat_static_buffer_base &, CompletionCondition>   {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, static_buffer_base         &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, static_buffer_base &, CompletionCondition>        {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, flat_buffer                &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, flat_buffer &, CompletionCondition>               {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, multi_buffer               &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, multi_buffer &, CompletionCondition>              {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, std::string                &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, std::string&, CompletionCondition>                {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, std::vector<unsigned char> &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, std::vector<unsigned char>&, CompletionCondition> {return {stream, buffer, std::move(completion_condition), ec};}
template<std::derived_from<concepts::write_stream> Stream, typename CompletionCondition> auto write(Stream & stream, streambuf                  &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::write_completion_ec_op<Stream, streambuf&, CompletionCondition>                  {return {stream, buffer, std::move(completion_condition), ec};}


}

#endif //BOOST_ASYNC_IO_WRITE_HPP
