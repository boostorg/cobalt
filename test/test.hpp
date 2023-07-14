// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_TEST2_HPP
#define BOOST_ASYNC_TEST2_HPP

#include <boost/async/task.hpp>
#include <boost/async/run.hpp>
#include <boost/async/spawn.hpp>

#include "doctest.h"

template<>
struct doctest::StringMaker<std::exception_ptr>
{
  static String convert(std::exception_ptr ex)
  {
    if (!ex)
      return "null";
    try
    {
      std::rethrow_exception(ex);
    }
    catch(std::exception & ex)
    {
      return ex.what();
    }
  }
};

inline void test_run(boost::async::task<void> (*func) ())
{
  using namespace boost;
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  async::pmr::unsynchronized_pool_resource res;
  async::this_thread::set_default_resource(&res);
  spawn(ctx, func(),
        +[](std::exception_ptr e)
        {
          CHECK(e == nullptr);
        });
  std::size_t n;
  n = ctx.run();

  if (::getenv("BOOST_ASYNC_BRUTE_FORCE"))
    while (n --> 0)
    {
      ctx.restart();
      spawn(ctx, func(),
            +[](std::exception_ptr e)
            {
              CHECK(e == nullptr);
            });
      for (std::size_t i = n; i > 0; i--)
        ctx.run_one();
    }

  async::this_thread::set_default_resource(async::pmr::get_default_resource());
}

// tag::test_case_macro[]
#define CO_TEST_CASE_IMPL(Function, ...)                                                                           \
static ::boost::async::task<void> Function();                                                                      \
DOCTEST_TEST_CASE(__VA_ARGS__)                                                                                     \
{                                                                                                                  \
    test_run(&Function);                                                                                           \
}                                                                                                                  \
static ::boost::async::task<void> Function()

#define CO_TEST_CASE(...) CO_TEST_CASE_IMPL(DOCTEST_ANONYMOUS(CO_DOCTEST_ANON_FUNC_), __VA_ARGS__)
// end::test_case_macro[]

struct stop
{
  bool await_ready() {return false;}
  void await_suspend(std::coroutine_handle<> h) { boost::async::detail::self_destroy(h); }
  void await_resume() {}
};

struct immediate
{
  int state = 0;
  immediate() = default;
  immediate(const immediate & i);
  bool await_ready() {CHECK(state++ == 0  ); return true;}
  void await_suspend(std::coroutine_handle<> h) { REQUIRE(false); }
  void await_resume() {CHECK(state++ == 1);}

  ~immediate()
  {
    CHECK(state == 2);
  }
};

struct immediate_bool
{
  int state = 0;

  bool await_ready() {CHECK(state++ == 0); return false;}
  bool await_suspend(std::coroutine_handle<> h) { CHECK(state++ == 1); return false; }
  void await_resume() {CHECK(state++ == 2);}

  ~immediate_bool()
  {
    CHECK(state == 3);
  }
};

struct immediate_handle
{
  int state = 0;

  bool await_ready() {CHECK(state++ == 0); return false;}
  std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) { CHECK(state++ == 1); return h; }
  void await_resume() {CHECK(state++ == 2);}

  ~immediate_handle()
  {
    CHECK(state == 3);
  }
};


struct posted
{
  int state = 0;

  bool await_ready() {CHECK(state++ == 0); return false;}
  void await_suspend(std::coroutine_handle<> h)
  {
    CHECK(state++ == 1);
    boost::asio::post(boost::async::this_thread::get_executor(), h);
  }
  void await_resume() {CHECK(state++ == 2);}
  ~posted()
  {
    CHECK(state == 3);
  }
};

struct posted_bool
{
  int state = 0;

  bool await_ready() {CHECK(state++ == 0); return false;}
  bool await_suspend(std::coroutine_handle<> h)
  {
    CHECK(state++ == 1);
    boost::asio::post(boost::async::this_thread::get_executor(), h);
    return true;
  }
  void await_resume() {CHECK(state++ == 2);}
  ~posted_bool()
  {
    CHECK(state == 3);
  }
};

struct posted_handle
{
  int state = 0;

  bool await_ready() {CHECK(state++ == 0); return false;}
  std::coroutine_handle<> await_suspend(std::coroutine_handle<> h)
  {
    CHECK(state++ == 1);
    return boost::async::detail::post_coroutine(
        boost::async::this_thread::get_executor(), h
        );
  }
  void await_resume() {CHECK(state++ == 2);}
  ~posted_handle()
  {
    CHECK(state == 3);
  }
};

#endif //BOOST_ASYNC_TEST2_HPP
