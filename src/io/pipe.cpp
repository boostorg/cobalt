//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/pipe.hpp>
#include <boost/async/io/detail/duplicate.hpp>
#include <boost/asio/append.hpp>
#include <boost/asio/connect_pipe.hpp>
#include <boost/asio/post.hpp>

namespace boost::async::io
{

readable_pipe::readable_pipe() : pipe_(this_thread::get_executor()) {}
readable_pipe::readable_pipe(native_handle_type native_handle)
  : pipe_(this_thread::get_executor(), native_handle) {}

system::result<void> readable_pipe::close()
{
  system::error_code ec;
  pipe_.close(ec);
  return ec ? ec : system::result<void>();
}

system::result<void> readable_pipe::cancel()
{
  system::error_code ec;
  pipe_.cancel(ec);
  return ec ? ec : system::result<void>();
}

bool readable_pipe::is_open() const { return pipe_.is_open(); }

void readable_pipe::async_read_some_impl_(
    buffers::mutable_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  pipe_.async_read_some(buffer, std::move(h));
}

void readable_pipe::async_write_some_impl_(
    buffers::const_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  constexpr static source_location loc{BOOST_CURRENT_LOCATION};
  system::error_code ec{asio::error::operation_not_supported, &loc};
  if (h.completed_immediately)
  {
    *h.completed_immediately = true;
    h(ec, 0ul);
  }
  else
    asio::post(h.executor, asio::append(std::move(h), ec, 0ul));
}


system::result<void> readable_pipe::assign(native_handle_type native_handle)
{
  system::error_code ec;
  pipe_.assign(native_handle, ec);
  return ec ? ec : system::result<void>();
}
auto readable_pipe::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto h = pipe_.release(ec);
  if (ec)
    return ec;
  else
    return h;
}

system::result<readable_pipe> readable_pipe::duplicate()
{
  auto res = detail::duplicate_handle(pipe_.native_handle());
  if (!res)
    return res.error();

  return readable_pipe(*res);
}


writable_pipe::writable_pipe() : pipe_(this_thread::get_executor()) {}
writable_pipe::writable_pipe(native_handle_type native_handle)
  : pipe_(this_thread::get_executor(), native_handle) {}

system::result<void> writable_pipe::close()
{
  system::error_code ec;
  pipe_.close(ec);
  return ec ? ec : system::result<void>();
}

system::result<void> writable_pipe::cancel()
{
  system::error_code ec;
  pipe_.cancel(ec);
  return ec ? ec : system::result<void>();
}

bool writable_pipe::is_open() const { return pipe_.is_open(); }

void writable_pipe::async_write_some_impl_(
    buffers::const_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  pipe_.async_write_some(buffer, std::move(h));
}


void writable_pipe::async_read_some_impl_(
    buffers::mutable_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  constexpr static source_location loc{BOOST_CURRENT_LOCATION};
  system::error_code ec{asio::error::operation_not_supported, &loc};
  if (h.completed_immediately)
  {
    *h.completed_immediately = true;
    h(ec, 0ul);
  }
  else
    asio::post(h.executor, asio::append(std::move(h), ec, 0ul));
}

system::result<void> writable_pipe::assign(native_handle_type native_handle)
{
  system::error_code ec;
  pipe_.assign(native_handle, ec);
  return ec ? ec : system::result<void>();
}
auto writable_pipe::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto h = pipe_.release(ec);
  if (ec)
    return ec;
  else
    return h;
}

system::result<writable_pipe> writable_pipe::duplicate()
{
  auto res = detail::duplicate_handle(pipe_.native_handle());
  if (!res)
    return res.error();

  return writable_pipe(*res);
}

system::result<std::pair<readable_pipe, writable_pipe>> make_pipe()
{
  readable_pipe rp{};
  writable_pipe wp{};

  system::error_code ec;
  asio::connect_pipe(rp.pipe_, wp.pipe_, ec);
  if (ec)
    return ec;
  else
    return std::make_pair(std::move(rp), std::move(wp));
}


}