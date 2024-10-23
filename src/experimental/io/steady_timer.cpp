//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/steady_timer.hpp>

#include <boost/asio/redirect_error.hpp>

namespace boost::cobalt::experimental::io
{

steady_timer::steady_timer() : timer_(this_thread::get_executor()) {}
steady_timer::steady_timer(const time_point &expiry_time) : timer_(this_thread::get_executor(), expiry_time) {}
steady_timer::steady_timer(const duration &expiry_time) : timer_(this_thread::get_executor(), expiry_time) {}

void steady_timer::cancel()
{
  timer_.cancel();
}


auto steady_timer::expiry() const -> time_point
{
  return timer_.expiry();
}

void steady_timer::reset(const time_point &expiry_time)
{
  timer_.expires_at(expiry_time);
}

void steady_timer::reset(const duration &expiry_time)
{
  timer_.expires_after(expiry_time);
}

bool steady_timer::expired() const { return timer_.expiry() < clock_type::now(); }

void steady_timer::initiate_wait_(void * this_, boost::cobalt::completion_handler<boost::system::error_code> handler)
{
  static_cast<steady_timer*>(this_)->timer_.async_wait(std::move(handler));
}


}