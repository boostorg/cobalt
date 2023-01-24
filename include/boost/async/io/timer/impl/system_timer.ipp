//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_TIMER_IMPL_SYSTEM_TIMER_IPP
#define BOOST_ASYNC_IO_TIMER_IMPL_SYSTEM_TIMER_IPP

#include <boost/async/io/timer/system_timer.hpp>
#include <boost/asio/redirect_error.hpp>

namespace boost::async::io
{


system_timer::system_timer(const executor_type& ex)
  : impl_(ex)
{
}
system_timer::system_timer(asio::io_context& context)
  : impl_(context)
{
}
system_timer::system_timer(const executor_type& ex, const time_point& expiry_time)
  : impl_(ex, expiry_time)
{
}
system_timer::system_timer(asio::io_context& context, const time_point& expiry_time)
  : impl_(context, expiry_time)
{
}
system_timer::system_timer(const executor_type& ex, const duration& expiry_time)
  : impl_(ex, expiry_time)
{
}
system_timer::system_timer(asio::io_context& context, const duration& expiry_time)
  : impl_(context, expiry_time)
{
}


system_timer::system_timer(system_timer&& other)  = default;
system_timer& system_timer::operator=(system_timer&& other)  = default;
system_timer::~system_timer() = default;

auto system_timer::get_executor() noexcept -> executor_type {return impl_.get_executor();}
std::size_t system_timer::cancel()                                  { return impl_.cancel(); }
std::size_t system_timer::cancel(boost::system::error_code& ec)     { return impl_.cancel(ec); }
std::size_t system_timer::cancel_one()                              { return impl_.cancel_one(); }
std::size_t system_timer::cancel_one(boost::system::error_code& ec) { return impl_.cancel_one(ec); }
auto system_timer::expires_at() const -> time_point { return impl_.expires_at();}
std::size_t system_timer::expires_at(const time_point& expiry_time) { return impl_.expires_at(expiry_time);}
std::size_t system_timer::expires_at(const time_point& expiry_time, boost::system::error_code& ec) { return impl_.expires_at(expiry_time, ec);}
auto system_timer::expires_from_now() const -> duration { return impl_.expires_from_now();}
std::size_t system_timer::expires_from_now(const duration& expiry_time) { return impl_.expires_from_now(expiry_time);}
std::size_t system_timer::expires_from_now(const duration& expiry_time,  boost::system::error_code& ec) { return impl_.expires_from_now(expiry_time, ec);}

std::size_t system_timer::expires_after(const duration& expiry_time) { return impl_.expires_after(expiry_time);}
auto system_timer::expiry() const -> time_point { return impl_.expiry();}


bool system_timer::expired() const
{
  return expires_from_now().count() < 0;
}

void system_timer::async_wait(boost::async::completion_handler<system::error_code> h)
{
  impl_.async_wait(std::move(h));
}

}

#endif //BOOST_ASYNC_IO_TIMER_IMPL_SYSTEM_TIMER_IPP
