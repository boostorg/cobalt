//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETACHED_HPP
#define BOOST_ASYNC_DETACHED_HPP

#include <boost/async/promise.hpp>

namespace boost::async
{

// tag::outline[]
struct detached
{

  // movable
  detached(const detached &) = delete;
  detached& operator=(const detached &) = delete;

  detached(detached &&lhs) noexcept = default;

  // enable `co_await`. <1>
  auto operator co_await ();

  // Cancel the detached coroutine.
  void cancel(asio::cancellation_type ct = asio::cancellation_type::all);

  // Check if the result is ready
  bool ready() const;
  // end::outline[]
  /* tag::outline[]
  // Get the return value if ready - otherwise throw
  void get();
     end::outline[] */

  void get(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    if (!ready())
      boost::throw_exception(std::logic_error("detached not ready"), loc);

    receiver_.rethrow_if();
    return receiver_.get_result();
  }

  detached(promise<void> && promise) : receiver_(std::move(promise.receiver_))
  {
  }
  using promise_type = typename promise<void>::promise_type;
 private:
  template<typename>
  friend struct detail::async_promise;


  detail::promise_receiver<void> receiver_;
  friend struct detail::async_initiate;
  // tag::outline[]

};
// end::outline[]

inline
bool detached::ready() const  { return receiver_.done; }

inline
auto detached::operator co_await () {return receiver_.get_awaitable();}

inline
void detached::cancel(asio::cancellation_type ct)
{
  if (!receiver_.done && receiver_.reference == &receiver_)
    receiver_.cancel_signal.emit(ct);
}

}


#endif //BOOST_ASYNC_DETACHED_HPP
