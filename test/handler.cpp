// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/detail/handler.hpp>

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


void test(boost::async::completion_handler<> ch)
{
  boost::asio::post(std::move(ch));
}

TEST_SUITE_BEGIN("handler");

struct immediate_aw
{
  bool await_ready() {return false;}

  std::optional<std::tuple<>> result;
  async::detail::completed_immediately_t completed_immediately;

  template<typename T>
  bool await_suspend(std::coroutine_handle<T> h)
  {
    async::completion_handler<> ch{h, result,
#if !defined(BOOST_ASYNC_NO_PMR)
                                   async::this_thread::get_default_resource(),
#endif
                                   &completed_immediately};

    auto exec = asio::get_associated_immediate_executor(ch, h.promise().get_executor());
    completed_immediately = async::detail::completed_immediately_t::initiating;
    asio::dispatch(exec, std::move(ch));

    CHECK(result);
    CHECK(completed_immediately == async::detail::completed_immediately_t::yes);

    return completed_immediately != async::detail::completed_immediately_t::yes;
  }

  void await_resume()
  {
    CHECK(completed_immediately);
    CHECK(result);
  }
};


struct non_immediate_aw
{
  bool await_ready() {return false;}

  std::optional<std::tuple<>> result;
  async::detail::completed_immediately_t completed_immediately;

  template<typename T>
  bool await_suspend(std::coroutine_handle<T> h)
  {
    async::completion_handler<> ch{h, result,
#if !defined(BOOST_ASYNC_NO_PMR)
                                   async::this_thread::get_default_resource(),
#endif
                                   &completed_immediately};

    auto exec = asio::get_associated_immediate_executor(ch, h.promise().get_executor());
    asio::dispatch(exec,
                   asio::deferred(
                       [exec = h.promise().get_executor()]()
                       {
                         return asio::post(exec, asio::deferred);
                       }))(std::move(ch));

    CHECK(!result);
    CHECK(completed_immediately != async::detail::completed_immediately_t::yes);

    return completed_immediately != async::detail::completed_immediately_t::yes;
  }

  void await_resume()
  {
    CHECK(completed_immediately != async::detail::completed_immediately_t::yes);
    CHECK(result);
  }
};




CO_TEST_CASE("immediate completion")
{
  co_await immediate_aw{};
  co_await non_immediate_aw{};
}

TEST_SUITE_END();
