//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//  
#ifndef BOOST_ASYNC_IO_LOCAL_IMPL_STREAM_IPP
#define BOOST_ASYNC_IO_LOCAL_IMPL_STREAM_IPP  


#include <boost/async/io/local/stream.hpp>  

namespace boost::async::io::local
{

stream::socket::socket(implementation_type&& impl) : impl_(std::move(impl)) {}
stream::socket& stream::socket::operator=(implementation_type&& impl)
{
  impl_ = std::move(impl);
  return *this;
}
stream::socket::socket(const executor_type& ex)                                                                           : impl_(ex) {}
stream::socket::socket(const executor_type& ex, const protocol_type& protocol)                                            : impl_(ex, protocol) {}
stream::socket::socket(const executor_type& ex, const endpoint_type& endpoint)                                            : impl_(ex, endpoint) {}
stream::socket::socket(const executor_type& ex, const protocol_type& protocol, const native_handle_type& native_socket)   : impl_(ex, protocol, native_socket) {}
stream::socket::socket(asio::io_context& context)                                                                         : impl_(context) {}
stream::socket::socket(asio::io_context& context, const protocol_type& protocol)                                          : impl_(context, protocol) {}
stream::socket::socket(asio::io_context& context, const endpoint_type& endpoint)                                          : impl_(context, endpoint) {}
stream::socket::socket(asio::io_context& context, const protocol_type& protocol, const native_handle_type& native_socket) : impl_(context, protocol, native_socket) {}

stream::socket::socket(socket&& other) noexcept = default;
stream::socket& stream::socket::operator=(socket&& other) = default;
stream::socket::~socket() = default;

auto stream::socket::get_executor() noexcept -> executor_type         { return impl_.get_executor(); }
auto stream::socket::lowest_layer() -> lowest_layer_type&             { return impl_; }
auto stream::socket::lowest_layer() const -> const lowest_layer_type& { return impl_; }
  
template <typename SettableSocketOption>
void stream::socket::set_option(const SettableSocketOption& option)
{
  impl_.set_option(option);
}

template <typename SettableSocketOption>
BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const SettableSocketOption& option, boost::system::error_code& ec)
{
  return impl_.set_option(option, ec);
}
template <typename GettableSocketOption>
void stream::socket::get_option(GettableSocketOption& option) const
{
  impl_.get_option(option);
}
template <typename GettableSocketOption>
BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(GettableSocketOption& option, boost::system::error_code& ec) const
{
  return impl_.get_option(option, ec);
}

template void stream::socket::set_option(const asio::socket_base::debug &);
template void stream::socket::set_option(const asio::socket_base::do_not_route &);
template void stream::socket::set_option(const asio::socket_base::enable_connection_aborted &);
template void stream::socket::set_option(const asio::socket_base::keep_alive &);
template void stream::socket::set_option(const asio::socket_base::linger &);
template void stream::socket::set_option(const asio::socket_base::out_of_band_inline &);
template void stream::socket::set_option(const asio::socket_base::receive_buffer_size &);
template void stream::socket::set_option(const asio::socket_base::receive_low_watermark &);
template void stream::socket::set_option(const asio::socket_base::reuse_address &);
template void stream::socket::set_option(const asio::socket_base::send_buffer_size &);
template void stream::socket::set_option(const asio::socket_base::send_low_watermark &);

template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::debug &,                     system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::do_not_route &,              system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::enable_connection_aborted &, system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::keep_alive &,                system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::linger &,                    system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::out_of_band_inline &,        system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::receive_buffer_size &,       system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::receive_low_watermark &,     system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::reuse_address &,             system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::send_buffer_size &,          system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::set_option(const asio::socket_base::send_low_watermark &,        system::error_code &);

template void stream::socket::get_option(boost::asio::socket_base::debug &) const;
template void stream::socket::get_option(boost::asio::socket_base::do_not_route &) const;
template void stream::socket::get_option(boost::asio::socket_base::enable_connection_aborted &) const;
template void stream::socket::get_option(boost::asio::socket_base::keep_alive &) const;
template void stream::socket::get_option(boost::asio::socket_base::linger &) const;
template void stream::socket::get_option(boost::asio::socket_base::out_of_band_inline &) const;
template void stream::socket::get_option(boost::asio::socket_base::receive_buffer_size &) const;
template void stream::socket::get_option(boost::asio::socket_base::receive_low_watermark &) const;
template void stream::socket::get_option(boost::asio::socket_base::reuse_address &) const;
template void stream::socket::get_option(boost::asio::socket_base::send_buffer_size &) const;
template void stream::socket::get_option(boost::asio::socket_base::send_low_watermark &) const;

template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::debug &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::do_not_route &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::enable_connection_aborted &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::keep_alive &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::linger &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::out_of_band_inline &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::receive_buffer_size &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::receive_low_watermark &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::reuse_address &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::send_buffer_size &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID stream::socket::get_option(boost::asio::socket_base::send_low_watermark &, system::error_code &) const;


