//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_WRITE_AT_IPP
#define BOOST_ASYNC_IO_IMPL_WRITE_AT_IPP

#include "boost/async/io/write_at.hpp"
#include <boost/asio/write_at.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{

void write_at_impl(concepts::random_access_write_device & stream, std::uint64_t offset, const_buffer buffer, detail::write_at_handler rh)
{
 asio::async_write_at(stream, offset, buffer, std::move(rh));
}
void write_at_impl(concepts::random_access_write_device & stream, std::uint64_t offset, const_buffer buffer, detail::completion_condition cond, detail::write_at_handler rh)
{
  asio::async_write_at(stream, offset, buffer, cond, std::move(rh));
}
void write_at_impl(concepts::random_access_write_device & stream, std::uint64_t offset, streambuf &buffer, detail::write_at_handler rh)
{
  asio::async_write_at(stream, offset, buffer, std::move(rh));
}
void write_at_impl(concepts::random_access_write_device & stream, std::uint64_t offset, streambuf &buffer, detail::completion_condition cond, detail::write_at_handler rh)
{
  asio::async_write_at(stream, offset, buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_WRITE_AT_IPP
