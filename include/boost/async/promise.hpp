//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_PROMISE_HPP
#define BOOST_ASYNC_PROMISE_HPP

#include <boost/async/detail/promise.hpp>

namespace boost::async
{

// tag::outline[]
template<typename Return>
struct [[nodiscard]] promise
{
    promise(promise &&lhs) noexcept;
    promise& operator=(promise && lhs) noexcept;

    // enable `co_await`. <1>
    auto operator co_await ();

    // Ignore the return value, i.e. detach it. <2>
    void operator +() &&;

    // Cancel the promise.
    void cancel(asio::cancellation_type ct = asio::cancellation_type::all);

    // Check if the result is ready
    bool ready() const;
    // Check if the promise can be awaited.
    explicit operator bool () const; // <3>

    // Detach or attach
    bool attached() const;
    void detach();
    void attach();
    // end::outline[]

    /* tag::outline[]
    // Get the return value if ready - otherwise throw
    Return get();
    end::outline[] */

    Return get(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      if (!ready())
        boost::throw_exception(std::logic_error("promise not ready"), loc);

      receiver_.rethrow_if();
      return receiver_.get_result();
    }

    using promise_type = detail::async_promise<Return>;
    promise(const promise &) = delete;
    promise& operator=(const promise &) = delete;

    ~promise()
    {
      if (attached_)
        cancel();
    }
  private:
    template<typename>
    friend struct detail::async_promise;

    promise(detail::async_promise<Return> * promise) : receiver_(promise->receiver, promise->signal), attached_{true}
    {
    }

    detail::promise_receiver<Return> receiver_;
    bool attached_;
    friend struct detail::async_initiate;

    friend struct detached;
    //tag::outline[]
};
// end::outline[]

template<typename T>
inline
promise<T>::promise(promise &&lhs) noexcept
    : receiver_(std::move(lhs.receiver_)), attached_(std::exchange(lhs.attached_, false))
{
}

template<typename T>
inline
promise<T>& promise<T>::operator=(promise && lhs) noexcept
{
    if (attached_)
    cancel();
  receiver_ = std::move(lhs.receiver_);
  attached_ = std::exchange(lhs.attached_, false);
}

template<typename T>
inline
auto promise<T>::operator co_await () {return receiver_.get_awaitable();}

// Ignore the returns value
template<typename T>
inline
void promise<T>::operator +() && {detach();}

template<typename T>
inline
void promise<T>::cancel(asio::cancellation_type ct)
{
  if (!receiver_.done && receiver_.reference == &receiver_)
    receiver_.cancel_signal.emit(ct);
}

template<typename T>
inline
bool promise<T>::ready() const  { return receiver_.done; }

template<typename T>
inline
promise<T>::operator bool () const
{
  return !receiver_.done || !receiver_.result_taken;
}

template<typename T>
inline
bool promise<T>::attached() const {return attached_;}

template<typename T>
inline
void promise<T>::detach() {attached_ = false;}
template<typename T>
inline
void promise<T>::attach() {attached_ = true;}


}

#endif //BOOST_ASYNC_PROMISE_HPP
