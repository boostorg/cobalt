// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/cobalt/detail/handler.hpp>

#include "doctest.h"
#include "test.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/post.hpp>

using namespace boost;

struct dummy_promise
{
    using executor_type = boost::asio::any_io_executor;
    executor_type get_executor() const;

};

static_assert(boost::asio::detail::has_executor_type<dummy_promise>::value);


void test(boost::cobalt::completion_handler<> ch)
{
  boost::asio::post(std::move(ch));
}

TEST_SUITE_BEGIN("handler");

struct immediate_aw
{
  bool await_ready() {return false;}

  std::optional<std::tuple<>> result;
  cobalt::detail::completed_immediately_t completed_immediately;

  template<typename T>
  bool await_suspend(std::coroutine_handle<T> h)
  {
    cobalt::completion_handler<> ch{h, result,
#if !defined(BOOST_COBALT_NO_PMR)
                                   cobalt::this_thread::get_default_resource(),
#endif
                                   &completed_immediately};

    auto exec = asio::get_associated_immediate_executor(ch, h.promise().get_executor());
    completed_immediately = cobalt::detail::completed_immediately_t::initiating;
    asio::dispatch(exec, std::move(ch));

    CHECK(result);
    CHECK(completed_immediately == cobalt::detail::completed_immediately_t::yes);

    return completed_immediately != cobalt::detail::completed_immediately_t::yes;
  }

  void await_resume()
  {
    CHECK(completed_immediately);
    CHECK(result);
  }
};

#if !defined(BOOST_COBALT_USE_IO_CONTEXT)

struct non_immediate_aw
{
  bool await_ready() {return false;}

  std::optional<std::tuple<>> result;
  cobalt::detail::completed_immediately_t completed_immediately;
  cobalt::detail::sbo_resource res;

  template<typename T>
  bool await_suspend(std::coroutine_handle<T> h)
  {
    cobalt::completion_handler<> ch{h, result, &res, &completed_immediately};

    auto exec = asio::get_associated_immediate_executor(ch, h.promise().get_executor());
    asio::dispatch(exec,
                   asio::deferred(
                       [exec = h.promise().get_executor()]()
                       {
                         return asio::post(exec, asio::deferred);
                       }))(std::move(ch));

    CHECK(!result);
    CHECK(completed_immediately != cobalt::detail::completed_immediately_t::yes);

    return completed_immediately != cobalt::detail::completed_immediately_t::yes;
  }

  void await_resume()
  {
    CHECK(completed_immediately != cobalt::detail::completed_immediately_t::yes);
    CHECK(result);
  }
};




CO_TEST_CASE("immediate completion")
{
  co_await immediate_aw{};
  co_await non_immediate_aw{};
}

#endif

TEST_CASE("immediate_executor")
{
  asio::io_context ctx;
  cobalt::detail::completed_immediately_t completed_immediately = cobalt::detail::completed_immediately_t::initiating;
  cobalt::detail::completion_handler_noop_executor chh{ctx.get_executor(), &completed_immediately};
  bool called = false;

  SUBCASE("initiating")
  {
    asio::dispatch(chh, [&] { called = true; });
    CHECK(called);
    CHECK(completed_immediately == cobalt::detail::completed_immediately_t::initiating);
  }

  SUBCASE("maybe")
  {
    completed_immediately = cobalt::detail::completed_immediately_t::maybe;
    asio::dispatch(chh, [&] { called = true; completed_immediately = cobalt::detail::completed_immediately_t::yes; });
    CHECK(called);
    CHECK(completed_immediately == cobalt::detail::completed_immediately_t::yes);
  }


  SUBCASE("maybe-not")
  {
    completed_immediately = cobalt::detail::completed_immediately_t::maybe;
    asio::dispatch(chh, [&] { called = true; });
    CHECK(called);
    CHECK(completed_immediately == cobalt::detail::completed_immediately_t::initiating);
  }

  SUBCASE("no")
  {
    completed_immediately = cobalt::detail::completed_immediately_t::no;
    asio::dispatch(chh, [&] { called = true; });
    CHECK(!called);
    CHECK(completed_immediately == cobalt::detail::completed_immediately_t::no);
    CHECK(ctx.run() == 1u);
    CHECK(called);
  }

  SUBCASE("no")
  {
    completed_immediately = cobalt::detail::completed_immediately_t::no;
    asio::dispatch(chh, [&] { called = true; });
    CHECK(!called);
    CHECK(completed_immediately == cobalt::detail::completed_immediately_t::no);
    CHECK(ctx.run() == 1u);
    CHECK(called);
  }
}

TEST_SUITE_END();
