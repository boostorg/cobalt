//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/detail/random_access_device.hpp>

namespace boost::async::io
{
auto random_access_device:: read_some_at(std::uint64_t offset, buffers::mutable_buffer_span    buffers)
  -> read_some_at_op_seq_
{
  return read_some_at_op_seq_{*this, offset, buffers};
}
auto random_access_device:: read_some_at(std::uint64_t offset, buffers::mutable_buffer_subspan buffers)
  -> read_some_at_op_seq_
{
  return read_some_at_op_seq_{*this, offset, buffers};
}
auto random_access_device:: read_some_at(std::uint64_t offset, buffers::mutable_buffer         buffer)
  -> read_some_at_op_
{
  return read_some_at_op_{*this, offset, buffer};
}
auto random_access_device::write_some_at(std::uint64_t offset, buffers::const_buffer_span    buffers)
  -> write_some_at_op_seq_
{
  return write_some_at_op_seq_{*this, offset, buffers};
}
auto random_access_device::write_some_at(std::uint64_t offset, buffers::const_buffer_subspan buffers)
  -> write_some_at_op_seq_
{
  return write_some_at_op_seq_{*this, offset, buffers};
}
auto random_access_device::write_some_at(std::uint64_t offset, buffers::const_buffer         buffer)
  -> write_some_at_op_
{
  return write_some_at_op_{*this, offset, buffer};
}


}