//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/thread.hpp>

#include <boost/asio/append.hpp>
#include <boost/asio/bind_executor.hpp>


namespace boost::async::detail
{

thread_promise::thread_promise()
    : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>(
    signal_helper_2::signal.slot(), asio::enable_total_cancellation())
{
  mtx.lock();
}

void run_thread(
    std::shared_ptr<thread_state> st,
    std::unique_ptr<thread_promise, coro_deleter<thread_promise>> h)
{

  pmr::unsynchronized_pool_resource resource;
  boost::async::this_thread::set_default_resource(&resource);
  h->resource = &resource;

  h->reset_cancellation_source(st->signal.slot());
  h->set_executor(st->ctx.get_executor());
  boost::async::this_thread::set_executor(st->ctx.get_executor());

  asio::post(
      st->ctx.get_executor(),
      [&st, h = std::move(h)]() mutable
      {
        std::lock_guard<std::mutex> lock{h->mtx};
        std::coroutine_handle<thread_promise>::from_promise(*h.release()).resume();
      });

  std::exception_ptr ep;

  try
  {
    st->ctx.run();
  }
  catch(...)
  {
    ep = std::current_exception();
  }

  st->done = true;
  st->signal.slot().clear();
  std::lock_guard<std::mutex> lock(st->mtx);
  if (!st->waitor && ep) // nobodies waiting, so unhandled exception
    std::rethrow_exception(ep);
  else if (st->waitor)
    asio::post(asio::append(*std::exchange(st->waitor, std::nullopt), ep));
}


boost::async::thread detail::thread_promise::get_return_object()
{
  auto st = std::make_shared<thread_state>();
  boost::async::thread res{std::thread{
      [st,
       h = std::unique_ptr<thread_promise, coro_deleter<thread_promise>>(this)]() mutable
      {
        run_thread(std::move(st), std::move(h));
      }
     }, st
    };

  return res;
 }


}
