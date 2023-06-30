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

struct [[nodiscard]] transfer_result
{
  system::error_code error;
  std::size_t transferred;

  using value_type = std::size_t;
  using error_type = system::error_code;

  // queries
  constexpr bool has_value() const noexcept { return transferred != 0; }
  constexpr bool has_error() const noexcept { return error.failed(); }
  constexpr explicit operator bool() const noexcept { return has_value() || !has_error(); }
  constexpr std::size_t value( boost::source_location const& loc = BOOST_CURRENT_LOCATION ) const noexcept
  {
    if (!has_value() || has_error())
      throw_exception_from_error(error, loc);
    return transferred;
  }
  constexpr std::size_t operator*() const noexcept { BOOST_ASSERT(has_value()); return transferred; }

  bool operator==(const system::error_code &ec) const { return error == ec;}
  bool operator!=(const system::error_code &ec) const { return error != ec;}

};

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
