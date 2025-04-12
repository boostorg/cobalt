//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/system_timer.hpp>

namespace boost::cobalt::io
{

system_timer::system_timer(const cobalt::executor & executor) : timer_(executor) {}
system_timer::system_timer(const time_point &expiry_time, const cobalt::executor & executor) : timer_(executor, expiry_time) {}
system_timer::system_timer(const duration &expiry_time, const cobalt::executor & executor) : timer_(executor, expiry_time) {}

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


void system_timer::initiate_wait_(void * this_, boost::cobalt::completion_handler<system::error_code> handler)
{
  static_cast<system_timer*>(this_)->timer_.async_wait(std::move(handler));
}

void system_timer::try_wait_(void * this_, boost::cobalt::handler<system::error_code> h)
{
  if (static_cast<system_timer*>(this_)->timer_.expiry()
      < std::chrono::system_clock::now())
    h({});
}

}