//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETACHED_HPP
#define BOOST_ASYNC_DETACHED_HPP

#include <boost/async/detail/promise.hpp>

namespace boost::async
{

struct detached
{
  using promise_type = detail::async_promise<void>;

  detached(const detached &) = delete;
  detached& operator=(const detached &) = delete;

  detached(detached &&lhs) noexcept = default;
  detached& operator=(detached &&) noexcept = default;

  auto operator co_await () {return receiver_.get_awaitable();}

  void cancel(asio::cancellation_type ct = asio::cancellation_type{0b111u})
  {
    if (!receiver_.done && receiver_.reference == &receiver_)
      receiver_.cancel_signal.emit(ct);
  }

  bool ready() const  { return receiver_.done; }
  explicit operator bool () const
  {
    return !receiver_.done || !receiver_.result_taken;
  }

  void get(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    if (!ready())
      boost::throw_exception(std::logic_error("promise not ready"), loc);

    receiver_.rethrow_if();
    return receiver_.get_result();
  }

 private:
  template<typename>
  friend struct detail::async_promise;

  detached(detail::async_promise<void> * promise) : receiver_(promise->receiver, promise->signal)
  {
  }

  detail::promise_receiver<void> receiver_;
  friend struct detail::async_initiate;
};



}


#endif //BOOST_ASYNC_DETACHED_HPP
