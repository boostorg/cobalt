//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IP_IMPL_UDP_IPP
#define BOOST_ASYNC_IO_IP_IMPL_UDP_IPP

#include <boost/async/io/ip/udp.hpp>

namespace boost::async::io::ip
{


udp::socket::socket(implementation_type&& impl) : impl_(std::move(impl)) {}
udp::socket& udp::socket::operator=(implementation_type&& impl)
{
  impl_ = std::move(impl);
  return *this;
}
udp::socket::socket(const executor_type& ex)                                                                           : impl_(ex) {}
udp::socket::socket(const executor_type& ex, const protocol_type& protocol)                                            : impl_(ex, protocol) {}
udp::socket::socket(const executor_type& ex, const endpoint_type& endpoint)                                            : impl_(ex, endpoint) {}
udp::socket::socket(const executor_type& ex, const protocol_type& protocol, const native_handle_type& native_socket)   : impl_(ex, protocol, native_socket) {}
udp::socket::socket(asio::io_context& context)                                                                         : impl_(context) {}
udp::socket::socket(asio::io_context& context, const protocol_type& protocol)                                          : impl_(context, protocol) {}
udp::socket::socket(asio::io_context& context, const endpoint_type& endpoint)                                          : impl_(context, endpoint) {}
udp::socket::socket(asio::io_context& context, const protocol_type& protocol, const native_handle_type& native_socket) : impl_(context, protocol, native_socket) {}

udp::socket::socket(socket&& other) noexcept = default;
udp::socket& udp::socket::operator=(socket&& other) = default;
udp::socket::~socket() = default;

auto udp::socket::get_executor() noexcept -> executor_type         { return impl_.get_executor(); }
auto udp::socket::lowest_layer() -> lowest_layer_type&             { return impl_; }
auto udp::socket::lowest_layer() const -> const lowest_layer_type& { return impl_; }

template <typename SettableSocketOption>
void udp::socket::set_option(const SettableSocketOption& option)
{
  impl_.set_option(option);
}

template <typename SettableSocketOption>
BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const SettableSocketOption& option, boost::system::error_code& ec)
{
  return impl_.set_option(option, ec);
}
template <typename GettableSocketOption>
void udp::socket::get_option(GettableSocketOption& option) const
{
  impl_.get_option(option);
}
template <typename GettableSocketOption>
BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(GettableSocketOption& option, boost::system::error_code& ec) const
{
  return impl_.get_option(option, ec);
}

template void udp::socket::set_option(const asio::socket_base::debug &);
template void udp::socket::set_option(const asio::socket_base::do_not_route &);
template void udp::socket::set_option(const asio::socket_base::enable_connection_aborted &);
template void udp::socket::set_option(const asio::socket_base::keep_alive &);
template void udp::socket::set_option(const asio::socket_base::linger &);
template void udp::socket::set_option(const asio::socket_base::out_of_band_inline &);
template void udp::socket::set_option(const asio::socket_base::receive_buffer_size &);
template void udp::socket::set_option(const asio::socket_base::receive_low_watermark &);
template void udp::socket::set_option(const asio::socket_base::reuse_address &);
template void udp::socket::set_option(const asio::socket_base::send_buffer_size &);
template void udp::socket::set_option(const asio::socket_base::send_low_watermark &);

template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::debug &,                     system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::do_not_route &,              system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::enable_connection_aborted &, system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::keep_alive &,                system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::linger &,                    system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::out_of_band_inline &,        system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::receive_buffer_size &,       system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::receive_low_watermark &,     system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::reuse_address &,             system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::send_buffer_size &,          system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::set_option(const asio::socket_base::send_low_watermark &,        system::error_code &);

template void udp::socket::get_option(boost::asio::socket_base::debug &) const;
template void udp::socket::get_option(boost::asio::socket_base::do_not_route &) const;
template void udp::socket::get_option(boost::asio::socket_base::enable_connection_aborted &) const;
template void udp::socket::get_option(boost::asio::socket_base::keep_alive &) const;
template void udp::socket::get_option(boost::asio::socket_base::linger &) const;
template void udp::socket::get_option(boost::asio::socket_base::out_of_band_inline &) const;
template void udp::socket::get_option(boost::asio::socket_base::receive_buffer_size &) const;
template void udp::socket::get_option(boost::asio::socket_base::receive_low_watermark &) const;
template void udp::socket::get_option(boost::asio::socket_base::reuse_address &) const;
template void udp::socket::get_option(boost::asio::socket_base::send_buffer_size &) const;
template void udp::socket::get_option(boost::asio::socket_base::send_low_watermark &) const;

