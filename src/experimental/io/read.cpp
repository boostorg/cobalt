//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/read.hpp>
#include <boost/cobalt/experimental/composition.hpp>

namespace boost::cobalt::experimental::io
{

void read_all::initiate(completion_handler<boost::system::error_code, std::size_t>)
{
  std::size_t m = 0u;
  while (asio::buffer_size(step.buffer) > 0u && !co_await this_coro::cancelled)
  {
    auto [ec, n] = co_await step;
    m += n;
    if (ec)
      co_return {ec, m};

    step.buffer += n;
  }

  if (!!co_await this_coro::cancelled)
    co_return {asio::error::operation_aborted, m};
  else
    co_return {{}, m};
}

}