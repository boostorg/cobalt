// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/async.hpp>
#include <boost/async/detail/monotonic_resource.hpp>
#include <boost/asio.hpp>

#if defined(BOOST_ASYNC_BENCH_WITH_CONTEXT)
#include <boost/asio/spawn.hpp>
#endif

using namespace boost;
constexpr std::size_t n = 50'000'000ull;

struct std_test
{
  asio::io_context & ctx;
  std::size_t i = 0u;
  void operator()()
  {
    if (i ++ < n)
      asio::post(ctx, std::move(*this));
  }
};

char buf[1024];
async::detail::monotonic_resource res{buf, sizeof(buf)};

struct mono_test
{
  asio::io_context & ctx;
  std::size_t i = 0u;

  using allocator_type = async::detail::monotonic_allocator<void>;
  allocator_type get_allocator() const { return async::detail::monotonic_allocator<void>{&res}; }

  void operator()()
  {
    res.release();
    if (i ++ < n)
      asio::post(ctx, std::move(*this));
  }
};

async::pmr::monotonic_buffer_resource pmr_res{buf, sizeof(buf)};

struct pmr_test
{
  asio::io_context & ctx;
  std::size_t i = 0u;

  using allocator_type = async::pmr::polymorphic_allocator<void>;
  allocator_type get_allocator() const { return async::pmr::polymorphic_allocator<void>{&pmr_res}; }

  void operator()()
  {
    pmr_res.release();
    if (i ++ < n)
      asio::post(ctx, std::move(*this));
  }
};



int main(int argc, char * argv[])
{

  {
    auto start = std::chrono::steady_clock::now();
    asio::io_context ctx{BOOST_ASIO_CONCURRENCY_HINT_1};
    std_test{ctx}();
    ctx.run();
    auto end = std::chrono::steady_clock::now();
    printf("std::allocator  : %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }

  {
    auto start = std::chrono::steady_clock::now();
    asio::io_context ctx{BOOST_ASIO_CONCURRENCY_HINT_1};
    mono_test{ctx}();
    ctx.run();
    auto end = std::chrono::steady_clock::now();
    printf("async::monotonic: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }

  {
    auto start = std::chrono::steady_clock::now();
    asio::io_context ctx{BOOST_ASIO_CONCURRENCY_HINT_1};
    pmr_test{ctx}();
    ctx.run();
    auto end = std::chrono::steady_clock::now();
    printf("pmr::monotonic: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }


  return 0;
}