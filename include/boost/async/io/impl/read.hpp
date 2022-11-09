//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_READ_HPP
#define BOOST_ASYNC_IO_IMPL_READ_HPP

#include "boost/async/io/read.hpp"
#include <boost/asio/read.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{


template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, mutable_buffer buffer, detail::read_handler rh)
{
 asio::async_read(pipe.implementation(), buffer, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, mutable_buffer buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), buffer, cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, flat_static_buffer_base &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, static_buffer_base &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, static_buffer_base &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, flat_buffer &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, flat_buffer &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, multi_buffer &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, multi_buffer &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, std::string &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), asio::dynamic_buffer(buffer), std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, std::string &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), asio::dynamic_buffer(buffer), cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, std::vector<unsigned char> &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), asio::dynamic_buffer(buffer), std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), asio::dynamic_buffer(buffer), cond, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, streambuf &buffer, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), buffer, std::move(rh));
}
template<typename ReadableStream>
void read_impl<ReadableStream>::call(ReadableStream & pipe, streambuf &buffer, detail::completion_condition cond, detail::read_handler rh)
{
  asio::async_read(pipe.implementation(), buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_READ_HPP