template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::debug &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::do_not_route &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::enable_connection_aborted &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::keep_alive &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::linger &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::out_of_band_inline &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::receive_buffer_size &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::receive_low_watermark &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::reuse_address &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::send_buffer_size &, system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID udp::socket::get_option(boost::asio::socket_base::send_low_watermark &, system::error_code &) const;


template <typename IoControlCommand>                    void udp::socket::io_control(IoControlCommand& command)                                {        impl_.io_control(command);     }
template <typename IoControlCommand> BOOST_ASIO_SYNC_OP_VOID udp::socket::io_control(IoControlCommand& command, boost::system::error_code& ec) { return impl_.io_control(command, ec); }

template                    void udp::socket::io_control(boost::asio::socket_base::bytes_readable &);
template BOOST_ASIO_SYNC_OP_VOID udp::socket::io_control(boost::asio::socket_base::bytes_readable &, system::error_code &);


bool udp::socket::non_blocking() const                                   { return impl_.non_blocking(); }
void udp::socket::non_blocking(bool mode)                                {        impl_.non_blocking(mode); }
void udp::socket::non_blocking(bool mode, boost::system::error_code& ec) {        impl_.non_blocking(mode, ec); }

bool udp::socket::native_non_blocking() const                                   { return impl_.native_non_blocking(); }
void udp::socket::native_non_blocking(bool mode)                                {        impl_.native_non_blocking(mode); }
void udp::socket::native_non_blocking(bool mode, boost::system::error_code& ec) {        impl_.native_non_blocking(mode, ec); }

void udp::socket::shutdown(shutdown_type what)                                { impl_.shutdown(what);};
void udp::socket::shutdown(shutdown_type what, boost::system::error_code& ec) { impl_.shutdown(what, ec);};
auto udp::socket::local_endpoint() const                              -> endpoint_type { return impl_.local_endpoint(); }
auto udp::socket::local_endpoint(boost::system::error_code& ec) const -> endpoint_type { return impl_.local_endpoint(ec); }

auto udp::socket::remote_endpoint() const                              -> endpoint_type { return impl_.remote_endpoint(); }
auto udp::socket::remote_endpoint(boost::system::error_code& ec) const -> endpoint_type { return impl_.remote_endpoint(ec); }
void udp::socket::open(const protocol_type& protocol)                                {        impl_.open(protocol);};
BOOST_ASIO_SYNC_OP_VOID udp::socket::open(const protocol_type& protocol, boost::system::error_code& ec) { return impl_.open(protocol, ec);};
void udp::socket::assign(const protocol_type& protocol, const native_handle_type& native_socket)                                 {        impl_.assign(protocol, native_socket);};
BOOST_ASIO_SYNC_OP_VOID udp::socket::assign(const protocol_type& protocol, const native_handle_type& native_socket, boost::system::error_code& ec)  { return impl_.assign(protocol, native_socket, ec);};
bool udp::socket::is_open() const {return impl_.is_open();}
void udp::socket::close()                               { impl_.close();}
void udp::socket::close(boost::system::error_code& ec)  { impl_.close(ec);}

auto udp::socket::release()                              -> native_handle_type{ return impl_.release(); }
auto udp::socket::release(boost::system::error_code& ec) -> native_handle_type{ return impl_.release(ec); }
auto udp::socket::native_handle()                        -> native_handle_type{ return impl_.native_handle(); }
void udp::socket::cancel()                              { impl_.cancel();}
void udp::socket::cancel(boost::system::error_code& ec) { impl_.cancel(ec);}
bool udp::socket::at_mark()                              const { return impl_.at_mark();}
bool udp::socket::at_mark(boost::system::error_code& ec) const { return impl_.at_mark(ec);}
std::size_t udp::socket::available()                              const { return impl_.available();}
std::size_t udp::socket::available(boost::system::error_code& ec) const { return impl_.available(ec);}
void udp::socket::bind(const endpoint_type& endpoint)                                {        impl_.bind(endpoint); }
BOOST_ASIO_SYNC_OP_VOID udp::socket::bind(const endpoint_type& endpoint, boost::system::error_code& ec) { return impl_.bind(endpoint, ec); }


