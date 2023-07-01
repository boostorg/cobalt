//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/popen.hpp>

namespace boost::async::io
{

popen::popen(boost::process::v2::filesystem::path executable,
                 std::initializer_list<core::string_view> args,
                 process_initializer initializer)
    : popen_(this_thread::get_executor(),
               executable,
               args,
               initializer.stdio,
               initializer.start_dir,
               initializer.env) {}


popen::popen(boost::process::v2::filesystem::path executable,
                 std::span<core::string_view> args,
                 process_initializer initializer)
    : popen_(this_thread::get_executor(),
               executable,
               args,
               initializer.stdio,
               initializer.start_dir,
               initializer.env) {}

pid_type popen::id() const {return popen_.id();}

system::result<void> popen::interrupt()
{
  system::error_code ec;
  popen_.interrupt(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> popen::request_exit()
{
  system::error_code ec;
  popen_.request_exit(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> popen::suspend()
{
  system::error_code ec;
  popen_.suspend(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> popen::resume()
{
  system::error_code ec;
  popen_.resume(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> popen::terminate()
{
  system::error_code ec;
  popen_.terminate(ec);
  return ec ? ec : system::result<void>();
}
popen::handle_type popen::detach()
{
  return popen_.detach();
}
system::result<bool> popen::running()
{
  system::error_code ec;
  auto res = popen_.running(ec);
  return ec ? ec : system::result<bool>(res);
}

system::result<void> popen::close()
{
  return this->terminate();
}

system::result<void> popen::cancel()
{
  system::error_code ec;
  popen_.get_stdin().cancel(ec);
  if (ec)
    return ec;
  popen_.get_stdout().cancel(ec);
  if (ec)
    return ec;
  return {};
}

bool popen::is_open() const
{
  return this->popen_.is_open();
}

void popen::async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h)
{
  popen_.async_read_some(buffer, std::move(h));
}
void popen::async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h)
{
  popen_.async_write_some(buffer, std::move(h));
}


}