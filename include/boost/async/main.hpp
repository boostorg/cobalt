// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_MAIN_HPP
#define BOOST_ASYNC_MAIN_HPP

#include <boost/async/detail/concepts.hpp>
#include <boost/async/detail/async_operation.hpp>
#include <boost/async/this_coro.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>

#include <coroutine>
#include <optional>

namespace boost::async
{

namespace detail { struct main_promise; }
struct main;

}

auto co_main(int argc, char * argv[]) -> boost::async::main;

namespace boost::async
{

class main
{
  detail::main_promise * promise;
  main(detail::main_promise * promise) : promise(promise) {}
  friend auto ::co_main(int argc, char * argv[]) -> boost::async::main;
  friend struct detail::main_promise;
};

}



#include <boost/async/detail/main.hpp>

#endif //BOOST_ASYNC_MAIN_HPP
