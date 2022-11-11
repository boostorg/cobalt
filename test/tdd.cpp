//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include <boost/asio.hpp>
#include <boost/async/io/concepts.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>

using namespace boost;

struct async_tester
{
  async_tester(asio::any_io_executor exec) : inp{exec, STDIN_FILENO} {}

  asio::posix::stream_descriptor inp;

  using executor_type = asio::any_io_executor;

  void async_read_some(
      asio::mutable_buffer buf,
      asio::any_completion_handler<void(system::error_code, std::size_t)> h)
  {
    inp.async_read_some(buf, std::move(h));
  }
};

/*
do_allocate: 288 8
asdasd
do_allocate: 0x612000000040 288 8
do_allocate: 288 8
12345678901234567890
do_allocate: 0x6120000001c0 288 8
do_allocate: 288 8
 */

auto res = boost::container::pmr::get_default_resource();

struct memory_res : boost::container::pmr::memory_resource
{
  memory_res() = default;
  void* do_allocate(std::size_t bytes, std::size_t alignment)
  {
    printf("do_allocate: %lu %lu\n", bytes, alignment);
    return res->allocate(bytes, alignment);
  }
  void do_deallocate(void* p, std::size_t bytes, std::size_t alignment)
  {
    printf("do_allocate: %p %lu %lu\n", p, bytes, alignment);
    return res->deallocate(p, bytes, alignment);
  }
  bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT
  {
    return res->is_equal(other);
  }
};

template<typename CompletionToken>
void test(async_tester & at, asio::mutable_buffer mb, CompletionToken && token)
{
  asio::async_initiate<CompletionToken, void(system::error_code, std::size_t)>(
      [](auto h, async_tester & at, asio::mutable_buffer mb)
      {
        at.async_read_some(mb, std::move(h));
      }, token, at, mb);
}

int main(int, char**)
{
  boost::asio::detail::prepared_buffers<boost::asio::const_buffer, 64ul> pb;
  printf("SIZE %d\n", pb.max_buffers);

  return 0;
}