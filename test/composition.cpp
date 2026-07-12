//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/composition.hpp>
#include <boost/cobalt/race.hpp>


#include "test.hpp"

using namespace boost::cobalt;

struct dummy_op final : op<boost::system::error_code, int>
{
  void initiate(completion_handler<boost::system::error_code, int>)
  {
    std::tuple<> t = co_await boost::asio::post(co_await this_coro::executor, boost::asio::deferred);
    t = co_await boost::asio::post(co_await this_coro::executor, use_op);
    co_return {boost::asio::error::no_such_device, 42};
  }
};

struct wrapping_dummy_op final : op<boost::system::error_code, int>
{
  void initiate(completion_handler<boost::system::error_code, int>)
  {
    co_return co_await dummy_op{};
  }
};


struct race_dummy_op final : op<boost::system::error_code, int>
{
  void initiate(completion_handler<boost::system::error_code, int>)
  {
    auto t = co_await left_race(dummy_op{}, dummy_op{});
    co_return {};
  }
};

CO_TEST_CASE(composition)
{
  auto [ec, n] = co_await boost::cobalt::as_tuple(dummy_op{});
  BOOST_CHECK(ec == boost::asio::error::no_such_device);
  BOOST_CHECK(n == 42);
  co_return ;
}

CO_TEST_CASE(wrapped_composition)
{
  auto [ec, n] = co_await boost::cobalt::as_tuple(wrapping_dummy_op{});
  BOOST_CHECK(ec == boost::asio::error::no_such_device);
  BOOST_CHECK(n == 42);
  co_return ;
}

CO_TEST_CASE(composition_race)
{
  auto [ec, n] = co_await boost::cobalt::as_tuple(dummy_op{});
  BOOST_CHECK(ec == boost::asio::error::no_such_device);
  BOOST_CHECK(n == 42);
  co_return ;
}


