//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_THREAD_HPP
#define BOOST_ASYNC_THREAD_HPP

#include <boost/async/detail/thread.hpp>
#include <boost/async/detail/await_result_helper.hpp>



namespace boost::async
{

struct thread
{
  using promise_type = detail::thread_promise;

  void cancel(asio::cancellation_type type = asio::cancellation_type::all)
  {
    if (auto st = state_)
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
    return detail::thread_awaitable{std::move(state_)};
  }
  auto operator co_await() && -> detail::thread_awaitable
  {
    return detail::thread_awaitable{std::move(thread_), std::move(state_)};
  }

  ~thread()
  {
    if (state_)
    {
      state_->ctx.stop();
      state_.reset();
    }

    if (thread_.joinable())
      thread_.join();
  }
  thread(thread &&) noexcept = default;


  using executor_type = executor;
  executor_type get_executor(const boost::source_location & loc = BOOST_CURRENT_LOCATION) const
  {
    auto st = state_;
    if (!st || st->done)
      boost::throw_exception(asio::execution::bad_executor(), loc);

    return st ->ctx.get_executor();
  }

  using id = std::thread::id;
  id get_id() const noexcept {return thread_.get_id();}
 private:
  thread(std::thread thr, std::shared_ptr<detail::thread_state> state)
      : thread_(std::move(thr)), state_(std::move(state))
  {
  }

  std::thread thread_;
  std::shared_ptr<detail::thread_state> state_;
  friend struct detail::thread_promise;
};



}

#endif //BOOST_ASYNC_THREAD_HPP
