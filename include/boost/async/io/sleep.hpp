//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SLEEP_HPP
#define BOOST_ASYNC_IO_SLEEP_HPP


#include <boost/async/io/system_timer.hpp>
#include <boost/async/io/result.hpp>

#include <boost/asio/basic_waitable_timer.hpp>

#include <chrono>

namespace boost::async::detail::io
{


struct steady_sleep final : async::io::result_op<>
{
  steady_sleep(const std::chrono::steady_clock::time_point & tp) : time_{tp} {}
  steady_sleep(const std::chrono::steady_clock::duration & du)
      : time_{std::chrono::steady_clock::now() + du} {}

  BOOST_ASYNC_DECL void ready(async::handler<system::error_code> h);
  BOOST_ASYNC_DECL void initiate(async::completion_handler<system::error_code> complete);

 private:
  std::chrono::steady_clock::time_point time_;
  std::optional<boost::asio::basic_waitable_timer<std::chrono::steady_clock,
      asio::wait_traits<std::chrono::steady_clock>,
      executor>> timer_;
};


struct system_sleep final : async::io::result_op<>
{
  system_sleep(const std::chrono::system_clock::time_point & tp) : time_{tp} {}
  system_sleep(const std::chrono::system_clock::duration & du)
      : time_{std::chrono::system_clock::now() + du} {}

  BOOST_ASYNC_DECL void ready(async::handler<system::error_code> h);
  BOOST_ASYNC_DECL void initiate(async::completion_handler<system::error_code> complete);

  private:
  std::chrono::system_clock::time_point time_;
  std::optional<boost::asio::basic_waitable_timer<std::chrono::system_clock,
      asio::wait_traits<std::chrono::system_clock>,
      executor>> timer_;
};

}

namespace boost::async::io
{



// NOTE: these don't need to be coros, we can optimize that out. Not sure that's worth it though
inline detail::io::steady_sleep sleep(const std::chrono::steady_clock::duration & d) { return d;}
inline detail::io::steady_sleep sleep(const std::chrono::steady_clock::time_point & tp) { return tp;}
inline detail::io::system_sleep sleep(const std::chrono::system_clock::time_point & tp) { return tp;}

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
