//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_SIGNAL_SET_IPP
#define BOOST_ASYNC_IO_IMPL_SIGNAL_SET_IPP

#include <boost/async/io/signal_set.hpp>

namespace boost::async::io
{

signal_set::signal_set(const executor_type& ex)
  : impl_(ex)
{
}
signal_set::signal_set(asio::io_context& context)
  : impl_(context)
{
}
signal_set::signal_set(const executor_type& ex, int signal_number_1)
  : impl_(ex)
{
}
signal_set::signal_set(asio::io_context& context, int signal_number_1)
  : impl_(context, signal_number_1)
{
}
signal_set::signal_set(const executor_type& ex, int signal_number_1, int signal_number_2)
  : impl_(ex, signal_number_1, signal_number_2)
{
}
signal_set::signal_set(asio::io_context& context, int signal_number_1, int signal_number_2)
  : impl_(context, signal_number_1, signal_number_2)
{
}
signal_set::signal_set(const executor_type& ex, int signal_number_1, int signal_number_2, int signal_number_3)
  : impl_(ex, signal_number_1, signal_number_2, signal_number_3)
{
}
signal_set::signal_set(asio::io_context& context, int signal_number_1, int signal_number_2, int signal_number_3)
  : impl_(context, signal_number_1, signal_number_2, signal_number_3)
{
}

signal_set::~signal_set() = default;


auto signal_set::get_executor() noexcept -> executor_type { return impl_.get_executor(); }

void signal_set::add(int signal_number)                                   {impl_.add(signal_number); }
void signal_set::add(int signal_number, boost::system::error_code& ec)    {impl_.add(signal_number, ec); }
void signal_set::remove(int signal_number)                                {impl_.remove(signal_number); }
void signal_set::remove(int signal_number, boost::system::error_code& ec) {impl_.remove(signal_number, ec); }
void signal_set::clear()                               {impl_.clear(); }
void signal_set::clear(boost::system::error_code& ec)  {impl_.clear(ec); }
void signal_set::cancel()                              {impl_.cancel(); }
void signal_set::cancel(boost::system::error_code& ec) {impl_.cancel(); }


void signal_set::wait_op_::await_resume_impl(boost::async::completion_handler<system::error_code, int> h)
{
  return this->impl.async_wait(std::move(h));
}

void signal_set::wait_op_ec_::await_resume_impl(boost::async::completion_handler<int> h)
{
  return this->impl.async_wait(asio::redirect_error(std::move(h), ec));
}

}

#endif //BOOST_ASYNC_IO_IMPL_SIGNAL_SET_IPP
