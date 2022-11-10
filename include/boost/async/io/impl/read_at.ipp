//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_READ_AT_IPP
#define BOOST_ASYNC_IO_IMPL_READ_AT_IPP

#include "boost/async/io/read_at.hpp"
#include <boost/asio/read_at.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{

void read_at_impl(concepts::random_access_read_device & stream, std::uint64_t offset, mutable_buffer buffer, detail::read_at_handler rh)
{
 asio::async_read_at(stream, offset, buffer, std::move(rh));
}
void read_at_impl(concepts::random_access_read_device & stream, std::uint64_t offset, mutable_buffer buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(stream, offset, buffer, cond, std::move(rh));
}
void read_at_impl(concepts::random_access_read_device & stream, std::uint64_t offset, streambuf &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(stream, offset, buffer, std::move(rh));
}
void read_at_impl(concepts::random_access_read_device & stream, std::uint64_t offset, streambuf &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(stream, offset, buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_READ_AT_IPP
