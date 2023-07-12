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

// avoid build-up
struct destroy_service final : asio::detail::service_base<destroy_service>
{

  using asio::detail::service_base<destroy_service>::service_base;

  void shutdown() override { to_delete.reset(nullptr); }
  std::unique_ptr<void, coro_deleter<void>> to_delete;
};

void self_destroy(std::coroutine_handle<void> h) noexcept
{
  auto & sd = asio::use_service<destroy_service>(this_thread::get_executor().context());

  if (sd.to_delete != nullptr)
  {
    // post is still in flight, no need to post again
    sd.to_delete.reset(h.address());
    return ;
  }
  sd.to_delete.reset(h.address());
  struct resetter
  {
    destroy_service & sd;
    ~resetter() {sd.to_delete.reset(nullptr);}
  };

  asio::post(this_thread::get_executor(),
              asio::bind_allocator(
                 this_thread::get_allocator(),
                 [&sd]() mutable {sd.to_delete.reset(nullptr);}));
}
#endif

}