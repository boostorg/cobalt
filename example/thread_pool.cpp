// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// This example shows how to use threads to offload cpu_intense work.

#include <boost/async.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/thread_pool.hpp>


namespace async = boost::async;
using boost::system::error_code;

// this is a function doing some CPU heavy work that should be offloaded onto a thread_pool
async::promise<int> cpu_intense_work(
    int a, int b,
    boost::asio::executor_arg_t = {}, async::executor = async::this_thread::get_executor())
    // ^set the executor manually. but default it so we can still use it with the thread_local one if present
{
  co_return a + b;
}

async::task<void> work(int min_a, int max_a, int b)
{
  auto exec = co_await async::this_coro::executor;
  for (int a = min_a; a <= max_a; a++)
  {
    // the following two calls offload the work to another thread.
    int c = co_await cpu_intense_work(a, b, boost::asio::executor_arg, exec);
    printf("The CPU intensive result of adding %d to %d, is %d\n", a, b, c);
  }
}
int main(int , char * [])
{
  const std::size_t n = 4u;
  boost::asio::thread_pool tp{n};

  // a very simple thread pool

  auto cpl =
      [](std::exception_ptr ep)
      {
          if (ep)
            try
            {
              std::rethrow_exception(ep);
            }
            catch(std::exception & e)
            {
              printf("Completed with exception %s\n", e.what());
            }
          };

  async::spawn(boost::asio::make_strand(tp.get_executor()), work(0, 10, 32),   cpl);
  async::spawn(boost::asio::make_strand(tp.get_executor()), work(10, 20, 22),  cpl);
  async::spawn(boost::asio::make_strand(tp.get_executor()), work(50, 60, -18), cpl);

  // wait them so they don't leak.
  tp.join();
  return 0;
}