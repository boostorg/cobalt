//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_GENERATOR_HPP
#define BOOST_ASYNC_GENERATOR_HPP

#include <boost/async/detail/generator.hpp>


namespace boost::async
{

template<typename Yield, typename Push = void>
struct [[nodiscard]] generator : detail::generator_base<Yield, Push>
{
  using promise_type = detail::generator_promise<Yield, Push>;

  generator(const generator &) = delete;
  generator& operator=(const generator &) = delete;

  generator(generator &&lhs) noexcept = default;
  generator& operator=(generator &&) noexcept = default;

  explicit operator bool() const
  {
    return !receiver_.done || receiver_.result || receiver_.exception;
  }

  void cancel(asio::cancellation_type ct = asio::cancellation_type::all)
  {
    if (!receiver_.done && receiver_.reference == &receiver_)
      receiver_.cancel_signal.emit(ct);
  }

  bool ready() const  { return receiver_.result || receiver_.exception; }

  Yield get()
  {
    if (!ready())
      boost::throw_exception(std::logic_error("generator not ready"));
    receiver_.rethrow_if();
    return receiver_.get_result();
  }
  ~generator() { cancel(); }
 private:
  template<typename, typename>
  friend struct detail::generator_base;
  template<typename, typename>
  friend struct detail::generator_promise;

  generator(detail::generator_promise<Yield, Push> * generator) : receiver_(generator->receiver, generator->signal)
  {
  }
  detail::generator_receiver<Yield, Push> receiver_;
};

}

#endif //BOOST_ASYNC_GENERATOR_HPP
