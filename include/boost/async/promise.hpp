//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_PROMISE_HPP
#define BOOST_ASYNC_PROMISE_HPP

#include <boost/async/detail/promise.hpp>

#include <boost/asio/append.hpp>



namespace boost::async
{

template<typename Return>
struct [[nodiscard]] promise
{
    using promise_type = detail::async_promise<Return>;

    promise(const promise &) = delete;
    promise& operator=(const promise &) = delete;

    promise(promise &&lhs) noexcept = default;
    promise& operator=(promise &&) noexcept = default;

    auto operator co_await () {return receiver_.get_awaitable();}

    // Ignore the returns value
    void operator +() && {detach();}

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

    Return get()
    {
      if (!ready())
        boost::throw_exception(std::logic_error("promise not ready"));

      receiver_.rethrow_if();
      return receiver_.get_result();
    }

    bool attached() const {return attached_;}
    void detach() {attached_ = false;}
    void attach() {attached_ = false;}

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
};



}


#endif //BOOST_ASYNC_PROMISE_HPP
