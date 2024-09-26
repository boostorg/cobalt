//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_IO_SYSTEM_TIMER_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_IO_SYSTEM_TIMER_HPP

#include <boost/cobalt/op.hpp>
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

  struct wait_op_ final : cobalt::op<system::error_code>
  {
    void ready(handler<system::error_code> h) override
    {
      if (timer_->expired())
        h({});
    }
    BOOST_COBALT_DECL
    void initiate(completion_handler<system::error_code> h) override;

    wait_op_(system_timer * timer) : timer_(timer) {}
   private:
    system_timer * timer_;
  };

 public:
  [[nodiscard]] wait_op_ wait() { return wait_op_{this}; }
 private:
  boost::asio::basic_waitable_timer<std::chrono::system_clock,
                                    asio::wait_traits<std::chrono::system_clock>,
                                    executor> timer_;
};

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_IO_SYSTEM_TIMER_HPP
