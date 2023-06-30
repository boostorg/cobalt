//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/buffers/register.hpp>
#include <boost/async/this_thread.hpp>
#include <boost/asio/registered_buffer.hpp>

namespace boost::async::io::buffers
{

buffer_registration register_(buffers::mutable_buffer buffer)
{
  return register_(buffers::mutable_buffer_subspan{&buffer, 1u});

}
buffer_registration register_(buffers::mutable_buffer_span buffer)
{
  return register_(buffers::mutable_buffer_subspan{buffer});
}

buffer_registration register_(buffers::mutable_buffer_subspan buffer)
{
  return asio::register_buffers(this_thread::get_executor().context(),
                                buffer, this_thread::get_allocator());
}

}