//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/composed.hpp>
#include <boost/cobalt/op.hpp>

#include <boost/test/unit_test.hpp>
#include "test.hpp"

using namespace boost;


BOOST_AUTO_TEST_SUITE(composed);

struct post_composed_op : cobalt::op<>
{

  post_composed_op() {}

  void initiate(cobalt::completion_handler<> complete)
  {
    co_await asio::post(cobalt::use_op);
  }
};


struct ec_composed_op : cobalt::op<system::error_code>
{
  ec_composed_op() {}

  void initiate(cobalt::completion_handler<system::error_code> complete)
  {
    co_return asio::error::already_open;
  }
};

struct ec_n_composed_op : cobalt::op<system::error_code, std::size_t>
{

  ec_n_composed_op() {}

  void initiate(cobalt::completion_handler<system::error_code, std::size_t> complete)
  {
    co_await asio::post(cobalt::use_op);
    co_return std::make_tuple(system::error_code{}, 2u);
  }
};


CO_TEST_CASE(composed_op)
{
  co_await post_composed_op();
  BOOST_CHECK_THROW(co_await ec_composed_op(), system::system_error);
  BOOST_CHECK(co_await ec_n_composed_op() == 2u);
}

BOOST_AUTO_TEST_SUITE_END();