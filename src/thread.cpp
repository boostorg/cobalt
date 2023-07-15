//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/thread.hpp>

namespace boost::async::detail
{

thread_promise::thread_promise()
    : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>(
    signal_helper_2::signal.slot(), asio::enable_total_cancellation())
{
}

void thread_promise::run()
{
  boost::async::this_thread::set_default_resource(&resource);
  auto st = state;

  if (st->signal.slot().is_connected())
    st->signal.slot().assign([this](asio::cancellation_type tp){signal.emit(tp);});

  exec.emplace(st->ctx.get_executor());
  boost::async::this_thread::set_executor(st->ctx.get_executor());

  asio::post(st->ctx.get_executor(),
             [this]
             {
               std::coroutine_handle<thread_promise>::from_promise(*this).resume();
             });
  try
  {
    st->ctx.run();
    st->done = true;
    if (st->waitor)
      asio::post(*std::exchange(st->waitor, std::nullopt));
  }
  catch(...)
  {
    std::lock_guard<std::mutex> lock(st->mtx);
    st->ep = std::current_exception();
    st->done = true;
    if (!st->waitor) // nobodies waiting, so unhandled exception
      throw;
    else
      asio::post(*std::exchange(st->waitor, std::nullopt));
  }

}


boost::async::thread detail::thread_promise::get_return_object()
{
  auto st = state;
  return boost::async::thread{std::thread{[this]{run();}}, std::move(st)};
}


}
