// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/async.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/parallel_group.hpp>

#if defined(BOOST_ASYNC_BENCH_WITH_CONTEXT)
#include <boost/asio/spawn.hpp>
#endif

using namespace boost;
constexpr std::size_t n = 3'000'000ull;

async::task<void> atest()
{
  async::channel<void> chan{0u};
  for (std::size_t i = 0u; i < n; i++)
    co_await async::gather(chan.write(), chan.read());

}

asio::awaitable<void> awtest()
{
  asio::experimental::channel<void(system::error_code)> chan{co_await async::this_coro::executor, 0u};
  for (std::size_t i = 0u; i < n; i++)
    co_await asio::experimental::make_parallel_group(
        chan.async_send(system::error_code{}, asio::deferred),
        chan.async_receive(asio::deferred))
        .async_wait(asio::experimental::wait_for_all(), asio::deferred);
}

#if defined(BOOST_ASYNC_BENCH_WITH_CONTEXT)

void stest(asio::yield_context ctx)
{
  asio::experimental::channel<void(system::error_code)> chan{ctx.get_executor(), 0u};
  for (std::size_t i = 0u; i < n; i++)
    asio::experimental::make_parallel_group(
        chan.async_send(system::error_code{}, asio::deferred),
        chan.async_receive(asio::deferred))
        .async_wait(asio::experimental::wait_for_all(), ctx);
}
#endif


int main(int argc, char * argv[])
{
  {
    auto start = std::chrono::steady_clock::now();
    async::run(atest());
    auto end = std::chrono::steady_clock::now();
    printf("async    : %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }

  {
    auto start = std::chrono::steady_clock::now();
    asio::io_context ctx{BOOST_ASIO_CONCURRENCY_HINT_1};
    asio::co_spawn(ctx, awtest(), asio::detached);
    ctx.run();
    auto end = std::chrono::steady_clock::now();
    printf("awaitable: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }

#if defined(BOOST_ASYNC_BENCH_WITH_CONTEXT)
  {
    auto start = std::chrono::steady_clock::now();
    asio::io_context ctx{BOOST_ASIO_CONCURRENCY_HINT_1};
    asio::spawn(ctx, stest, asio::detached);
    ctx.run();
    auto end = std::chrono::steady_clock::now();
    printf("stackful : %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }
#endif

  return 0;
}