void udp::socket::async_wait(waitable_device::wait_type w, boost::async::detail::completion_handler<system::error_code> h) { return impl_.async_wait(w, std::move(h)); }
void udp::socket::async_connect(const endpoint_type& peer_endpoint, boost::async::detail::completion_handler<system::error_code> h){return impl_.async_connect(peer_endpoint, std::move(h));}
void udp::socket::async_send(      const_buffer buffer, concepts::write_handler h)                           {return impl_.async_send(      buffer,        std::move(h));}
void udp::socket::async_send(      const_buffer buffer, message_flags flags, concepts::write_handler h)      {return impl_.async_send(      buffer, flags, std::move(h));}
void udp::socket::async_receive( mutable_buffer buffer, concepts::read_handler h)                            {return impl_.async_receive(   buffer,        std::move(h));}
void udp::socket::async_receive( mutable_buffer buffer, message_flags flags, concepts::read_handler h)       {return impl_.async_receive(   buffer, flags, std::move(h));}

void udp::socket::async_send_to(     mutable_buffer buffers, endpoint_type& sender_endpoint,                                   concepts::write_handler h) {return impl_.async_send_to(buffers, sender_endpoint,        std::move(h));}
void udp::socket::async_send_to(     mutable_buffer buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags, concepts::write_handler h) {return impl_.async_send_to(buffers, sender_endpoint, flags, std::move(h));}
void udp::socket::async_receive_from(mutable_buffer buffers, endpoint_type& sender_endpoint,                                   concepts::read_handler  h) {return impl_.async_receive_from(buffers, sender_endpoint,        std::move(h));}
void udp::socket::async_receive_from(mutable_buffer buffers, endpoint_type& sender_endpoint, socket_base::message_flags flags, concepts::read_handler  h) {return impl_.async_receive_from(buffers, sender_endpoint, flags, std::move(h));}



udp::resolver::resolver(const executor_type& ex) : impl_(ex) {}
udp::resolver::resolver(asio::io_context& context) : impl_(context) {}

udp::resolver::resolver(resolver&& other) = default;
udp::resolver& udp::resolver::operator=(resolver&& other) = default;
udp::resolver::~resolver() = default;

auto udp::resolver::get_executor() noexcept  -> executor_type { return impl_.get_executor(); }
void udp::resolver::cancel() { impl_.cancel(); }

void udp::resolver::async_resolve(asio::string_view host, asio::string_view service,
                   boost::async::detail::completion_handler<system::error_code, results_type> h)
{
  impl_.async_resolve(host, service, std::move(h));
}

void udp::resolver::async_resolve(asio::string_view host, asio::string_view  service,
                   resolver_base::flags resolve_flags,
                   boost::async::detail::completion_handler<system::error_code, results_type> h)
{
  impl_.async_resolve(host, service, resolve_flags, std::move(h));
}

void udp::resolver::async_resolve(const protocol_type& protocol,
                   asio::string_view host, asio::string_view service,
                   boost::async::detail::completion_handler<system::error_code, results_type> h)
{
  impl_.async_resolve(protocol, host, service, std::move(h));
}

void udp::resolver::async_resolve(const protocol_type& protocol,
                   asio::string_view host,
                   asio::string_view service,
                   resolver_base::flags resolve_flags,
                   boost::async::detail::completion_handler<system::error_code, results_type> h)
{
  impl_.async_resolve(protocol, host, service, resolve_flags, std::move(h));
}
void udp::resolver::async_resolve(const endpoint_type& e,
                   boost::async::detail::completion_handler<system::error_code, results_type> h)
{
  impl_.async_resolve(e, std::move(h));
}



}

#endif //BOOST_ASYNC_IO_IP_IMPL_UDP_IPP
