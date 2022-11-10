//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_READ_IPP
#define BOOST_ASYNC_IO_IMPL_READ_IPP

#include "boost/async/io/read.hpp"
#include <boost/asio/read.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{


void read_impl(concepts::read_stream &stream, mutable_buffer buffer, detail::read_handler rh)
{
 asio::async_read(stream, buffer, std::move(rh));
}
void read_impl(concepts::read_stream &stream, mutable_buffer buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, buffer, cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, flat_static_buffer_base &buffer, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), std::move(rh));
}
void read_impl(concepts::read_stream &stream, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, static_buffer_base &buffer, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), std::move(rh));
}
void read_impl(concepts::read_stream &stream, static_buffer_base &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, flat_buffer &buffer, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), std::move(rh));
}
void read_impl(concepts::read_stream &stream, flat_buffer &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, multi_buffer &buffer, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), std::move(rh));
}
void read_impl(concepts::read_stream &stream, multi_buffer &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, std::string &buffer, detail::read_handler rh)
{
  asio::async_read(stream, asio::dynamic_buffer(buffer), std::move(rh));
}
void read_impl(concepts::read_stream &stream, std::string &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, asio::dynamic_buffer(buffer), cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, std::vector<unsigned char> &buffer, detail::read_handler rh)
{
  asio::async_read(stream, asio::dynamic_buffer(buffer), std::move(rh));
}
void read_impl(concepts::read_stream &stream, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, asio::dynamic_buffer(buffer), cond, std::move(rh));
}
void read_impl(concepts::read_stream &stream, streambuf &buffer, detail::read_handler rh)
{
  asio::async_read(stream, buffer, std::move(rh));
}
void read_impl(concepts::read_stream &stream, streambuf &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(stream, buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_READ_IPP
