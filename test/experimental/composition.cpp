//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/composition.hpp>

#include "../test.hpp"

using namespace boost::cobalt;

struct dummy_op : op<boost::system::error_code, std::size_t>
{
  void initiate(completion_handler<boost::system::error_code, std::size_t>)
  {
    std::tuple<> t = co_await boost::asio::post(co_await this_coro::executor, boost::asio::deferred);
    t = co_await boost::asio::post(co_await this_coro::executor, use_op);
    co_return {boost::asio::error::no_such_device, 42u};
  }

};





CO_TEST_CASE(composition)
{
  co_await boost::cobalt::as_tuple(dummy_op{});
  co_return ;
}