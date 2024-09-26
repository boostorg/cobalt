//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/pipe.hpp>
#include <boost/cobalt/experimental/io/stream.hpp>
#include <boost/asio/connect_pipe.hpp>

namespace boost::cobalt::experimental::io
{

static_assert(read_stream<readable_pipe>);

readable_pipe::readable_pipe()
    : implementation_(this_thread::get_executor())
{
}

readable_pipe::readable_pipe(const native_handle_type & native_file)
    : implementation_(this_thread::get_executor(), native_file)
{
}

readable_pipe::readable_pipe(readable_pipe && sf) noexcept = default;

system::result<void> readable_pipe::assign(const native_handle_type & native_file)
{
  system::error_code ec;
  implementation_.assign(native_file, ec);
  return ec ? ec : system::result<void>{};
}
system::result<void> readable_pipe::cancel()
{
  system::error_code ec;
  implementation_.cancel(ec);
  return ec ? ec : system::result<void>{};
}

executor readable_pipe::get_executor() { return implementation_.get_executor();}
bool readable_pipe::is_open() const { return implementation_.is_open();}

auto readable_pipe::native_handle() -> native_handle_type {return implementation_.native_handle();}

auto readable_pipe::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto r = implementation_.release(ec);
  return ec ? ec : system::result<native_handle_type>{r};
}


system::result<void>           readable_pipe::close_op_::await_resume(as_result_tag)
{
  system::error_code ec;
  f_.close(ec);
  return ec ? ec : system::result<void>{};
}
std::tuple<system::error_code> readable_pipe::close_op_::await_resume(as_tuple_tag)
{
  system::error_code ec;
  f_.close(ec);
  return std::make_tuple(ec);
}

void                           readable_pipe::close_op_::await_resume()
{
  f_.close();
}

void readable_pipe::read_some_op_::initiate(cobalt::completion_handler<system::error_code, std::size_t> complete)
{
  variant2::visit(
      [&](auto buf)
      {
        implementation.async_read_some(std::move(buf), std::move(complete));
      }, buffer);
}



static_assert(write_stream<writable_pipe>);


writable_pipe::writable_pipe()
    : implementation_(this_thread::get_executor())
{
}

writable_pipe::writable_pipe(const native_handle_type & native_file)
    : implementation_(this_thread::get_executor(), native_file)
{
}

writable_pipe::writable_pipe(writable_pipe && sf) noexcept = default;

system::result<void> writable_pipe::assign(const native_handle_type & native_file)
{
  system::error_code ec;
  implementation_.assign(native_file, ec);
  return ec ? ec : system::result<void>{};
}
system::result<void> writable_pipe::cancel()
{
  system::error_code ec;
  implementation_.cancel(ec);
  return ec ? ec : system::result<void>{};
}

executor writable_pipe::get_executor() { return implementation_.get_executor();}
bool writable_pipe::is_open() const { return implementation_.is_open();}

auto writable_pipe::native_handle() -> native_handle_type {return implementation_.native_handle();}

auto writable_pipe::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto r = implementation_.release(ec);
  return ec ? ec : system::result<native_handle_type>{r};
}



system::result<void>           writable_pipe::close_op_::await_resume(as_result_tag)
{
  system::error_code ec;
  f_.close(ec);
  return ec ? ec : system::result<void>{};
}
std::tuple<system::error_code> writable_pipe::close_op_::await_resume(as_tuple_tag)
{
  system::error_code ec;
  f_.close(ec);
  return std::make_tuple(ec);
}

void                           writable_pipe::close_op_::await_resume()
{
  f_.close();
}

void writable_pipe::write_some_op_::initiate(cobalt::completion_handler<system::error_code, std::size_t> complete)
{
  variant2::visit(
      [&](auto buf)
      {
        implementation.async_write_some(std::move(buf), std::move(complete));
      }, buffer);
}


system::result<std::pair<readable_pipe, writable_pipe>> pipe()
{
  readable_pipe r;
  writable_pipe w;
  system::error_code ec;
  asio::connect_pipe(r.implementation_, w.implementation_);
  if (ec)
    return ec;
  else
    return std::make_pair(std::move(r), std::move(w));
}

}
