//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_SSL_IPP
#define BOOST_ASYNC_IO_IMPL_SSL_IPP

#include <boost/async/io/ssl.hpp>
#include <boost/async/io/ip/tcp.hpp>

namespace boost::async::io
{

template<typename Stream> ssl<Stream>::~ssl() = default;
template<typename Stream> auto ssl<Stream>::get_executor() noexcept -> executor_type { return next_layer_.get_executor(); }

template<typename Stream> auto ssl<Stream>::native_handle()    -> native_handle_type     {return impl_.native_handle(); }
template<typename Stream> auto ssl<Stream>::next_layer() const -> const next_layer_type& {return next_layer_; }
template<typename Stream> auto ssl<Stream>::next_layer()       -> next_layer_type&       {return next_layer_; }
template<typename Stream>                    void ssl<Stream>::set_verify_mode(asio::ssl::verify_mode v)                                {        impl_.set_verify_mode(v);}
template<typename Stream> BOOST_ASIO_SYNC_OP_VOID ssl<Stream>::set_verify_mode(asio::ssl::verify_mode v, boost::system::error_code& ec) { return impl_.set_verify_mode(v, ec);}
template<typename Stream>                    void ssl<Stream>::set_verify_depth(int depth)                                {        impl_.set_verify_depth(depth);}
template<typename Stream> BOOST_ASIO_SYNC_OP_VOID ssl<Stream>::set_verify_depth(int depth, boost::system::error_code& ec) { return impl_.set_verify_depth(depth, ec);}

template<typename Stream>
void ssl<Stream>::async_handshake(handshake_type type, boost::async::completion_handler<system::error_code> h)
{
  return impl_.async_handshake(type, std::move(h));
}
template<typename Stream>
void ssl<Stream>::async_handshake(handshake_type type, asio::const_buffer buffer, boost::async::completion_handler<system::error_code, std::size_t> h)
{
  return impl_.async_handshake(type, buffer, std::move(h));
}

template<typename Stream>
void ssl<Stream>::async_shutdown(boost::async::completion_handler<system::error_code> h)
{
  return impl_.async_shutdown(std::move(h));
}
template<typename Stream> void ssl<Stream>:: async_write_some(any_const_buffer_range buffer, concepts::write_handler h) { return impl_.async_write_some(buffer, std::move(h)); }
template<typename Stream> void ssl<Stream>::async_write_some(const_buffer            buffer, concepts::write_handler h) { return impl_.async_write_some(buffer, std::move(h)); }
template<typename Stream> void ssl<Stream>::async_write_some(prepared_buffers        buffer, concepts::write_handler h) { return impl_.async_write_some(buffer, std::move(h)); }
template<typename Stream> void ssl<Stream>::async_read_some(asio::mutable_buffer                     buffer, concepts::read_handler h) { return impl_.async_read_some(buffer, std::move(h));}
template<typename Stream> void ssl<Stream>::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::read_handler h) { return impl_.async_read_some(buffer, std::move(h));}
template<typename Stream> void ssl<Stream>::async_read_some(multi_buffer::mutable_buffers_type       buffer, concepts::read_handler h) { return impl_.async_read_some(buffer, std::move(h));}

template struct ssl<ip::tcp::socket>;

}

#endif //BOOST_ASYNC_IO_IMPL_SSL_IPP
