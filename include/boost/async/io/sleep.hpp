//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SLEEP_HPP
#define BOOST_ASYNC_IO_SLEEP_HPP

#include <boost/async/io/steady_timer.hpp>
#include <boost/async/io/system_timer.hpp>

#include <boost/async/promise.hpp>

namespace boost::async::detail::io
{

struct steady_sleep
{
  steady_sleep(const std::chrono::steady_clock::time_point & tp) : tim{tp} {}
  steady_sleep(const std::chrono::steady_clock::duration & du)   : tim{du} {}

  auto operator co_await() { return tim.wait(); }
 private:
  async::io::steady_timer tim;
};


struct system_sleep
{
  system_sleep(const std::chrono::system_clock::time_point & tp) : tim{tp} {}
  system_sleep(const std::chrono::system_clock::duration & du)   : tim{du} {}

  auto operator co_await() { return tim.wait(); }
 private:
  async::io::system_timer tim;
};

}

namespace boost::async::io
{



// NOTE: these don't need to be coros, we can optimize that out. Not sure that's worth it though
detail::io::steady_sleep sleep(const std::chrono::steady_clock::duration & d) { return d;}
detail::io::steady_sleep sleep(const std::chrono::steady_clock::time_point & tp) { return tp;}
detail::io::system_sleep sleep(const std::chrono::system_clock::time_point & tp) { return tp;}

template<typename Duration>
detail::io::steady_sleep sleep(const std::chrono::time_point<std::chrono::steady_clock, Duration> & tp)
{
  return sleep(std::chrono::time_point_cast<std::chrono::steady_clock::duration >(tp));
}


template<typename Duration>
detail::io::system_sleep sleep(const std::chrono::time_point<std::chrono::system_clock, Duration> & tp)
{
  return sleep(std::chrono::time_point_cast<std::chrono::system_clock::duration >(tp));
}


template<typename Rep, typename Period>
detail::io::steady_sleep sleep(const std::chrono::duration<Rep, Period> & dur)
{
  return sleep(std::chrono::duration_cast<std::chrono::steady_clock::duration >(dur));
}

}

#endif //BOOST_ASYNC_IO_SLEEP_HPP
