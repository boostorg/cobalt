//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_TIMER_IMPL_STEADY_TIMER_IPP
#define BOOST_ASYNC_IO_TIMER_IMPL_STEADY_TIMER_IPP

#include <boost/async/io/timer/steady_timer.hpp>
#include <boost/asio/redirect_error.hpp>

namespace boost::async::io
{


steady_timer::steady_timer(const executor_type& ex)
  : impl_(ex)
{
}
steady_timer::steady_timer(asio::io_context& context)
  : impl_(context)
{
}
steady_timer::steady_timer(const executor_type& ex, const time_point& expiry_time)
  : impl_(ex, expiry_time)
{
}
steady_timer::steady_timer(asio::io_context& context, const time_point& expiry_time)
  : impl_(context, expiry_time)
{
}
steady_timer::steady_timer(const executor_type& ex, const duration& expiry_time)
  : impl_(ex, expiry_time)
{
}
steady_timer::steady_timer(asio::io_context& context, const duration& expiry_time)
  : impl_(context, expiry_time)
{
}


steady_timer::steady_timer(steady_timer&& other)  = default;
steady_timer& steady_timer::operator=(steady_timer&& other)  = default;
steady_timer::~steady_timer() = default;

auto steady_timer::get_executor() noexcept -> executor_type {return impl_.get_executor();}
std::size_t steady_timer::cancel()                                  { return impl_.cancel(); }
std::size_t steady_timer::cancel(boost::system::error_code& ec)     { return impl_.cancel(ec); }
std::size_t steady_timer::cancel_one()                              { return impl_.cancel_one(); }
std::size_t steady_timer::cancel_one(boost::system::error_code& ec) { return impl_.cancel_one(ec); }
auto steady_timer::expires_at() const -> time_point { return impl_.expires_at();}
std::size_t steady_timer::expires_at(const time_point& expiry_time) { return impl_.expires_at(expiry_time);}
std::size_t steady_timer::expires_at(const time_point& expiry_time, boost::system::error_code& ec) { return impl_.expires_at(expiry_time, ec);}
auto steady_timer::expires_from_now() const -> duration { return impl_.expires_from_now();}
std::size_t steady_timer::expires_from_now(const duration& expiry_time) { return impl_.expires_from_now(expiry_time);}
std::size_t steady_timer::expires_from_now(const duration& expiry_time,  boost::system::error_code& ec) { return impl_.expires_from_now(expiry_time, ec);}

std::size_t steady_timer::expires_after(const duration& expiry_time) { return impl_.expires_after(expiry_time);}
auto steady_timer::expiry() const -> time_point { return impl_.expiry();}


bool steady_timer::expired() const
{
  return expires_from_now().count() < 0;
}

void steady_timer::async_wait(boost::async::completion_handler<system::error_code> h)
{
  impl_.async_wait(std::move(h));
}

}

#endif //BOOST_ASYNC_IO_TIMER_IMPL_STEADY_TIMER_IPP
