// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_COBALT_TEST2_HPP
#define BOOST_COBALT_TEST2_HPP

#include <boost/cobalt/task.hpp>
#include <boost/cobalt/run.hpp>
#include <boost/cobalt/spawn.hpp>

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

inline void test_run(boost::cobalt::task<void> (*func) ())
{
  using namespace boost;
#if !defined(BOOST_COBALT_NO_PMR)
  cobalt::pmr::unsynchronized_pool_resource res;
  cobalt::this_thread::set_default_resource(&res);
#endif
  {
    asio::io_context ctx;
    cobalt::this_thread::set_executor(ctx.get_executor());
    spawn(ctx, func(),
          +[](std::exception_ptr e)
          {
            CHECK(e == nullptr);
          });
    std::size_t n;
    n = ctx.run();

    if (::getenv("BOOST_COBALT_BRUTE_FORCE"))
      while (n-- > 0)
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
  }
#if !defined(BOOST_COBALT_NO_PMR)
  cobalt::this_thread::set_default_resource(cobalt::pmr::get_default_resource());
#endif
}

// tag::test_case_macro[]
#define CO_TEST_CASE_IMPL(Function, ...)                                                                           \
static ::boost::cobalt::task<void> Function();                                                                      \
DOCTEST_TEST_CASE(__VA_ARGS__)                                                                                     \
{                                                                                                                  \
    test_run(&Function);                                                                                           \
}                                                                                                                  \
static ::boost::cobalt::task<void> Function()

#define CO_TEST_CASE(...) CO_TEST_CASE_IMPL(DOCTEST_ANONYMOUS(CO_DOCTEST_ANON_FUNC_), __VA_ARGS__)
// end::test_case_macro[]

struct stop
{
  bool await_ready() {return false;}
  void await_suspend(std::coroutine_handle<> h) { boost::cobalt::detail::self_destroy(h); }
  void await_resume() {}
};

struct immediate
{
  int state = 0;
  immediate() = default;
  immediate(const immediate & i);
  bool await_ready() {CHECK(state++ == 0  ); return true;}
  void await_suspend(std::coroutine_handle<>) { REQUIRE(false); }
  void await_resume() {CHECK(state++ == 1);}

  ~immediate()
  {
    if (state != 0)
      CHECK(state == 2);
  }
};

struct immediate_bool
{
  int state = 0;

  bool await_ready() {CHECK(state++ == 0); return false;}
  bool await_suspend(std::coroutine_handle<>) { CHECK(state++ == 1); return false; }
  void await_resume() {CHECK(state++ == 2);}

  ~immediate_bool()
  {
    if (state != 0)
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
    if (state != 0)
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
    boost::asio::post(boost::cobalt::this_thread::get_executor(), h);
  }
  void await_resume() {CHECK(state++ == 2);}
  ~posted()
  {
    if (state != 0)
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
    boost::asio::post(boost::cobalt::this_thread::get_executor(), h);
    return true;
  }
  void await_resume() {CHECK(state++ == 2);}
  ~posted_bool()
  {
    if (state != 0)
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
    return boost::cobalt::detail::post_coroutine(
        boost::cobalt::this_thread::get_executor(), h
        );
  }
  void await_resume() {CHECK(state++ == 2);}
  ~posted_handle()
  {
    if (state != 0)
      CHECK(state == 3);
  }
};

#endif //BOOST_COBALT_TEST2_HPP
