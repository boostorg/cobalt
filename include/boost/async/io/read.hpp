//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_READ_HPP
#define BOOST_ASYNC_IO_READ_HPP

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

using read_handler = boost::async::completion_handler<system::error_code, std::size_t>;

void read_impl(concepts::read_stream & stream, mutable_buffer buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, mutable_buffer buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, flat_static_buffer_base &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, static_buffer_base &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, static_buffer_base &buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, flat_buffer &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, flat_buffer &buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, multi_buffer &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, multi_buffer &buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, std::string &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, std::string &buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, std::vector<unsigned char> &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, streambuf &buffer, detail::read_handler rh);
void read_impl(concepts::read_stream & stream, streambuf &buffer, detail::completion_condition cond, detail::read_handler rh);

template<typename Buffer>
struct [[nodiscard]] read_op
{
  concepts::read_stream & stream;
  Buffer buffer;

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      read_impl(stream, buffer, {h, result});
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
struct [[nodiscard]] read_ec_op
{
  concepts::read_stream & stream;
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
      read_impl(stream, buffer, {h, result});
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
struct [[nodiscard]] read_completion_op final : completion_condition_base
{
  concepts::read_stream & stream;
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
      read_impl(stream, buffer, completion_condition{this}, {h, result});
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
struct [[nodiscard]] read_completion_ec_op final : completion_condition_base
{
  concepts::read_stream & stream;
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
      read_impl(stream, buffer, completion_condition{this}, {h, result});
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


inline auto read(concepts::read_stream & stream, mutable_buffer              buffer) -> detail::read_op<mutable_buffer>              {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, flat_static_buffer_base    &buffer) -> detail::read_op<flat_static_buffer_base &>   {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, static_buffer_base         &buffer) -> detail::read_op<static_buffer_base &>        {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, flat_buffer                &buffer) -> detail::read_op<flat_buffer &>               {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, multi_buffer               &buffer) -> detail::read_op<multi_buffer &>              {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, std::string                &buffer) -> detail::read_op<std::string&>                {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, std::vector<unsigned char> &buffer) -> detail::read_op<std::vector<unsigned char>&> {return {stream, buffer};}
inline auto read(concepts::read_stream & stream, streambuf                  &buffer) -> detail::read_op<streambuf&>                  {return {stream, buffer};}

inline auto read(concepts::read_stream & stream, mutable_buffer              buffer, system::error_code & ec) -> detail::read_ec_op<mutable_buffer>              {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, flat_static_buffer_base    &buffer, system::error_code & ec) -> detail::read_ec_op<flat_static_buffer_base &>   {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, static_buffer_base         &buffer, system::error_code & ec) -> detail::read_ec_op<static_buffer_base &>        {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, flat_buffer                &buffer, system::error_code & ec) -> detail::read_ec_op<flat_buffer &>               {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, multi_buffer               &buffer, system::error_code & ec) -> detail::read_ec_op<multi_buffer &>              {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, std::string                &buffer, system::error_code & ec) -> detail::read_ec_op<std::string&>                {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, std::vector<unsigned char> &buffer, system::error_code & ec) -> detail::read_ec_op<std::vector<unsigned char>&> {return {stream, buffer, ec};}
inline auto read(concepts::read_stream & stream, streambuf                  &buffer, system::error_code & ec) -> detail::read_ec_op<streambuf&>                  {return {stream, buffer, ec};}

template<typename CompletionCondition> auto read(concepts::read_stream & stream, mutable_buffer              buffer, CompletionCondition completion_condition) -> detail::read_completion_op<mutable_buffer, CompletionCondition>              {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, flat_static_buffer_base    &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<flat_static_buffer_base &, CompletionCondition>   {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, static_buffer_base         &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<static_buffer_base &, CompletionCondition>        {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, flat_buffer                &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<flat_buffer &, CompletionCondition>               {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, multi_buffer               &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<multi_buffer &, CompletionCondition>              {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, std::string                &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<std::string&, CompletionCondition>                {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, std::vector<unsigned char> &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<std::vector<unsigned char>&, CompletionCondition> {return {stream, buffer, std::move(completion_condition)};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, streambuf                  &buffer, CompletionCondition completion_condition) -> detail::read_completion_op<streambuf&, CompletionCondition>                  {return {stream, buffer, std::move(completion_condition)};}

template<typename CompletionCondition> auto read(concepts::read_stream & stream, mutable_buffer              buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<mutable_buffer, CompletionCondition>              {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, flat_static_buffer_base    &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<flat_static_buffer_base &, CompletionCondition>   {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, static_buffer_base         &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<static_buffer_base &, CompletionCondition>        {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, flat_buffer                &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<flat_buffer &, CompletionCondition>               {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, multi_buffer               &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<multi_buffer &, CompletionCondition>              {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, std::string                &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<std::string&, CompletionCondition>                {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, std::vector<unsigned char> &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<std::vector<unsigned char>&, CompletionCondition> {return {stream, buffer, std::move(completion_condition), ec};}
template<typename CompletionCondition> auto read(concepts::read_stream & stream, streambuf                  &buffer, CompletionCondition completion_condition, system::error_code & ec) -> detail::read_completion_ec_op<streambuf&, CompletionCondition>                  {return {stream, buffer, std::move(completion_condition), ec};}


}

#endif //BOOST_ASYNC_IO_READ_HPP
