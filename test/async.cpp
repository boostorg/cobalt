//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <coro/async.hpp>
#include <coro/main.hpp>

#include "doctest.h"
#include "test.hpp"
#include "asio/detached.hpp"
#include <asio/steady_timer.hpp>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>

#include <new>

TEST_SUITE_BEGIN("async");

coro::async<void> test0()
{
    co_return;
}
coro::async<double> test2(int i)
{
    co_await test0();
    co_return i;
}

coro::async<int> test1(asio::any_io_executor exec)
{
    co_await test2(42);
    co_await test2(42);
    co_await asio::post(exec, asio::deferred);
    co_return 452;
}

struct tracking_res final : std::pmr::memory_resource
{
  void* do_allocate( std::size_t bytes, std::size_t alignment ) override
  {
    auto p = std::pmr::new_delete_resource()->allocate(bytes, alignment);
    printf("Helau %d -> %p", bytes, p);
    return p;
  }

  void do_deallocate( void* p, std::size_t bytes, std::size_t alignment ) override
  {
    printf("Byebye %d -> %p", bytes, p);
    std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
  }

  bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept
  {
    return dynamic_cast<const tracking_res*>(&other) != nullptr;
  }


};

TEST_CASE("test-1")
{

    bool done = false;



    asio::io_context ctx;
    asio::steady_timer tim{ctx};

    tracking_res res;
    asio::co_spawn(ctx,
                   []() -> asio::awaitable<void> {co_return;},
                   asio::bind_allocator(std::pmr::polymorphic_allocator<void>(&res), asio::detached));
    coro::spawn(
          ctx.get_executor(),
          test1(ctx.get_executor()),
          [&](std::exception_ptr ex, int res)
          {
            CHECK(ex == nullptr);
            CHECK(res == 452);
            done = true;
          });

    ctx.run();
    CHECK(done);
}

CO_TEST_CASE("async-1")
{
    co_await test1(co_await asio::this_coro::executor);
    co_return;
}

TEST_SUITE_END();