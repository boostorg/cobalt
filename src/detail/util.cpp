// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/detail/util.hpp>
#include <boost/async/this_thread.hpp>

#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/post.hpp>

namespace boost::async::detail
{

#if BOOST_ASYNC_NO_SELF_DELETE

void self_destroy(std::coroutine_handle<void> h, const async::executor & exec) noexcept
{
  asio::post(exec,
              asio::bind_allocator(
                 this_thread::get_allocator(),
                 [del=std::unique_ptr<void, coro_deleter<void>>(h.address())]() mutable
                 {
                 }));
}
#endif

}