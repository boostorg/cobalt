//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_WRITE_HPP
#define BOOST_ASYNC_IO_IMPL_WRITE_HPP

#include "boost/async/io/write.hpp"
#include <boost/asio/write.hpp>
#include <boost/beast/core/buffer_ref.hpp>

namespace boost::async::io::detail
{


template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, const_buffer buffer, detail::write_handler rh)
{
 asio::async_write(pipe.implementation(), buffer, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, const_buffer buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), buffer, cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, flat_static_buffer_base &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, flat_static_buffer_base &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, static_buffer_base &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, static_buffer_base &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, flat_buffer &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, flat_buffer &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, multi_buffer &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, multi_buffer &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), beast::buffer_ref(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, std::string &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), asio::dynamic_buffer(buffer), std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, std::string &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), asio::dynamic_buffer(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, std::vector<unsigned char> &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), asio::dynamic_buffer(buffer), std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, std::vector<unsigned char> &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), asio::dynamic_buffer(buffer), cond, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, streambuf &buffer, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), buffer, std::move(rh));
}
template<typename WritableStream>
void write_impl<WritableStream>::call(WritableStream & pipe, streambuf &buffer, detail::completion_condition cond, detail::write_handler rh)
{
  asio::async_write(pipe.implementation(), buffer, cond, std::move(rh));
}

}

#endif //BOOST_ASYNC_IO_IMPL_WRITE_HPP
