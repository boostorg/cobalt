//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IMPL_TEST_IPP
#define BOOST_ASYNC_IMPL_TEST_IPP

#include <boost/async/test.hpp>
#include <boost/asio/detached.hpp>

namespace boost::async
{

std::coroutine_handle<detail::test_case_promise> test_case::await_suspend(std::coroutine_handle<void> h)
{
  promise->awaited_from.reset(h.address());
  return std::coroutine_handle<detail::test_case_promise>::from_promise(*promise);
}

void run(test_case&& tc)
{
  asio::io_context ctx;
  this_thread::set_executor(ctx.get_executor());
  async_run(ctx, std::move(tc), asio::detached);
  ctx.run();
}

test_case detail::test_case_promise::get_return_object()
{
  return async::test_case{this};
}

}

#endif //BOOST_ASYNC_IMPL_TEST_IPP
