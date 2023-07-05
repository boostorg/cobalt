//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/system_timer.hpp>

#include <boost/asio/redirect_error.hpp>

namespace boost::async::io
{

system_timer::system_timer() : timer_(this_thread::get_executor()) {}
system_timer::system_timer(const time_point &expiry_time) : timer_(this_thread::get_executor(), expiry_time) {}
system_timer::system_timer(const duration &expiry_time) : timer_(this_thread::get_executor(), expiry_time) {}

void system_timer::cancel()
{
  timer_.cancel();
}

auto system_timer::expiry() const -> time_point
{
  return timer_.expiry();
}

void system_timer::reset(const time_point &expiry_time)
{
  timer_.expires_at(expiry_time);
}

void system_timer::reset(const duration &expiry_time)
{
  timer_.expires_after(expiry_time);
}

bool system_timer::expired() const { return timer_.expiry() < clock_type::now(); }

void system_timer::wait_op_::ready(async::handler<system::error_code> h)
{
  if (timer_->expired())
    h({});
}
void system_timer::wait_op_::initiate(async::completion_handler<system::error_code> complete)
{
  timer_->timer_.async_wait(std::move(complete));
}

}