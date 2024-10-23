//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_IO_SYSTEM_TIMER_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_IO_SYSTEM_TIMER_HPP

#include <boost/cobalt/op.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>
#include <boost/asio/basic_waitable_timer.hpp>

#include <boost/system/result.hpp>

namespace boost::cobalt::experimental::io
{

struct system_timer final
{
  using wait_result = system::result<void>;

  /// The clock type.
  typedef std::chrono::system_clock clock_type;

  /// The duration type of the clock.
  typedef typename clock_type::duration duration;

  /// The time point type of the clock.
  typedef typename clock_type::time_point time_point;

  system_timer();
  system_timer(const time_point& expiry_time);
  system_timer(const duration& expiry_time);

  void cancel();

  time_point expiry() const;
  void reset(const time_point& expiry_time);
  void reset(const duration& expiry_time);
  bool expired() const;

  [[nodiscard]] wait_op wait() { return {this, initiate_wait_}; }
 private:
  BOOST_COBALT_DECL static void initiate_wait_(void *, boost::cobalt::completion_handler<boost::system::error_code>);
  boost::asio::basic_waitable_timer<std::chrono::system_clock,
                                    asio::wait_traits<std::chrono::system_clock>,
                                    executor> timer_;
};

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_IO_SYSTEM_TIMER_HPP