template <typename IoControlCommand>                    void stream::socket::io_control(IoControlCommand& command)                                {        impl_.io_control(command);     }
template <typename IoControlCommand> BOOST_ASIO_SYNC_OP_VOID stream::socket::io_control(IoControlCommand& command, boost::system::error_code& ec) { return impl_.io_control(command, ec); }

template                    void stream::socket::io_control(boost::asio::socket_base::bytes_readable &);
template BOOST_ASIO_SYNC_OP_VOID stream::socket::io_control(boost::asio::socket_base::bytes_readable &, system::error_code &);


bool stream::socket::non_blocking() const                                   { return impl_.non_blocking(); }
void stream::socket::non_blocking(bool mode)                                {        impl_.non_blocking(mode); }
void stream::socket::non_blocking(bool mode, boost::system::error_code& ec) {        impl_.non_blocking(mode, ec); }

bool stream::socket::native_non_blocking() const                                   { return impl_.native_non_blocking(); }
void stream::socket::native_non_blocking(bool mode)                                {        impl_.native_non_blocking(mode); }
void stream::socket::native_non_blocking(bool mode, boost::system::error_code& ec) {        impl_.native_non_blocking(mode, ec); }
    
void stream::socket::shutdown(shutdown_type what)                                { impl_.shutdown(what);};
void stream::socket::shutdown(shutdown_type what, boost::system::error_code& ec) { impl_.shutdown(what, ec);};
auto stream::socket::local_endpoint() const                              -> endpoint_type { return impl_.local_endpoint(); }
auto stream::socket::local_endpoint(boost::system::error_code& ec) const -> endpoint_type { return impl_.local_endpoint(ec); }
  
auto stream::socket::remote_endpoint() const                              -> endpoint_type { return impl_.remote_endpoint(); }
auto stream::socket::remote_endpoint(boost::system::error_code& ec) const -> endpoint_type { return impl_.remote_endpoint(ec); }
                   void stream::socket::open(const protocol_type& protocol)                                {        impl_.open(protocol);};
BOOST_ASIO_SYNC_OP_VOID stream::socket::open(const protocol_type& protocol, boost::system::error_code& ec) { return impl_.open(protocol, ec);};
                   void stream::socket::assign(const protocol_type& protocol, const native_handle_type& native_socket)                                 {        impl_.assign(protocol, native_socket);};
BOOST_ASIO_SYNC_OP_VOID stream::socket::assign(const protocol_type& protocol, const native_handle_type& native_socket, boost::system::error_code& ec)  { return impl_.assign(protocol, native_socket, ec);};
bool stream::socket::is_open() const {return impl_.is_open();}
void stream::socket::close()                               { impl_.close();}
void stream::socket::close(boost::system::error_code& ec)  { impl_.close(ec);}

auto stream::socket::release()                              -> native_handle_type{ return impl_.release(); }
auto stream::socket::release(boost::system::error_code& ec) -> native_handle_type{ return impl_.release(ec); }
auto stream::socket::native_handle()                        -> native_handle_type{ return impl_.native_handle(); }
void stream::socket::cancel()                              { impl_.cancel();}
void stream::socket::cancel(boost::system::error_code& ec) { impl_.cancel(ec);}
bool stream::socket::at_mark()                              const { return impl_.at_mark();}
bool stream::socket::at_mark(boost::system::error_code& ec) const { return impl_.at_mark(ec);}
std::size_t stream::socket::available()                              const { return impl_.available();}
std::size_t stream::socket::available(boost::system::error_code& ec) const { return impl_.available(ec);}
                   void stream::socket::bind(const endpoint_type& endpoint)                                {        impl_.bind(endpoint); }
BOOST_ASIO_SYNC_OP_VOID stream::socket::bind(const endpoint_type& endpoint, boost::system::error_code& ec) { return impl_.bind(endpoint, ec); }


void stream::socket::async_wait(waitable_device::wait_type w, boost::async::completion_handler<system::error_code> h) { impl_.async_wait(w, std::move(h)); }
void stream::socket::async_connect(const endpoint_type& peer_endpoint, boost::async::completion_handler<system::error_code> h){ impl_.async_connect(peer_endpoint, std::move(h));}
void stream::socket::async_send(      const_buffer buffer, concepts::write_handler h)                           { impl_.async_send(      buffer,        std::move(h));}
void stream::socket::async_send(      const_buffer buffer, message_flags flags, concepts::write_handler h)      { impl_.async_send(      buffer, flags, std::move(h));}
void stream::socket::async_receive( mutable_buffer buffer, concepts::read_handler h)                            { impl_.async_receive(   buffer,        std::move(h));}
void stream::socket::async_receive( mutable_buffer buffer, message_flags flags, concepts::read_handler h)       { impl_.async_receive(   buffer, flags, std::move(h));}
void stream::socket::async_write_some(const_buffer buffer, concepts::write_handler h)                           { impl_.async_write_some(buffer,        std::move(h));}
void stream::socket::async_write_some(prepared_buffers buffer, concepts::write_handler h)                       { impl_.async_write_some(buffer,        std::move(h));}
void stream::socket::async_write_some(any_const_buffer_range buffer, concepts::write_handler h)                 { impl_.async_write_some(buffer,        std::move(h));}
void stream::socket::async_read_some(asio::mutable_buffer buffer,                     concepts::read_handler h) { impl_.async_read_some( buffer,        std::move(h));}
void stream::socket::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::read_handler h) { impl_.async_read_some( buffer,        std::move(h));}
void stream::socket::async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::read_handler h) { impl_.async_read_some( buffer,        std::move(h));}

