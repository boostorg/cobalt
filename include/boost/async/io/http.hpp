//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BEAST_ASYNC_IO_HTTP_HPP
#define BEAST_ASYNC_IO_HTTP_HPP

#include <boost/beast/http/basic_parser.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/vector_body.hpp>

#include <boost/async/io/concepts.hpp>

namespace boost::async::io::http
{

using fields = beast::http::basic_fields<container::pmr::polymorphic_allocator<char>>;

template<class Body>
using request  = beast::http::request<Body, fields>;
template<class Body>
using response = beast::http::response<Body, fields>;

using request_header  = beast::http::request_header<fields>;
using response_header = beast::http::response_header<fields>;


/// An HTTP/1 parser for producing a request message.
template<class Body>
using request_parser = beast::http::parser<true, Body, container::pmr::polymorphic_allocator<char>>;

/// An HTTP/1 parser for producing a response message.
template<class Body>
using response_parser = beast::http::parser<false, Body, container::pmr::polymorphic_allocator<char>>;

/// A serializer for HTTP/1 requests
template<class Body>
using request_serializer = beast::http::serializer<true, Body, fields>;

/// A serializer for HTTP/1 responses
template<class Body>
using response_serializer = beast::http::serializer<false, Body, fields>;

using beast::http::buffer_body;
using beast::http::empty_body;
using beast::http::file_body;
using beast::http::string_body;
using vector_body = beast::http::vector_body<unsigned char>;

using beast::http::error;
using beast::http::field;
using beast::http::status;
using beast::http::status_class;
using beast::http::verb;
using beast::http::basic_parser;

namespace detail
{

void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<vector_body> & msg, concepts::read_handler h);

void async_read(concepts::read_stream & stream, static_buffer_base& buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<vector_body> & msg, concepts::read_handler h);

void async_read(concepts::read_stream & stream, flat_buffer& buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<vector_body> & msg, concepts::read_handler h);

void async_read(concepts::read_stream & stream, multi_buffer& buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<vector_body> & msg, concepts::read_handler h);

void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<vector_body> & msg, concepts::read_handler h);

void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<vector_body> & msg, concepts::read_handler h);

void async_read(concepts::read_stream & stream, streambuf & buffer, basic_parser<true>& parser,  concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer, basic_parser<false>& parser, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer, response<buffer_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer, response<empty_body>  & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer, response<file_body>   & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer, response<string_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<vector_body> & msg, concepts::read_handler h);
void async_read(concepts::read_stream & stream, streambuf & buffer, response<vector_body> & msg, concepts::read_handler h);


void async_read_header(concepts::read_stream & stream, flat_static_buffer_base& buffer,    basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, flat_static_buffer_base& buffer,    basic_parser< false >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, static_buffer_base& buffer,         basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, static_buffer_base& buffer,         basic_parser< false >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, flat_buffer& buffer,                basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, flat_buffer& buffer,                basic_parser< false >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, multi_buffer& buffer,               basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, multi_buffer& buffer,               basic_parser< false >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,                basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,                basic_parser< false >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer, basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer, basic_parser< false >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, streambuf& buffer,                  basic_parser< true >& parser, concepts::read_handler h);
void async_read_header(concepts::read_stream & stream, streambuf& buffer,                  basic_parser< false >& parser, concepts::read_handler h);

void async_read_some(concepts::read_stream & stream, flat_static_buffer_base& buffer,    basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, flat_static_buffer_base& buffer,    basic_parser< false >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, static_buffer_base& buffer,         basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, static_buffer_base& buffer,         basic_parser< false >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, flat_buffer& buffer,                basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, flat_buffer& buffer,                basic_parser< false >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, multi_buffer& buffer,               basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, multi_buffer& buffer,               basic_parser< false >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,                basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,                basic_parser< false >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer, basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer, basic_parser< false >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, streambuf& buffer,                  basic_parser< true >& parser, concepts::read_handler h);
void async_read_some(concepts::read_stream & stream, streambuf& buffer,                  basic_parser< false >& parser, concepts::read_handler h);

void async_write(concepts::write_stream & stream,  request_serializer< buffer_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response_serializer< buffer_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request_serializer< empty_body>  & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response_serializer< empty_body>  & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request_serializer< file_body>   & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response_serializer< file_body>   & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request_serializer< string_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response_serializer< string_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request_serializer< vector_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response_serializer< vector_body> & msg, concepts::write_handler h);

void async_write(concepts::write_stream & stream,  request<buffer_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response<buffer_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request<empty_body>  & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response<empty_body>  & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request<file_body>   & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response<file_body>   & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request<string_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response<string_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream,  request<vector_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, response<vector_body> & msg, concepts::write_handler h);

void async_write(concepts::write_stream & stream, const  request<buffer_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const response<buffer_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const  request<empty_body>  & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const response<empty_body>  & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const  request<string_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const response<string_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const  request<vector_body> & msg, concepts::write_handler h);
void async_write(concepts::write_stream & stream, const response<vector_body> & msg, concepts::write_handler h);


void async_write_header(concepts::read_stream & stream,  request_serializer< buffer_body> & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream, response_serializer< buffer_body> & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream,  request_serializer< empty_body>  & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream, response_serializer< empty_body>  & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream,  request_serializer< file_body>   & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream, response_serializer< file_body>   & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream,  request_serializer< string_body> & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream, response_serializer< string_body> & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream,  request_serializer< vector_body> & msg, concepts::write_handler h);
void async_write_header(concepts::read_stream & stream, response_serializer< vector_body> & msg, concepts::write_handler h);

void async_write_some(concepts::read_stream & stream,  request_serializer< buffer_body> & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream, response_serializer< buffer_body> & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream,  request_serializer< empty_body>  & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream, response_serializer< empty_body>  & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream,  request_serializer< file_body>   & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream, response_serializer< file_body>   & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream,  request_serializer< string_body> & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream, response_serializer< string_body> & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream,  request_serializer< vector_body> & msg, concepts::write_handler h);
void async_write_some(concepts::read_stream & stream, response_serializer< vector_body> & msg, concepts::write_handler h);

}
}

#endif //BEAST_ASYNC_IO_HTTP_HPP
