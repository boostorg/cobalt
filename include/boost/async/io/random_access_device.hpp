//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_RANDOM_ACCESS_DEVICE_HPP
#define BOOST_ASYNC_IO_RANDOM_ACCESS_DEVICE_HPP

#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct random_access_device
{
  [[nodiscard]] virtual system::result<void> close() = 0;
  [[nodiscard]] virtual system::result<void> cancel() = 0;
  [[nodiscard]] virtual bool is_open() const = 0;

  virtual ~random_access_device() =  default;
 protected:
  virtual void async_read_some_at_impl_ (std::uint64_t offset, buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) = 0;
  virtual void async_write_some_at_impl_(std::uint64_t offset, buffers::  const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) = 0;
 private:
  struct read_some_at_op_;
  struct read_some_at_op_seq_;
  struct write_some_at_op_;
  struct write_some_at_op_seq_;
 public:
  BOOST_ASYNC_DECL [[nodiscard]] read_some_at_op_seq_   read_some_at(std::uint64_t offset, buffers::mutable_buffer_span    buffers);
  BOOST_ASYNC_DECL [[nodiscard]] read_some_at_op_seq_   read_some_at(std::uint64_t offset, buffers::mutable_buffer_subspan buffers);
  BOOST_ASYNC_DECL [[nodiscard]] read_some_at_op_       read_some_at(std::uint64_t offset, buffers::mutable_buffer         buffer);
  BOOST_ASYNC_DECL [[nodiscard]] write_some_at_op_seq_ write_some_at(std::uint64_t offset, buffers::const_buffer_span    buffers);
  BOOST_ASYNC_DECL [[nodiscard]] write_some_at_op_seq_ write_some_at(std::uint64_t offset, buffers::const_buffer_subspan buffers);
  BOOST_ASYNC_DECL [[nodiscard]] write_some_at_op_     write_some_at(std::uint64_t offset, buffers::const_buffer         buffer);
};

}

#include <boost/async/io/detail/random_access_device.hpp>
#endif //BOOST_ASYNC_IO_STREAM_HPP
