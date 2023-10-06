//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_STREAM_HPP
#define BOOST_ASYNC_IO_STREAM_HPP

#include <boost/async/op.hpp>

#include <boost/assert.hpp>
#include <boost/async/io/buffers/const_buffer_span.hpp>
#include <boost/async/io/buffers/mutable_buffer_span.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/result.hpp>

namespace boost::async::io
{




struct stream
{
  [[nodiscard]] virtual system::result<void> close() = 0;
  [[nodiscard]] virtual system::result<void> cancel() = 0;
  [[nodiscard]] virtual bool is_open() const = 0;

  virtual ~stream() =  default;
 protected:
  virtual void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) = 0;
  virtual void async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) = 0;
 private:
  struct read_some_op_;
  struct read_some_op_seq_;
  struct write_some_op_;
  struct write_some_op_seq_;
 public:
  BOOST_ASYNC_DECL [[nodiscard]] read_some_op_seq_ read_some(buffers::mutable_buffer_subspan buffers);
  BOOST_ASYNC_DECL [[nodiscard]] read_some_op_seq_ read_some(buffers::mutable_buffer_span    buffers);
  BOOST_ASYNC_DECL [[nodiscard]] read_some_op_     read_some(buffers::mutable_buffer         buffer);
  BOOST_ASYNC_DECL [[nodiscard]] write_some_op_seq_ write_some(buffers::const_buffer_subspan buffers);
  BOOST_ASYNC_DECL [[nodiscard]] write_some_op_seq_ write_some(buffers::const_buffer_span    buffers);
  BOOST_ASYNC_DECL [[nodiscard]] write_some_op_     write_some(buffers::const_buffer         buffer);
};

}

#include <boost/async/io/detail/stream.hpp>
#endif //BOOST_ASYNC_IO_STREAM_HPP
