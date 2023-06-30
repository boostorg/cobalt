//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SYSTEM_TIMER_HPP
#define BOOST_ASYNC_IO_SYSTEM_TIMER_HPP

#include <boost/async/op.hpp>
#include <boost/asio/basic_waitable_timer.hpp>

#include <boost/system/result.hpp>

namespace boost::async::io
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

  BOOST_ASYNC_DECL system_timer();
  BOOST_ASYNC_DECL system_timer(const time_point& expiry_time);
  BOOST_ASYNC_DECL system_timer(const duration& expiry_time);

  BOOST_ASYNC_DECL void cancel();

  BOOST_ASYNC_DECL time_point expiry() const;
  BOOST_ASYNC_DECL void reset(const time_point& expiry_time);
  BOOST_ASYNC_DECL void reset(const duration& expiry_time);
  BOOST_ASYNC_DECL bool expired() const;

  struct wait_op_ : detail::deferred_op_resource_base
  {
    bool await_ready() const { return timer_->expired(); }
    BOOST_ASYNC_DECL void init_op(completion_handler<system::error_code> handler);

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        init_op(completion_handler<system::error_code>{h, result_, get_resource(h)});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }
    [[nodiscard]] wait_result await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      auto ec = std::get<0>(result_.value_or(std::make_tuple(system::error_code{})));
      return ec ? wait_result(ec) : system::in_place_value;
    }

    wait_op_(system_timer * timer) : timer_(timer) {}
   private:
    system_timer * timer_;
    std::exception_ptr error;
    std::optional<std::tuple<system::error_code>> result_;
  };

 public:
  [[nodiscard]] wait_op_ wait() { return wait_op_{this}; }
  wait_op_ operator co_await () { return wait(); }
 private:
  boost::asio::basic_waitable_timer<std::chrono::system_clock,
                                    asio::wait_traits<std::chrono::system_clock>,
                                    executor_type> timer_;
};

}

#endif //BOOST_ASYNC_IO_SYSTEM_TIMER_HPP
