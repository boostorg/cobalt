//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_READ_UNTIL_HPP
#define BOOST_ASYNC_IO_READ_UNTIL_HPP

#include <boost/async/io/buffer.hpp>
#include <boost/async/io/detail/match_condition.hpp>
#include <boost/async/io/concepts.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/system/error_code.hpp>

#include <boost/regex_fwd.hpp>
#include <concepts>

namespace boost::async::io
{

namespace detail
{

using read_handler = boost::async::detail::completion_handler<system::error_code, std::size_t>;

template<typename ReadableStream>
struct read_until_impl
{
  static void call(ReadableStream & pipe, flat_static_buffer_base &buffer,    match_condition<flat_static_buffer_base> cond, read_handler rh);
  static void call(ReadableStream & pipe, flat_static_buffer_base &buffer,    char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, flat_static_buffer_base &buffer,    asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, flat_static_buffer_base &buffer,    const boost::regex & expr,                     read_handler rh);
  static void call(ReadableStream & pipe, static_buffer_base &buffer,         match_condition<static_buffer_base> cond,      read_handler rh);
  static void call(ReadableStream & pipe, static_buffer_base &buffer,         char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, static_buffer_base &buffer,         asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, static_buffer_base &buffer,         const boost::regex & expr,                     read_handler rh);
  static void call(ReadableStream & pipe, flat_buffer &buffer,                match_condition<flat_buffer> cond,             read_handler rh);
  static void call(ReadableStream & pipe, flat_buffer &buffer,                char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, flat_buffer &buffer,                asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, flat_buffer &buffer,                const boost::regex & expr,                     read_handler rh);
  static void call(ReadableStream & pipe, multi_buffer &buffer,               match_condition<multi_buffer> cond,            read_handler rh);
  static void call(ReadableStream & pipe, multi_buffer &buffer,               char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, multi_buffer &buffer,               asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, multi_buffer &buffer,               const boost::regex & expr,                     read_handler rh);
  static void call(ReadableStream & pipe, std::string &buffer,
                   match_condition<asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>> cond,    read_handler rh);
  static void call(ReadableStream & pipe, std::string &buffer,                char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, std::string &buffer,                asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, std::string &buffer,                const boost::regex & expr,                     read_handler rh);
  static void call(ReadableStream & pipe, std::vector<unsigned char> &buffer,
                   match_condition<asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>> cond,          read_handler rh);
  static void call(ReadableStream & pipe, std::vector<unsigned char> &buffer, char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, std::vector<unsigned char> &buffer, asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, std::vector<unsigned char> &buffer, const boost::regex & expr,                     read_handler rh);
  static void call(ReadableStream & pipe, streambuf &buffer,                  match_condition<streambuf>   cond,             read_handler rh);
  static void call(ReadableStream & pipe, streambuf &buffer,                  char delim,                                    read_handler rh);
  static void call(ReadableStream & pipe, streambuf &buffer,                  asio::string_view delim,                       read_handler rh);
  static void call(ReadableStream & pipe, streambuf &buffer,                  const boost::regex & expr,                     read_handler rh);
};

template<typename ReadableStream, typename Buffer>
struct read_until_op
{
  ReadableStream & stream;
  Buffer buffer;

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      read_until_impl<ReadableStream>::call(stream, buffer, {h, result});
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


template<typename ReadableStream, typename Buffer>
struct read_until_ec_op
{
  ReadableStream & stream;
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
      read_until_impl<ReadableStream>::call(stream, buffer, {h, result});
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




template<typename ReadableStream, typename Buffer, typename Matcher>
struct read_until_matcher_op final
{
  ReadableStream & stream;
  Buffer buffer;
  Matcher matcher;

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      read_until_impl<ReadableStream>::call(stream, buffer, matcher, {h, result});
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


template<typename ReadableStream, typename Buffer, typename Matcher>
struct read_until_matcher_ec_op final
{
  ReadableStream & stream;
  Buffer buffer;
  Matcher matcher;

  system::error_code & ec;
  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      read_until_impl<ReadableStream>::call(stream, buffer, matcher, {h, result});
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


template<typename ReadableStream, typename Buffer, typename MatchCondition>
struct read_until_match_condition_op final : match_condition_base<std::decay_t<Buffer>>
{
  ReadableStream & stream;
  Buffer buffer;
  MatchCondition match_condition_;

  using iterator = typename match_condition_base<std::decay_t<Buffer>>::iterator;
  std::pair<iterator, bool> operator()(iterator begin, iterator end) final
  {
    return match_condition_(begin, end);
  }

  std::optional<std::tuple<system::error_code, std::size_t>> result;
  std::exception_ptr error;

  constexpr static bool await_ready() {return false;}

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      read_until_impl<ReadableStream>::call(stream, buffer, match_condition{this}, {h, result});
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


template<typename ReadableStream, typename Buffer, typename MatchCondition>
struct read_until_match_condition_ec_op final : match_condition_base<std::decay_t<Buffer>>
{
  ReadableStream & stream;
  Buffer buffer;
  MatchCondition match_condition_;

