//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PROCESS_ASYNC_IMPL_PROCESS_IPP
#define PROCESS_ASYNC_IMPL_PROCESS_IPP

#include <boost/async/process.hpp>
#include <boost/process/v2/impl/environment.ipp>


namespace boost::async
{

auto process::get_executor() -> executor_type { return impl_.get_executor(); }

auto process::handle() -> handle_type & { return impl_.handle(); }
process::process(implementation_type && impl) : impl_(std::move(impl)) {}

process::process(process&& lhs) = default;
process& process::operator=(process&& lhs) = default;
process::~process() = default;


process::process(executor_type executor, const filesystem::path& exe, std::initializer_list<string_view> args,  process_args && arg)
  : impl_(boost::process::v2::default_process_launcher()(executor, exe, std::move(args), std::move(arg.stdio), std::move(arg.start_dir), std::move(arg.env)))
{
}
process::process(executor_type executor, const filesystem::path& exe, std::initializer_list<wstring_view> args, process_args && arg)
  : impl_(boost::process::v2::default_process_launcher()(executor, exe, std::move(args), std::move(arg.stdio), std::move(arg.start_dir), std::move(arg.env)))
{
}
process::process(executor_type executor, const filesystem::path& exe, std::vector<string_view> args,            process_args && arg)
  : impl_(boost::process::v2::default_process_launcher()(executor, exe, std::move(args), std::move(arg.stdio), std::move(arg.start_dir), std::move(arg.env)))
{
}
process::process(asio::io_context & context, const filesystem::path& exe, std::initializer_list<string_view> args, process_args && arg)
  : impl_(boost::process::v2::default_process_launcher()(context.get_executor(), exe, std::move(args), std::move(arg.stdio), std::move(arg.start_dir), std::move(arg.env)))
{
}
process::process(asio::io_context & context, const filesystem::path& exe, std::vector<string_view> args,           process_args && arg)
  : impl_(boost::process::v2::default_process_launcher()(context.get_executor(), exe, std::move(args), std::move(arg.stdio), std::move(arg.start_dir), std::move(arg.env)))
{
}

process::process(executor_type exec, boost::process::v2::pid_type pid) : impl_(exec, pid) {}
process::process(executor_type exec, boost::process::v2::pid_type pid, native_handle_type native_handle) : impl_(exec, pid, native_handle) {}
process::process(executor_type exec) : impl_(exec) {}
process::process(asio::io_context & context, boost::process::v2::pid_type pid)                                   : impl_(context.get_executor(), pid) {}
process::process(asio::io_context & context, boost::process::v2::pid_type pid, native_handle_type native_handle) : impl_(context.get_executor(), pid, native_handle) {}
process::process(asio::io_context & context) : impl_(context) {}

void process::interrupt()                           {impl_.interrupt();}
void process::interrupt(system::error_code & ec)    {impl_.interrupt(ec);}
void process::request_exit()                        {impl_.request_exit();}
void process::request_exit(system::error_code & ec) {impl_.request_exit(ec);}
void process::terminate()                           {impl_.terminate();}
void process::terminate(system::error_code & ec)    {impl_.terminate(ec);}

int process::wait()                        { return impl_.wait(); }
int process::wait(system::error_code & ec) { return impl_.wait(ec); }
auto process::detach() -> handle_type               { return impl_.detach(); }


int process::exit_code() const { return impl_.exit_code(); }
boost::process::v2::pid_type process::id() const { return impl_.id(); }
boost::process::v2::native_exit_code_type process::native_exit_code() const { return impl_.native_exit_code(); }

bool process::running()                                 { return impl_.running(); };
bool process::running(system::error_code & ec) noexcept { return impl_.running(ec); };

bool process::is_open() const { return impl_.is_open(); }
void process::async_wait(detail::completion_handler<system::error_code, boost::process::v2::native_exit_code_type> h)
{
  impl_.async_wait(std::move(h));
}


}


#endif //PROCESS_ASYNC_IMPL_PROCESS_IPP
