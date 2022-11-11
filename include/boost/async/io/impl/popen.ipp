//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_POPEN_IPP
#define BOOST_ASYNC_IO_IMPL_POPEN_IPP

#include <boost/async/io/popen.hpp>

namespace boost::async::io
{


auto popen::get_executor() -> executor_type { return impl_.get_executor(); }

auto popen::handle() -> handle_type & { return impl_.handle(); }
popen::popen(implementation_type && impl) : impl_(std::move(impl)) {}

popen::popen(popen&& lhs) = default;
popen& popen::operator=(popen&& lhs) = default;
popen::~popen() = default;


popen::popen(executor_type executor, const filesystem::path& exe, std::initializer_list<string_view> args,  popen_args && arg)
    : impl_(executor, exe, std::move(args), std::move(arg.start_dir), std::move(arg.env))
{
}
popen::popen(executor_type executor, const filesystem::path& exe, std::initializer_list<wstring_view> args, popen_args && arg)
    : impl_(executor, exe, std::move(args), std::move(arg.start_dir), std::move(arg.env))
{
}
popen::popen(executor_type executor, const filesystem::path& exe, std::vector<string_view> args,            popen_args && arg)
    : impl_(executor, exe, std::move(args), std::move(arg.start_dir), std::move(arg.env))
{
}
popen::popen(asio::io_context & context, const filesystem::path& exe, std::initializer_list<string_view> args, popen_args && arg)
    : impl_(context.get_executor(), exe, std::move(args), std::move(arg.start_dir), std::move(arg.env))
{
}
popen::popen(asio::io_context & context, const filesystem::path& exe, std::vector<string_view> args,           popen_args && arg)
    : impl_(context.get_executor(), exe, std::move(args), std::move(arg.start_dir), std::move(arg.env))
{
}

popen::popen(executor_type exec) : impl_(exec) {}
popen::popen(asio::io_context & context) : impl_(context) {}

void popen::interrupt()                           {impl_.interrupt();}
void popen::interrupt(system::error_code & ec)    {impl_.interrupt(ec);}
void popen::request_exit()                        {impl_.request_exit();}
void popen::request_exit(system::error_code & ec) {impl_.request_exit(ec);}
void popen::terminate()                           {impl_.terminate();}
void popen::terminate(system::error_code & ec)    {impl_.terminate(ec);}

int popen::wait()                        { return impl_.wait(); }
int popen::wait(system::error_code & ec) { return impl_.wait(ec); }
auto popen::detach() -> handle_type               { return impl_.detach(); }


int popen::exit_code() const { return impl_.exit_code(); }
boost::process::v2::pid_type popen::id() const { return impl_.id(); }
boost::process::v2::native_exit_code_type popen::native_exit_code() const { return impl_.native_exit_code(); }

bool popen::running()                                 { return impl_.running(); };
bool popen::running(system::error_code & ec) noexcept { return impl_.running(ec); };

bool popen::is_open() const { return impl_.is_open(); }
void popen::async_wait(boost::async::detail::completion_handler<system::error_code, boost::process::v2::native_exit_code_type> h)
{
  impl_.async_wait(std::move(h));
}


void popen::async_write_some(any_const_buffer_range buffer, concepts::write_handler h)                 {impl_.async_write_some(buffer, std::move(h));}
void popen::async_write_some(const_buffer     buffer, concepts::write_handler h)                       {impl_.async_write_some(buffer, std::move(h));}
void popen::async_write_some(prepared_buffers buffer, concepts::write_handler h)                       {impl_.async_write_some(buffer, std::move(h));}
void popen::async_read_some(asio::mutable_buffer buffer,                     concepts::read_handler h) {impl_.async_read_some(buffer, std::move(h));}
void popen::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::read_handler h) {impl_.async_read_some(buffer, std::move(h));}
void popen::async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::read_handler h) {impl_.async_read_some(buffer, std::move(h));}


}

#endif //BOOST_ASYNC_IO_IMPL_POPEN_IPP
