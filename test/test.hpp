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

inline auto test_run(boost::async::task<void> (*func) ())
{
  using namespace boost;
  asio::io_context ctx;
  async::this_thread::set_executor(ctx.get_executor());
  spawn(ctx, func(),
        [](std::exception_ptr e)
        {
          CHECK(e == nullptr);
        });
  std::size_t n = ctx.run();

  if (::getenv("BOOST_ASYNC_BRUTE_FORCE"))
    while (n --> 0)
    {
      ctx.reset();
      spawn(ctx, func(),
            [](std::exception_ptr e)
            {
              CHECK(e == nullptr);
            });
      for (std::size_t i = n; i > 0; i--)
        ctx.run_one();
    }
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

#endif //BOOST_ASYNC_TEST2_HPP
