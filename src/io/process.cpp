//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/process.hpp>

namespace boost::async::io
{

process::process(boost::process::v2::filesystem::path executable,
                 std::initializer_list<core::string_view> args,
                 process_initializer initializer,
                 const async::executor & exec)
    : process_(exec,
               executable,
               args,
               initializer.stdio,
               initializer.start_dir,
               initializer.env) {}


process::process(boost::process::v2::filesystem::path executable,
                 std::span<core::string_view> args,
                 process_initializer initializer,
                 const async::executor & exec)
    : process_(exec,
               executable,
               args,
               initializer.stdio,
               initializer.start_dir,
               initializer.env) {}


process::process(pid_type pid, const async::executor & exec) : process_(exec, pid) {}
process::process(pid_type pid, native_handle_type native_handle, const async::executor & exec)
                : process_(exec, pid, native_handle) {}

void process::wait_op_::initiate(completion_handler<system::error_code, int> handler)
{
  process_.async_wait(std::move(handler));
}

pid_type process::id() const {return process_.id();}

system::result<void> process::interrupt()
{
  system::error_code ec;
  process_.interrupt(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> process::request_exit()
{
  system::error_code ec;
  process_.request_exit(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> process::suspend()
{
  system::error_code ec;
  process_.suspend(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> process::resume()
{
  system::error_code ec;
  process_.resume(ec);
  return ec ? ec : system::result<void>();
}
system::result<void> process::terminate()
{
  system::error_code ec;
  process_.terminate(ec);
  return ec ? ec : system::result<void>();
}
process::handle_type process::detach()
{
  return process_.detach();
}
system::result<bool> process::running()
{
  system::error_code ec;
  auto res = process_.running(ec);
  return ec ? system::result<bool>(system::in_place_error, ec) : system::result<bool>(res);
}

}