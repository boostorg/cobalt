//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_TIMER_IMPL_DEADLINE_TIMER_IPP
#define BOOST_ASYNC_IO_TIMER_IMPL_DEADLINE_TIMER_IPP

#include <boost/async/io/timer/deadline_timer.hpp>
#include <boost/asio/redirect_error.hpp>

namespace boost::async::io
{

deadline_timer::deadline_timer(const executor_type& ex)
  : impl_(ex)
{
}
deadline_timer::deadline_timer(asio::io_context& context)
  : impl_(context)
{
}
deadline_timer::deadline_timer(const executor_type& ex, const time_type& expiry_time)
  : impl_(ex, expiry_time)
{
}
deadline_timer::deadline_timer(asio::io_context& context, const time_type& expiry_time)
  : impl_(context, expiry_time)
{
}
deadline_timer::deadline_timer(const executor_type& ex, const duration_type& expiry_time)
  : impl_(ex, expiry_time)
{
}
deadline_timer::deadline_timer(asio::io_context& context, const duration_type& expiry_time)
  : impl_(context, expiry_time)
{
}


deadline_timer::deadline_timer(deadline_timer&& other)  = default;
deadline_timer& deadline_timer::operator=(deadline_timer&& other)  = default;
deadline_timer::~deadline_timer() = default;

auto deadline_timer::get_executor() noexcept -> executor_type {return impl_.get_executor();}
std::size_t deadline_timer::cancel()                                  { return impl_.cancel(); }
std::size_t deadline_timer::cancel(boost::system::error_code& ec)     { return impl_.cancel(ec); }
std::size_t deadline_timer::cancel_one()                              { return impl_.cancel_one(); }
std::size_t deadline_timer::cancel_one(boost::system::error_code& ec) { return impl_.cancel_one(ec); }
auto deadline_timer::expires_at() const -> time_type { return impl_.expires_at();}
std::size_t deadline_timer::expires_at(const time_type& expiry_time) { return impl_.expires_at(expiry_time);}
std::size_t deadline_timer::expires_at(const time_type& expiry_time, boost::system::error_code& ec) { return impl_.expires_at(expiry_time, ec);}
auto deadline_timer::expires_from_now() const -> duration_type { return impl_.expires_from_now();}
std::size_t deadline_timer::expires_from_now(const duration_type& expiry_time) { return impl_.expires_from_now(expiry_time);}
std::size_t deadline_timer::expires_from_now(const duration_type& expiry_time,  boost::system::error_code& ec) { return impl_.expires_from_now(expiry_time, ec);}


bool deadline_timer::expired() const
{
  return expires_from_now().total_nanoseconds() < 0ll;
}

void deadline_timer::async_wait(boost::async::completion_handler<system::error_code> h)
{
  impl_.async_wait(std::move(h));
}

}


#endif //BOOST_ASYNC_IO_TIMER_IMPL_DEADLINE_TIMER_IPP
