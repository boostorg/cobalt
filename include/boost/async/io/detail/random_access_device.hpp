//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_RANDOM_ACCESS_DEVICE_HPP
#define BOOST_ASYNC_IO_DETAIL_RANDOM_ACCESS_DEVICE_HPP

#include <boost/async/io/random_access_device.hpp>
#include <boost/async/io/transfer_result.hpp>

namespace boost::async::io
{

struct random_access_device::read_some_at_op_ final : transfer_op
{
  read_some_at_op_(read_some_at_op_ && ) noexcept = default;

  void initiate(completion_handler<system::error_code, std::size_t> h) override
  {
    random_access_device_.async_read_some_at_impl_(offset_, {&buffer_, 1u}, std::move(h));
  }

  read_some_at_op_(random_access_device & rs, std::uint64_t offset, buffers::mutable_buffer buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::mutable_buffer buffer_;
};


struct random_access_device::read_some_at_op_seq_ final : transfer_op
{

  void initiate(completion_handler<system::error_code, std::size_t> h) override
  {
    random_access_device_.async_read_some_at_impl_(offset_, buffer_, std::move(h));
  }

  read_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::mutable_buffer_subspan buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}
  read_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::mutable_buffer_span buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::mutable_buffer_subspan buffer_;
};
struct random_access_device::write_some_at_op_ final : transfer_op
{
  write_some_at_op_(write_some_at_op_ && ) noexcept = default;
  void initiate(completion_handler<system::error_code, std::size_t> h) override
  {
    random_access_device_.async_write_some_at_impl_(offset_, {&buffer_, 1}, std::move(h));
  }

  write_some_at_op_(random_access_device & rs, std::uint64_t offset, buffers::const_buffer buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::const_buffer buffer_;
};


struct random_access_device::write_some_at_op_seq_ final : transfer_op
{
  void initiate(completion_handler<system::error_code, std::size_t> h) override
  {
    random_access_device_.async_write_some_at_impl_(offset_, buffer_, std::move(h));
  }

  write_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::const_buffer_subspan buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}
  write_some_at_op_seq_(random_access_device & rs, std::uint64_t offset, buffers::const_buffer_span buffer)
      : random_access_device_(rs), offset_(offset), buffer_(buffer) {}

 private:
  random_access_device & random_access_device_;
  std::uint64_t offset_;
  buffers::const_buffer_subspan buffer_;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_RANDOM_ACCESS_DEVICE_HPP
