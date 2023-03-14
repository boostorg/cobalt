//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_THREAD_HPP
#define BOOST_ASYNC_THREAD_HPP

#include <boost/async/detail/thread.hpp>



namespace boost::async
{

struct thread
{
  using promise_type = detail::thread_promise;

  void cancel(asio::cancellation_type type = asio::cancellation_type::all)
  {
    if (state_)
      asio::post(state_->ctx,[s= state_, type]{s->signal.emit(type);});
  }

  void join() {thread_.join();}
  bool joinable() const {return thread_.joinable();}
  void detach()
  {
    thread_.detach();
    state_ = nullptr;
  }

  auto operator co_await() &-> detail::thread_awaitable
  {
    return  detail::thread_awaitable{std::move(state_)};
  }
  auto operator co_await() && -> detail::thread_awaitable
  {
    return  detail::thread_awaitable{std::move(thread_), std::move(state_)};
  }

  ~thread()
  {
    if (state_)
      std::exchange(state_, nullptr)->ctx.stop();

    if (thread_.joinable())
      thread_.join();
  }
  thread(thread &&) noexcept = default;
 private:
  thread(std::thread thr, std::shared_ptr<detail::thread_promise::state_t> state)
      : thread_(std::move(thr)), state_(std::move(state))
  {
  }

  std::thread thread_;
  std::shared_ptr<detail::thread_promise::state_t> state_;
  friend struct detail::thread_promise;
};


}

#endif //BOOST_ASYNC_THREAD_HPP
