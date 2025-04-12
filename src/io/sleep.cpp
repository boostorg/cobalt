//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/sleep.hpp>

namespace boost::cobalt::detail::io
{

steady_sleep::steady_sleep(const std::chrono::steady_clock::time_point & tp) : tp{tp} {}
steady_sleep::steady_sleep(const std::chrono::steady_clock::duration & du)   : tp{std::chrono::steady_clock::now() + du} {}

system_sleep::system_sleep(const std::chrono::system_clock::time_point & tp) : tp{tp} {}
system_sleep::system_sleep(const std::chrono::system_clock::duration & du)   : tp{std::chrono::system_clock::now() + du} {}


void steady_sleep::initiate(completion_handler<system::error_code> h)
{
  timer_.emplace(h.get_executor(), tp);
  timer_->async_wait(std::move(h));
}

void system_sleep::initiate(completion_handler<system::error_code> h)
{
  timer_.emplace(h.get_executor(), tp);
  timer_->async_wait(std::move(h));
}

}
