// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/cobalt/leaf.hpp>
#include <boost/cobalt/promise.hpp>
#include <boost/leaf/result.hpp>

#include "doctest.h"
#include "test.hpp"

using namespace boost;


TEST_SUITE_BEGIN("leaf");

CO_TEST_CASE("try_catch")
{
  CHECK(co_await cobalt::try_catch(
      []() -> cobalt::promise<int>
      {
        throw std::runtime_error("TestException");
        co_return 42;
      }(),
      [](std::runtime_error & re)
      {
        CHECK(re.what() == std::string("TestException"));
        return -1;
      },
      [](std::exception &)
      {
          CHECK(false);
          return -2;
      }) == -1);

  CHECK(co_await cobalt::try_catch(
      []() -> cobalt::promise<int>
      {
        co_return 42;
      }(),
      [](std::runtime_error &)
      {
        CHECK(false);
        return -1;
      },
      [](std::exception &)
      {
        CHECK(false);
        return -2;
      }) == 42);
}


CO_TEST_CASE("try_handle_all")
{
  CHECK(co_await cobalt::try_handle_all(
      []() -> cobalt::promise<leaf::result<int>>
      {
        throw std::runtime_error("TestException");
        co_return 42;
      }(),
      [](const std::runtime_error & re)
      {
        CHECK(re.what() == std::string("TestException"));
        return -1;
      },
      [](const std::exception &)
      {
        CHECK(false);
        return -2;
      },
      []
      {
        CHECK(false);
        return -3;
      }) == -1);

  CHECK(co_await cobalt::try_handle_all(
      []() -> cobalt::promise<leaf::result<int>>
      {
        co_return 42;
      }(),
      [](const std::runtime_error &)
      {
        CHECK(false);
        return -1;
      },
      [](const std::exception &)
      {
        CHECK(false);
        return -2;
      },
      []
      {
        CHECK(false);
        return -3;
      }) == 42);
}


CO_TEST_CASE("try_handle_all")
{
  CHECK((co_await cobalt::try_handle_some(
      []() -> cobalt::promise<leaf::result<int>>
      {
        throw std::runtime_error("TestException");
        co_return 42;
      }(),
      [](const std::runtime_error & re)
      {
        CHECK(re.what() == std::string("TestException"));
        return -1;
      },
      [](const std::exception &)
      {
        CHECK(false);
        return -2;
      },
      []
      {
        CHECK(false);
        return -3;
      })).value() == -1);

  CHECK((co_await cobalt::try_handle_some(
      []() -> cobalt::promise<leaf::result<int>>
      {
        co_return 42;
      }(),
      [](const std::runtime_error &)
      {
        CHECK(false);
        return -1;
      },
      [](const std::exception &)
      {
        CHECK(false);
        return -2;
      },
      []
      {
        CHECK(false);
        return -3;
      })).value() == 42);
}


TEST_SUITE_END();