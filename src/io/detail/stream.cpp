//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/detail/stream.hpp>

namespace boost::async::io
{

auto stream:: read_some(buffers::mutable_buffer_subspan buffers) -> read_some_op_seq_ { return read_some_op_seq_{*this, buffers};}
auto stream:: read_some(buffers::mutable_buffer_span    buffers) -> read_some_op_seq_ { return read_some_op_seq_{*this, buffers};}
auto stream:: read_some(buffers::mutable_buffer         buffer)  -> read_some_op_ { return read_some_op_{*this, buffer};}
auto stream::write_some(buffers::const_buffer_subspan buffers)   -> write_some_op_seq_ { return write_some_op_seq_{*this, buffers};}
auto stream::write_some(buffers::const_buffer_span    buffers)   -> write_some_op_seq_ { return write_some_op_seq_{*this, buffers};}
auto stream::write_some(buffers::const_buffer         buffer)    -> write_some_op_ { return write_some_op_{*this, buffer};}

void stream::read_some_op_::initiate(completion_handler<system::error_code, std::size_t> h)
{
  rstream_.async_read_some_impl_({&buffer_, 1u}, std::move(h));
}

void stream::read_some_op_seq_::initiate(completion_handler<system::error_code, std::size_t> h)
{
  rstream_.async_read_some_impl_(buffer_, std::move(h));
}

void stream::write_some_op_::initiate(completion_handler<system::error_code, std::size_t> h)
{
  rstream_.async_write_some_impl_({&buffer_, 1u}, std::move(h));
}

void stream::write_some_op_seq_::initiate(completion_handler<system::error_code, std::size_t> h)
{
  rstream_.async_write_some_impl_(buffer_, std::move(h));
}



}