//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BEAST_ASYNC_IO_IMPL_HTTP_IPP
#define BEAST_ASYNC_IO_IMPL_HTTP_IPP

#include <boost/async/io/http.hpp>
#include <boost/beast/core/buffer_ref.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>

namespace boost::async::io::http::detail
{

void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_static_buffer_base& buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read(concepts::read_stream & stream, static_buffer_base& buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, static_buffer_base& buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read(concepts::read_stream & stream, flat_buffer& buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, flat_buffer& buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read(concepts::read_stream & stream, multi_buffer& buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, multi_buffer& buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> & buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read(concepts::read_stream & stream, streambuf & buffer, basic_parser<true>& parser,  concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer, basic_parser<false>& parser, concepts::read_handler h) {beast::http::async_read(stream, buffer, parser, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer, response<buffer_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer, response<empty_body>  & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer, response<file_body>   & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer, response<string_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer,  request<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}
void async_read(concepts::read_stream & stream, streambuf & buffer, response<vector_body> & msg, concepts::read_handler h) {beast::http::async_read(stream, buffer, msg, std::move(h));}

void async_read_header(concepts::read_stream & stream, flat_static_buffer_base& buffer,                                                         basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, flat_static_buffer_base& buffer,                                                         basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, static_buffer_base& buffer,                                                              basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, static_buffer_base& buffer,                                                              basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, flat_buffer& buffer,                                                                     basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, flat_buffer& buffer,                                                                     basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, multi_buffer& buffer,                                                                    basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, multi_buffer& buffer,                                                                    basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer,       basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer,       basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, streambuf& buffer,                                                                       basic_parser< true >& parser,  concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }
void async_read_header(concepts::read_stream & stream, streambuf& buffer,                                                                       basic_parser< false >& parser, concepts::read_handler h) { return beast::http::async_read_header(stream, buffer, parser, std::move(h)); }

void async_read_some(concepts::read_stream & stream, flat_static_buffer_base& buffer,                                                         basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, flat_static_buffer_base& buffer,                                                         basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, static_buffer_base& buffer,                                                              basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, static_buffer_base& buffer,                                                              basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, flat_buffer& buffer,                                                                     basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, flat_buffer& buffer,                                                                     basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, multi_buffer& buffer,                                                                    basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, multi_buffer& buffer,                                                                    basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>& buffer, basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer,       basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>>& buffer,       basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, streambuf& buffer,                                                                       basic_parser< true >& parser,  concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }
void async_read_some(concepts::read_stream & stream, streambuf& buffer,                                                                       basic_parser< false >& parser, concepts::read_handler h)  { return beast::http::async_read_some(stream, buffer, parser, std::move(h)); }

void async_write(concepts::write_stream & stream,  request_serializer< buffer_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response_serializer< buffer_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request_serializer< empty_body>  & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response_serializer< empty_body>  & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request_serializer< file_body>   & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response_serializer< file_body>   & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request_serializer< string_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response_serializer< string_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request_serializer< vector_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response_serializer< vector_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }

void async_write(concepts::write_stream & stream,  request<buffer_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response<buffer_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request<empty_body>  & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response<empty_body>  & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request<file_body>   & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response<file_body>   & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request<string_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response<string_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream,  request<vector_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, response<vector_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }

void async_write(concepts::write_stream & stream, const  request<buffer_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const response<buffer_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const  request<empty_body>  & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const response<empty_body>  & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const  request<string_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const response<string_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const  request<vector_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }
void async_write(concepts::write_stream & stream, const response<vector_body> & msg, concepts::write_handler h) { beast::http::async_write(stream, msg, std::move(h)); }


void async_write_header(concepts::write_stream & stream,  request_serializer< buffer_body> & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream, response_serializer< buffer_body> & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream,  request_serializer< empty_body>  & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream, response_serializer< empty_body>  & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream,  request_serializer< file_body>   & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream, response_serializer< file_body>   & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream,  request_serializer< string_body> & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream, response_serializer< string_body> & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream,  request_serializer< vector_body> & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }
void async_write_header(concepts::write_stream & stream, response_serializer< vector_body> & msg, concepts::write_handler h)  { beast::http::async_write_header(stream, msg, std::move(h)); }


void async_write_some(concepts::write_stream & stream,  request_serializer< buffer_body> & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream, response_serializer< buffer_body> & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream,  request_serializer< empty_body>  & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream, response_serializer< empty_body>  & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream,  request_serializer< file_body>   & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream, response_serializer< file_body>   & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream,  request_serializer< string_body> & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream, response_serializer< string_body> & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream,  request_serializer< vector_body> & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }
void async_write_some(concepts::write_stream & stream, response_serializer< vector_body> & msg, concepts::write_handler h) { beast::http::async_write_some(stream, msg, std::move(h)); }

}

#endif //BEAST_ASYNC_IO_IMPL_HTTP_IPP
