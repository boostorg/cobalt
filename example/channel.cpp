// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/async/channel.hpp>
#include <boost/async/main.hpp>
#include <boost/async/promise.hpp>

#include <iostream>

namespace async = boost::async;

// tag::channel_example[]
async::promise<void> producer(async::channel<int> & chan)
{
  for (int i = 0; i < 4; i++)
    co_await chan.write(i);

  chan.close();
}

async::main co_main(int argc, char * argv[])
{
  async::channel<int> c;

  auto p = producer(c);
  while (c.is_open())
    std::cout << co_await c.read() << std::endl;

  co_await p;
  co_return 0;
}
// end::channel_example[]
