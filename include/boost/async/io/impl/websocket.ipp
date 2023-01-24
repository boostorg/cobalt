//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_WEBSOCKET_IPP
#define BOOST_ASYNC_IO_IMPL_WEBSOCKET_IPP

#include <boost/async/io/websocket.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace boost::async::io
{

template<typename Stream> websocket<Stream>::~websocket() = default;
template<typename Stream> auto websocket<Stream>::get_executor() noexcept -> executor_type { return impl_.get_executor(); }
template<typename Stream> auto websocket<Stream>::next_layer() noexcept       -> next_layer_type&       { return next_layer_; }
template<typename Stream> auto websocket<Stream>::next_layer() const noexcept -> next_layer_type const& { return next_layer_; }

template<typename Stream> bool websocket<Stream>::is_open()    const noexcept {return impl_.is_open();}
template<typename Stream> bool websocket<Stream>::got_binary() const noexcept {return impl_.got_binary();}
template<typename Stream> bool websocket<Stream>::is_message_done() const noexcept {return impl_.is_message_done();}

template<typename Stream> beast::websocket::close_reason const& websocket<Stream>::reason() const noexcept {return impl_.reason(); }

template<typename Stream> std::size_t websocket<Stream>::read_size_hint( std::size_t initial_size)        const {return impl_.read_size_hint(initial_size);}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint()                                 const {return impl_.read_size_hint();}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(flat_static_buffer_base & buffer) const {return impl_.read_size_hint(buffer);}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(static_buffer_base & buffer)      const {return impl_.read_size_hint(buffer);}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(flat_buffer & buffer)             const {return impl_.read_size_hint(buffer);}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(multi_buffer & buffer)            const {return impl_.read_size_hint(buffer);}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(streambuf & buffer)               const {return impl_.read_size_hint(buffer);}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(std::string  & buf)  const
{
  auto buffer = asio::dynamic_buffer(buf);
  return impl_.read_size_hint(buffer);
}
template<typename Stream> std::size_t websocket<Stream>::read_size_hint(std::vector<unsigned char> & buf) const
{
  auto buffer = asio::dynamic_buffer(buf);
  return impl_.read_size_hint(buffer);
}


template<typename Stream> void websocket<Stream>::set_option(decorator opt)                                 {impl_.set_option(std::move(opt));}
template<typename Stream> void websocket<Stream>::set_option(timeout const& opt)                            {impl_.set_option(opt);}
template<typename Stream> void websocket<Stream>::get_option(timeout& opt)                                  {impl_.get_option(opt);}
template<typename Stream> void websocket<Stream>::set_option(beast::websocket::permessage_deflate const& o) {impl_.set_option(o);}
template<typename Stream> void websocket<Stream>::get_option(beast::websocket::permessage_deflate& o)       {impl_.get_option(o);}

template<typename Stream> void websocket<Stream>::auto_fragment(bool value) {       impl_.auto_fragment(value);}
template<typename Stream> bool websocket<Stream>::auto_fragment() const     {return impl_.auto_fragment();}

template<typename Stream> void websocket<Stream>::binary(bool value) {       impl_.binary(value);}
template<typename Stream> bool websocket<Stream>::binary() const     {return impl_.binary();}
template<typename Stream> void websocket<Stream>::control_callback(std::function<void(beast::websocket::frame_type, beast::string_view)> cb) {return impl_.control_callback(std::move(cb));}
template<typename Stream> void websocket<Stream>::control_callback() {impl_.control_callback();}

template<typename Stream> void        websocket<Stream>::read_message_max(std::size_t amount) { impl_.read_message_max(amount);}
template<typename Stream> std::size_t websocket<Stream>::read_message_max() const       {return impl_.read_message_max();}
template<typename Stream> void websocket<Stream>::secure_prng(bool value) {return impl_.secure_prng(value);}
template<typename Stream> void websocket<Stream>::write_buffer_bytes(std::size_t amount)   { impl_.write_buffer_bytes(amount);}
template<typename Stream> std::size_t websocket<Stream>::write_buffer_bytes() const { return impl_.write_buffer_bytes();}
template<typename Stream> void websocket<Stream>::text(bool value) {    impl_.text(value);}
template<typename Stream> bool websocket<Stream>::text() const { return impl_.text();}
template<typename Stream> void websocket<Stream>::compress(bool value) {    impl_.compress(value);}
template<typename Stream> bool websocket<Stream>::compress() const { return impl_.compress();}

template<typename Stream>
void websocket<Stream>::async_handshake(
    beast::string_view host,
    beast::string_view target,
    boost::async::completion_handler<system::error_code> h)
{
  impl_.async_handshake(host, target, std::move(h));
}

template<typename Stream>
void websocket<Stream>::async_handshake(response_type& res,
    string_view host,
    string_view target,boost::async::completion_handler<system::error_code> h)
{
  impl_.async_handshake(res, host, target, std::move(h));
}

template<typename Stream> void websocket<Stream>::async_accept(boost::async::completion_handler<system::error_code> h)                                               {impl_.async_accept(std::move(h));}
template<typename Stream> void websocket<Stream>::async_accept(const const_buffer & buffers, boost::async::completion_handler<system::error_code> h)                 {impl_.async_accept(buffers, std::move(h));}
template<typename Stream> void websocket<Stream>::async_accept(const http::request<http::buffer_body> &msg,  boost::async::completion_handler<system::error_code> h) {impl_.async_accept(msg, std::move(h));}
template<typename Stream> void websocket<Stream>::async_accept(const http::request<http::empty_body>  &msg,  boost::async::completion_handler<system::error_code> h) {impl_.async_accept(msg, std::move(h));}
template<typename Stream> void websocket<Stream>::async_accept(const http::request<http::file_body>   &msg,  boost::async::completion_handler<system::error_code> h) {impl_.async_accept(msg, std::move(h));}
template<typename Stream> void websocket<Stream>::async_accept(const http::request<http::string_body> &msg,  boost::async::completion_handler<system::error_code> h) {impl_.async_accept(msg, std::move(h));}
template<typename Stream> void websocket<Stream>::async_accept(const http::request<http::vector_body> &msg,  boost::async::completion_handler<system::error_code> h) {impl_.async_accept(msg, std::move(h));}

template<typename Stream> void websocket<Stream>::async_close(beast::websocket::close_reason const& cr,   boost::async::completion_handler<system::error_code> h) {impl_.async_close(cr, std::move(h));}
template<typename Stream> void websocket<Stream>::async_ping( beast::websocket::ping_data const& payload, boost::async::completion_handler<system::error_code> h) {impl_.async_ping(payload, std::move(h));}
template<typename Stream> void websocket<Stream>::async_pong( beast::websocket::ping_data const& payload, boost::async::completion_handler<system::error_code> h) {impl_.async_pong(payload, std::move(h));}
template<typename Stream> void websocket<Stream>::async_read(flat_static_buffer_base &buffer,    concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}
template<typename Stream> void websocket<Stream>::async_read(static_buffer_base &buffer,         concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}
template<typename Stream> void websocket<Stream>::async_read(flat_buffer &buffer,                concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}
template<typename Stream> void websocket<Stream>::async_read(multi_buffer &buffer,               concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}
template<typename Stream> void websocket<Stream>::async_read(streambuf &buffer,                  concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}

template<typename Stream>
void websocket<Stream>::async_read(asio::dynamic_string_buffer<char,  std::char_traits<char>, std::allocator<char>> &buffer,
                                   concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}

template<typename Stream>
void websocket<Stream>::async_read(asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> &buffer,
                                   concepts::read_handler rh) {impl_.async_read(buffer, std::move(rh));}

template<typename Stream> void websocket<Stream>::async_read_some(asio::mutable_buffer buffer,                     concepts::read_handler h) {impl_.async_read_some(buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::read_handler h) {impl_.async_read_some(buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::read_handler h) {impl_.async_read_some(buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_write(any_const_buffer_range buffer, concepts::write_handler h) {impl_.async_write(buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_write(const_buffer           buffer, concepts::write_handler h) {impl_.async_write(buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_write(prepared_buffers       buffer, concepts::write_handler h) {impl_.async_write(buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_write_some(any_const_buffer_range buffer, concepts::write_handler h) {impl_.async_write_some(true, buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_write_some(const_buffer           buffer, concepts::write_handler h) {impl_.async_write_some(true, buffer, std::move(h));}
template<typename Stream> void websocket<Stream>::async_write_some(prepared_buffers       buffer, concepts::write_handler h) {impl_.async_write_some(true, buffer, std::move(h));}

template struct websocket<    ip::tcp::socket>;
template struct websocket<ssl<ip::tcp::socket>>;

}

#endif //BOOST_ASYNC_IO_IMPL_WEBSOCKET_IPP
