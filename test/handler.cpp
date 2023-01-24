// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "boost/async/detail/handler.hpp"

#include "doctest.h"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/post.hpp>

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

TEST_CASE("testing")
{
  int res = 0;
  boost::async::completion_handler<int> ch{[&](int i ){ res = i;}};

  CHECK(res == 0);
  ch(42);
  CHECK(res == 42);

}