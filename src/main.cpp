//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "boost/async/main.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/signal_set.hpp>


namespace boost::async::detail
{

auto main_promise::final_suspend() noexcept -> std::suspend_never
{
  system::error_code ec;
  if (signal_set)
    signal_set->cancel(ec);
  return std::suspend_never(); // enable_yielding_tasks::final_suspend();
}

int main_promise::run_main(::boost::async::main mn)
{
  asio::io_context ctx{BOOST_ASIO_CONCURRENCY_HINT_1};
  boost::async::this_thread::set_executor(ctx.get_executor());
  int res = -1;
  mn.promise->result = &res;
  mn.promise->exec.emplace(ctx.get_executor());
  mn.promise->exec_ = mn.promise->exec->get_executor();
  auto p = std::coroutine_handle<detail::main_promise>::from_promise(*mn.promise);
  asio::basic_signal_set<executor_type> ss{ctx, SIGINT, SIGTERM};
  mn.promise->signal_set = &ss;
  struct work
  {
    asio::basic_signal_set<executor_type> & ss;
    asio::cancellation_signal & signal;
    void operator()(system::error_code ec, int sig) const
    {
      if (sig == SIGINT)
        signal.emit(asio::cancellation_type::total);
      if (sig == SIGTERM)
        signal.emit(asio::cancellation_type::terminal);
      if (!ec)
        ss.async_wait(*this);
    }
  };

  ss.async_wait(work{ss, mn.promise->signal});
  asio::post(ctx.get_executor(), [p]{p.resume();});

  ctx.run();
  return res;
}


}
