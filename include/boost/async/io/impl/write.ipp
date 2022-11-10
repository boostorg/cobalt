//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_WRITE_IPP
#define BOOST_ASYNC_IO_IMPL_WRITE_IPP

#include "boost/async/io/write.hpp"
#include <boost/asio/write.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{


void write_impl(concepts::write_stream & stream, const_buffer buffer, detail::write_handler rh)
{
 asio::async_write(stream, buffer, std::move(rh));
}
void write_impl(concepts::write_stream & stream, const_buffer buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, buffer, cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, flat_static_buffer_base &buffer, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), std::move(rh));
}
void write_impl(concepts::write_stream & stream, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, static_buffer_base &buffer, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), std::move(rh));
}
void write_impl(concepts::write_stream & stream, static_buffer_base &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, flat_buffer &buffer, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), std::move(rh));
}
void write_impl(concepts::write_stream & stream, flat_buffer &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, multi_buffer &buffer, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), std::move(rh));
}
void write_impl(concepts::write_stream & stream, multi_buffer &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, beast::buffer_ref(buffer), cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, std::string &buffer, detail::write_handler rh)
{
  asio::async_write(stream, asio::dynamic_buffer(buffer), std::move(rh));
}
void write_impl(concepts::write_stream & stream, std::string &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, asio::dynamic_buffer(buffer), cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, std::vector<unsigned char> &buffer, detail::write_handler rh)
{
  asio::async_write(stream, asio::dynamic_buffer(buffer), std::move(rh));
}
void write_impl(concepts::write_stream & stream, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, asio::dynamic_buffer(buffer), cond, std::move(rh));
}
void write_impl(concepts::write_stream & stream, streambuf &buffer, detail::write_handler rh)
{
  asio::async_write(stream, buffer, std::move(rh));
}
void write_impl(concepts::write_stream & stream, streambuf &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(stream, buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_WRITE_IPP
