//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_READ_AT_HPP
#define BOOST_ASYNC_IO_IMPL_READ_AT_HPP

#include "boost/async/io/read_at.hpp"
#include <boost/asio/read_at.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{

template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, mutable_buffer buffer, detail::read_at_handler rh)
{
 asio::async_read_at(pipe.implementation(), offset, buffer, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, mutable_buffer buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, buffer, cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, flat_static_buffer_base &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, static_buffer_base &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, static_buffer_base &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, flat_buffer &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, flat_buffer &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, multi_buffer &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, multi_buffer &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, std::string &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, asio::dynamic_buffer(buffer), std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, std::string &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, asio::dynamic_buffer(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, std::vector<unsigned char> &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, asio::dynamic_buffer(buffer), std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, asio::dynamic_buffer(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, streambuf &buffer, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, buffer, std::move(rh));
}
template<typename WritableStream>
void read_at_impl<WritableStream>::call(WritableStream & pipe, std::uint64_t offset, streambuf &buffer, detail::completion_condition cond, detail::read_at_handler rh)
{
  asio::async_read_at(pipe.implementation(), offset, buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_READ_AT_HPP
