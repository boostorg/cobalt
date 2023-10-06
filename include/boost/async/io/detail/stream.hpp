//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DETAIL_STREAM_HPP
#define BOOST_ASYNC_IO_DETAIL_STREAM_HPP

#include <boost/async/io/transfer_result.hpp>
#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct stream::read_some_op_ final : transfer_op
{
  read_some_op_(read_some_op_ && ) noexcept = default;
  read_some_op_(stream & rs, buffers::mutable_buffer buffer)
      : rstream_(rs), buffer_(buffer) {}

  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, std::size_t> h) override;
 private:
  stream & rstream_;
  buffers::mutable_buffer buffer_;
};


struct stream::read_some_op_seq_ final : transfer_op
{
  read_some_op_seq_(stream & rs, buffers::mutable_buffer_subspan buffer)
      : rstream_(rs), buffer_(buffer) {}
  read_some_op_seq_(stream & rs, buffers::mutable_buffer_span buffer)
      : rstream_(rs), buffer_(buffer) {}

  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, std::size_t> h) override;
 private:
  stream & rstream_;
  buffers::mutable_buffer_subspan buffer_;
};

struct stream::write_some_op_ final : transfer_op
{
  write_some_op_(write_some_op_ && ) noexcept = default;
  write_some_op_(stream & rs, buffers::const_buffer buffer)
      : rstream_(rs), buffer_(buffer) {}

  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, std::size_t> h) override;

 private:
  stream & rstream_;
  buffers::const_buffer buffer_;
};


struct stream::write_some_op_seq_ final : transfer_op
{
  write_some_op_seq_(stream & rs, buffers::const_buffer_subspan buffer)
      : rstream_(rs), buffer_(buffer) {}
  write_some_op_seq_(stream & rs, buffers::const_buffer_span buffer)
      : rstream_(rs), buffer_(buffer) {}

  BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, std::size_t> h) override;

 private:
  stream & rstream_;
  buffers::const_buffer_subspan buffer_;
};

}

#endif //BOOST_ASYNC_IO_DETAIL_STREAM_HPP