  using iterator = typename match_condition_base<std::decay_t<Buffer>>::iterator;
  std::pair<iterator, bool> operator()(iterator begin, iterator end) final
  {
    return match_condition_(begin, end);
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
      read_until_impl<ReadableStream>::call(stream, buffer, match_condition{this}, {h, result});
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


template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer) -> detail::read_until_op<Stream, mutable_buffer>              {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer) -> detail::read_until_op<Stream, flat_static_buffer_base &>   {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer) -> detail::read_until_op<Stream, static_buffer_base &>        {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer) -> detail::read_until_op<Stream, flat_buffer &>               {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer) -> detail::read_until_op<Stream, multi_buffer &>              {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer) -> detail::read_until_op<Stream, std::string&>                {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer) -> detail::read_until_op<Stream, std::vector<unsigned char>&> {return {stream, buffer};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer) -> detail::read_until_op<Stream, streambuf&>                  {return {stream, buffer};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, mutable_buffer>              {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, flat_static_buffer_base &>   {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, static_buffer_base &>        {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, flat_buffer &>               {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, multi_buffer &>              {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, std::string&>                {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, std::vector<unsigned char>&> {return {stream, buffer, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, system::error_code & ec) -> detail::read_until_ec_op<Stream, streambuf&>                  {return {stream, buffer, ec};}

template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, mutable_buffer              buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, mutable_buffer, MatchCondition>              {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, flat_static_buffer_base &, MatchCondition>   {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, static_buffer_base         &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, static_buffer_base &, MatchCondition>        {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, flat_buffer                &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, flat_buffer &, MatchCondition>               {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, multi_buffer               &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, multi_buffer &, MatchCondition>              {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, std::string                &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, std::string&, MatchCondition>                {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, std::vector<unsigned char>&, MatchCondition> {return {stream, buffer, std::move(match_condition)};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, streambuf                  &buffer, MatchCondition match_condition) -> detail::read_until_match_condition_op<Stream, streambuf&, MatchCondition>                  {return {stream, buffer, std::move(match_condition)};}

template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, mutable_buffer              buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, mutable_buffer, MatchCondition>              {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, flat_static_buffer_base &, MatchCondition>   {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, static_buffer_base         &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, static_buffer_base &, MatchCondition>        {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, flat_buffer                &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, flat_buffer &, MatchCondition>               {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, multi_buffer               &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, multi_buffer &, MatchCondition>              {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, std::string                &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, std::string&, MatchCondition>                {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, std::vector<unsigned char>&, MatchCondition> {return {stream, buffer, std::move(match_condition), ec};}
template<std::derived_from<concepts::read_stream> Stream, typename MatchCondition> auto read_until(Stream & stream, streambuf                  &buffer, MatchCondition match_condition, system::error_code & ec) -> detail::read_until_match_condition_ec_op<Stream, streambuf&, MatchCondition>                  {return {stream, buffer, std::move(match_condition), ec};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, char delim) -> detail::read_until_matcher_op<Stream, mutable_buffer, char>              {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, char delim) -> detail::read_until_matcher_op<Stream, flat_static_buffer_base &, char>   {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, char delim) -> detail::read_until_matcher_op<Stream, static_buffer_base &, char>        {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, char delim) -> detail::read_until_matcher_op<Stream, flat_buffer &, char>               {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, char delim) -> detail::read_until_matcher_op<Stream, multi_buffer &, char>              {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, char delim) -> detail::read_until_matcher_op<Stream, std::string&, char>                {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, char delim) -> detail::read_until_matcher_op<Stream, std::vector<unsigned char>&, char> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, char delim) -> detail::read_until_matcher_op<Stream, streambuf&, char>                  {return {stream, buffer, delim};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, mutable_buffer, char>              {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, flat_static_buffer_base &, char>   {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, static_buffer_base &, char>        {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, flat_buffer &, char>               {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, multi_buffer &, char>              {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, std::string&, char>                {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, std::vector<unsigned char>&, char> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, char delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, streambuf&, char>                  {return {stream, buffer, delim, ec};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, mutable_buffer,              asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, flat_static_buffer_base &,   asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, static_buffer_base &,        asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, flat_buffer &,               asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, multi_buffer &,              asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, std::string&,                asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, std::vector<unsigned char>&, asio::string_view> {return {stream, buffer, delim};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, asio::string_view delim) -> detail::read_until_matcher_op<Stream, streambuf&,                  asio::string_view> {return {stream, buffer, delim};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, mutable_buffer,               asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, flat_static_buffer_base &,    asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, static_buffer_base &,         asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, flat_buffer &,                asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, multi_buffer &,               asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, std::string&,                 asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, std::vector<unsigned char>&,  asio::string_view> {return {stream, buffer, delim, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, asio::string_view  delim, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, streambuf&,                   asio::string_view> {return {stream, buffer, delim, ec};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, mutable_buffer,              const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, flat_static_buffer_base &,   const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, static_buffer_base &,        const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, flat_buffer &,               const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, multi_buffer &,              const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, std::string&,                const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, std::vector<unsigned char>&, const boost::regex &> {return {stream, buffer, expr};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, const boost::regex &expr) -> detail::read_until_matcher_op<Stream, streambuf&,                  const boost::regex &> {return {stream, buffer, expr};}

template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, mutable_buffer              buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, mutable_buffer,               const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_static_buffer_base    &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, flat_static_buffer_base &,    const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, static_buffer_base         &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, static_buffer_base &,         const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, flat_buffer                &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, flat_buffer &,                const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, multi_buffer               &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, multi_buffer &,               const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::string                &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, std::string&,                 const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, std::vector<unsigned char> &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, std::vector<unsigned char>&,  const boost::regex &> {return {stream, buffer, expr, ec};}
template<std::derived_from<concepts::read_stream> Stream> auto read_until(Stream & stream, streambuf                  &buffer, const boost::regex &expr, system::error_code & ec) -> detail::read_until_matcher_ec_op<Stream, streambuf&,                   const boost::regex &> {return {stream, buffer, expr, ec};}

}

#endif //BOOST_ASYNC_IO_READ_UNTIL_HPP