// acceptor

stream::acceptor::acceptor(implementation_type&& impl) : impl_(std::move(impl)) {}
stream::acceptor& stream::acceptor::operator=(implementation_type&& impl)
{
  impl_ = std::move(impl);
  return *this;
}

stream::acceptor::acceptor(const executor_type& ex)                                                                           : impl_(ex) {}
stream::acceptor::acceptor(const executor_type& ex, const protocol_type& protocol)                                            : impl_(ex, protocol) {}
stream::acceptor::acceptor(const executor_type& ex, const endpoint_type& endpoint, bool reuse_addr)                           : impl_(ex, endpoint, reuse_addr) {}
stream::acceptor::acceptor(const executor_type& ex, const protocol_type& protocol, const native_handle_type& native_acceptor) : impl_(ex, protocol, native_acceptor) {}
stream::acceptor::acceptor(asio::io_context& context)                                                                           : impl_(context) {}
stream::acceptor::acceptor(asio::io_context& context, const protocol_type& protocol)                                            : impl_(context, protocol) {}
stream::acceptor::acceptor(asio::io_context& context, const endpoint_type& endpoint, bool reuse_addr)                           : impl_(context, endpoint, reuse_addr) {}
stream::acceptor::acceptor(asio::io_context& context, const protocol_type& protocol, const native_handle_type& native_acceptor) : impl_(context, protocol, native_acceptor) {}

stream::acceptor::acceptor(acceptor&& other) noexcept = default;
stream::acceptor& stream::acceptor::operator=(acceptor&& other) noexcept = default;
stream::acceptor::~acceptor() = default;
auto stream::acceptor::get_executor() noexcept -> executor_type { return impl_.get_executor(); }
                   void stream::acceptor::open(const protocol_type& protocol)                                { return impl_.open(protocol); }
BOOST_ASIO_SYNC_OP_VOID stream::acceptor::open(const protocol_type& protocol, boost::system::error_code& ec) { return impl_.open(protocol, ec); }
                   void stream::acceptor::assign(const protocol_type& protocol, const native_handle_type& native_acceptor)                                {return impl_.assign(protocol, native_acceptor);}
BOOST_ASIO_SYNC_OP_VOID stream::acceptor::assign(const protocol_type& protocol, const native_handle_type& native_acceptor, boost::system::error_code& ec) {return impl_.assign(protocol, native_acceptor, ec);}
bool stream::acceptor::is_open() const { return impl_.is_open();}
                   void stream::acceptor::bind(const endpoint_type& endpoint)                                {return impl_.bind(endpoint);}
BOOST_ASIO_SYNC_OP_VOID stream::acceptor::bind(const endpoint_type& endpoint, boost::system::error_code& ec) {return impl_.bind(endpoint, ec);}
        
                   void stream::acceptor::listen(int backlog )                               {        impl_.listen(backlog); }
BOOST_ASIO_SYNC_OP_VOID stream::acceptor::listen(int backlog, boost::system::error_code& ec) { return impl_.listen(backlog, ec); }

void stream::acceptor::close()                               { impl_.close();}
void stream::acceptor::close(boost::system::error_code& ec)  { impl_.close(ec);}
auto stream::acceptor::release()                              -> native_handle_type { return impl_.release(); }
auto stream::acceptor::release(boost::system::error_code& ec) -> native_handle_type { return impl_.release(ec); }
auto stream::acceptor::native_handle()                        -> native_handle_type { return impl_.native_handle(); }
void stream::acceptor::cancel()                              { impl_.cancel();}
void stream::acceptor::cancel(boost::system::error_code& ec) { impl_.cancel(ec);}
      
void stream::acceptor::async_wait(waitable_device::wait_type w, boost::async::completion_handler<system::error_code> h) { return impl_.async_wait(w, std::move(h)); }
void stream::acceptor::async_accept(socket & sock,                               boost::async::completion_handler<system::error_code> h)         { impl_.async_accept(sock.implementation(),          std::move(h)); }
void stream::acceptor::async_accept(socket & sock, endpoint_type& peer_endpoint, boost::async::completion_handler<system::error_code> h)         { impl_.async_accept(sock.implementation(),          std::move(h)); }
void stream::acceptor::async_accept(                                             boost::async::completion_handler<system::error_code, socket::implementation_type> h) { impl_.async_accept(                                std::move(h)); }
void stream::acceptor::async_accept(endpoint_type& peer_endpoint,                boost::async::completion_handler<system::error_code, socket::implementation_type> h) { impl_.async_accept(peer_endpoint,                  std::move(h)); }
    
    
}
#endif //BOOST_ASYNC_IO_LOCAL_IMPL_STREAM_IPP
