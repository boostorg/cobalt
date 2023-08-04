// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/io/sleep.hpp>

namespace boost::async::detail::io
{

void steady_sleep::ready(async::handler<system::error_code> h)
{
  if (time_ < std::chrono::steady_clock::now())
    h({});
}
void steady_sleep::initiate(async::completion_handler<system::error_code> complete)
{
  auto & timer = timer_.emplace(complete.get_executor(), time_);
  timer.async_wait(std::move(complete));
}

void system_sleep::ready(async::handler<system::error_code> h)
{
  if (time_ < std::chrono::system_clock::now())
    h({});
}
void system_sleep::initiate(async::completion_handler<system::error_code> complete)
{
  auto &timer = timer_.emplace(complete.get_executor(), time_);
  timer.async_wait(std::move(complete));
}


}