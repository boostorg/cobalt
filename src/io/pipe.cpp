//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/pipe.hpp>
#include <boost/asio/connect_pipe.hpp>

namespace boost::cobalt::io
{

readable_pipe::readable_pipe(const cobalt::executor & executor)
    : implementation_(executor)
{
}

readable_pipe::readable_pipe(native_handle_type native_file, const cobalt::executor & executor)
    : implementation_(executor, native_file)
{
}

readable_pipe::readable_pipe(readable_pipe && sf) noexcept = default;

system::result<void> readable_pipe::assign(native_handle_type native_file)
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


system::result<void>           readable_pipe::close()
{
  system::error_code ec;
  implementation_.close(ec);
  return ec ? ec : system::result<void>{};
}

void readable_pipe::initiate_read_some_(void * this_, mutable_buffer_sequence buffer,
                                        boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buf)
        {
          static_cast<readable_pipe*>(this_)->implementation_.async_read_some(buffer, std::move(handler));
        });
}



writable_pipe::writable_pipe(const cobalt::executor & executor)
    : implementation_(executor)
{
}

writable_pipe::writable_pipe(native_handle_type native_file, const cobalt::executor & executor)
    : implementation_(executor, native_file)
{
}

writable_pipe::writable_pipe(writable_pipe && sf) noexcept = default;

system::result<void> writable_pipe::assign(native_handle_type native_file)
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

system::result<void> writable_pipe::close()
{
  system::error_code ec;
  implementation_.close(ec);
  return ec ? ec : system::result<void>{};
}


void writable_pipe::initiate_write_some_(void * this_, const_buffer_sequence buffer,
                                         boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buf)
        {
          static_cast<writable_pipe*>(this_)->implementation_.async_write_some(buffer, std::move(handler));
        });
}


system::result<std::pair<readable_pipe, writable_pipe>> pipe(const cobalt::executor & executor)
{
  readable_pipe r{executor};
  writable_pipe w{executor};
  system::error_code ec;
  asio::connect_pipe(r.implementation_, w.implementation_);
  if (ec)
    return ec;
  else
    return std::make_pair(std::move(r), std::move(w));
}

}
