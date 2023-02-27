// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/with.hpp>

// Suppress
template<typename Exception>
struct suppress_exception
{
};

template<typename Exception>
auto tag_invoke(const boost::async::with_exit_tag & wet,
                const suppress_exception<Exception> &,
                std::exception_ptr & e) -> std::suspend_never
{
    try
    {
      if (e)
        std::rethrow_exception(e);
    }
    catch(Exception &)
    {
      e = nullptr;
    }
    catch(...)
    {

    }
    return std::suspend_never{};
}

template<typename Exception>
suppress_exception<Exception> suppress() {return {}; }

struct suppress_error_code
{
  boost::system::error_code to_suppress;
};

auto tag_invoke(const boost::async::with_exit_tag & wet,
                const suppress_error_code & sec,
                std::exception_ptr &e) ->  std::suspend_never
{
  try
  {
    if (e)
      std::rethrow_exception(e);
  }
  catch(boost::system::system_error & se)
  {
    if (se.code() == sec.to_suppress)
      e = nullptr;
  }
  catch(...)
  {
  }
  return  std::suspend_never{};
}

suppress_error_code suppress(boost::system::error_code ec) {return suppress_error_code{ec}; }

#include <boost/async/main.hpp>
#include <boost/async/op.hpp>
#include <boost/async/promise.hpp>
#include <boost/asio/steady_timer.hpp>

using namespace boost;

async::promise<void> delay(std::chrono::milliseconds ms)
{
  asio::steady_timer tim{co_await async::this_coro::executor, ms};
  co_await tim.async_wait(async::use_op);
}

async::promise<void> suppress_all()
{
  co_await async::with(suppress<system::system_error>(),
                  [](const auto & )
                  {
                    return delay(std::chrono::milliseconds(100000));
                  });
}

async::promise<void> suppress_cancelled()
{
  co_await async::with(suppress(asio::error::operation_aborted),
                       [](const auto & )
                       {
                         return delay(std::chrono::milliseconds(100000));
                       });
}

async::main co_main(int argc, char * argv[])
{

  auto p1 = suppress_all();
  auto p2 = suppress_cancelled();
  co_await delay(std::chrono::milliseconds(10));
  p1.cancel();
  p2.cancel();
  co_await p1; // suppress system_error
  co_await p2; // suppress operation_aborted error

  co_return 0;
}
