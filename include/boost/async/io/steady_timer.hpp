//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_STEADY_TIMER_HPP
#define BOOST_ASYNC_IO_STEADY_TIMER_HPP

#include <boost/async/io/result.hpp>
#include <boost/asio/basic_waitable_timer.hpp>

#include <boost/system/result.hpp>

namespace boost::async::detail::io
{
struct steady_sleep;
}

namespace boost::async::io
{

struct steady_timer
{
  using wait_result = system::result<void>;

  /// The clock type.
  typedef std::chrono::steady_clock clock_type;

  /// The duration type of the clock.
  typedef typename clock_type::duration duration;

  /// The time point type of the clock.
  typedef typename clock_type::time_point time_point;

  BOOST_ASYNC_DECL steady_timer();
  BOOST_ASYNC_DECL steady_timer(const time_point& expiry_time);
  BOOST_ASYNC_DECL steady_timer(const duration& expiry_time);

  BOOST_ASYNC_DECL void cancel();

  BOOST_ASYNC_DECL time_point expiry() const;
  BOOST_ASYNC_DECL void reset(const time_point& expiry_time);
  BOOST_ASYNC_DECL void reset(const duration& expiry_time);
  BOOST_ASYNC_DECL bool expired() const;

 private:
  struct wait_op_ final : result_op<void>
  {
    void ready(async::handler<system::error_code> h) {if (timer_->expired()) h({});}
    BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code> handler) override;
    wait_op_(steady_timer * timer) : timer_(timer) {}
   private:
    steady_timer * timer_;
  };

 public:
  [[nodiscard]] wait_op_ wait() { return wait_op_{this}; }
  wait_op_ operator co_await () { return wait(); }
 private:
  friend struct detail::io::steady_sleep;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock,
                                    asio::wait_traits<std::chrono::steady_clock>,
                                    executor> timer_;
};

}

#endif //BOOST_ASYNC_IO_STEADY_TIMER_HPP
