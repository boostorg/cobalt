//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/promise.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/with.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>

#include "doctest.h"
#include "test.hpp"

TEST_SUITE_BEGIN("with");

namespace asio = boost::asio;

struct finalizer_test
{
    using executor_type = boost::cobalt::executor ;
    executor_type exec;
    executor_type get_executor()
    {
        return exec;
    }

    bool exit_called = false;
    std::exception_ptr e{};

    auto exit(std::exception_ptr ee)
    {
        exit_called = true;
        e = ee;
        return asio::post(exec, boost::cobalt::use_op);
    }

    ~finalizer_test()
    {
        CHECK(e != nullptr);
    }
};

struct value_finalizer_test
{
  using executor_type = boost::cobalt::executor ;
  executor_type exec;
  executor_type get_executor()
  {
    return exec;
  }

  bool exit_called = false;
  std::exception_ptr e;

  auto exit(std::exception_ptr ee)
  {
    exit_called = true;
    CHECK(!ee);
    return asio::post(exec, boost::cobalt::use_op);
  }

};

CO_TEST_CASE("sync")
{
    finalizer_test ft{co_await boost::cobalt::this_coro::executor};

    CHECK_THROWS(
        co_await boost::cobalt::with (
            &ft,
            [](finalizer_test *)
            {
                throw std::runtime_error("42");
            },
            [](finalizer_test * ft, std::exception_ptr e)
            {
              return ft->exit(e);
            }));


    CHECK(ft.e != nullptr);
}

CO_TEST_CASE("cobalt")
{
    finalizer_test ft{co_await boost::cobalt::this_coro::executor};

    CHECK_THROWS(
            co_await boost::cobalt::with (
                    &ft,
                    [](finalizer_test * ft) -> boost::cobalt::promise<void>
                    {
                      throw std::runtime_error("42");
                      co_return;
                    },
                    [](finalizer_test * ft, std::exception_ptr e)
                    {
                      return ft->exit(e);
                    }));


    CHECK(ft.e != nullptr);
}

CO_TEST_CASE("sync-int")
{
  value_finalizer_test ft{co_await boost::cobalt::this_coro::executor};

  CHECK(23 ==
      co_await boost::cobalt::with (
          &ft,
          [](value_finalizer_test * ft)
          {
              return 23;
          },
          [](value_finalizer_test * ft, std::exception_ptr e)
          {
            return ft->exit(e);
          }));


  CHECK(ft.e == nullptr);
}

CO_TEST_CASE("cobalt-int")
{
  value_finalizer_test ft{co_await boost::cobalt::this_coro::executor};

  CHECK(
      42 ==
      co_await boost::cobalt::with (
          &ft,
          [](value_finalizer_test * ft) -> boost::cobalt::promise<int>
          {
            co_return 42;
          },
          [](value_finalizer_test * ft, std::exception_ptr e)
          {
            return ft->exit(e);
          }));

  CHECK(ft.e == nullptr);
}


TEST_SUITE_END();