//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_SERIAL_PORT_IPP
#define BOOST_ASYNC_IO_IMPL_SERIAL_PORT_IPP

#include <boost/async/io/serial_port.hpp>


namespace boost::async::io
{

serial_port::serial_port(const executor_type& ex) : impl_(ex) {}
serial_port::serial_port(asio::io_context& context) : impl_(context) {}

serial_port::serial_port(const executor_type& ex, const char* device) : impl_(ex, device) {}
serial_port::serial_port(const executor_type& ex, const std::string& device) : impl_(ex, device) {}
serial_port::serial_port(const executor_type& ex, const native_handle_type& native_serial_port) : impl_(ex, native_serial_port) {}
serial_port::serial_port(asio::io_context& context, const char* device) : impl_(context, device) {}
serial_port::serial_port(asio::io_context& context, const std::string& device) : impl_(context, device) {}
serial_port::serial_port(asio::io_context& context, const native_handle_type& native_serial_port) : impl_(context, native_serial_port) {}

serial_port::serial_port(serial_port&& other) = default;
serial_port& serial_port::operator=(serial_port&& other) = default;

serial_port::~serial_port() = default;

auto serial_port::get_executor() noexcept  -> executor_type { return impl_.get_executor(); }

auto serial_port::lowest_layer() -> lowest_layer_type& { return impl_.lowest_layer();}
auto serial_port::lowest_layer() const -> const lowest_layer_type&  { return impl_.lowest_layer();}

void serial_port::open(const std::string& device) { impl_.open(device);}
BOOST_ASIO_SYNC_OP_VOID serial_port::open(const std::string& device, boost::system::error_code& ec) { return impl_.open(device, ec);}
void serial_port::assign(const native_handle_type& native_serial_port) { impl_.assign(native_serial_port);}
BOOST_ASIO_SYNC_OP_VOID serial_port::assign(const native_handle_type& native_serial_port, boost::system::error_code& ec) { return impl_.assign(native_serial_port, ec);}
bool serial_port::is_open() const { return impl_.is_open();}
void serial_port::close()  { impl_.close();}
void serial_port::close(boost::system::error_code& ec) { impl_.close(ec);}
auto serial_port::native_handle() -> native_handle_type { return impl_.native_handle();}
void serial_port::cancel()  { impl_.cancel();}
void serial_port::cancel(boost::system::error_code& ec) { impl_.cancel(ec);}
void serial_port::send_break() { impl_.send_break();}
void serial_port::send_break(boost::system::error_code& ec) { impl_.send_break(ec);}

// boost::asio::serial_port_base::baud_rate
// boost::asio::serial_port_base::flow_control
// boost::asio::serial_port_base::parity
// boost::asio::serial_port_base::stop_bits
// boost::asio::serial_port_base::character_size

template <typename SettableSerialPortOption>
void serial_port::set_option(const SettableSerialPortOption& option) { impl_.set_option(option); }

template <typename SettableSerialPortOption>
BOOST_ASIO_SYNC_OP_VOID serial_port::set_option(const SettableSerialPortOption& option,
                                                boost::system::error_code& ec)
{
  return impl_.set_option(option, ec);
}

template <typename GettableSerialPortOption>
void serial_port::get_option(GettableSerialPortOption& option) const { return impl_.get_option(option);}


template <typename GettableSerialPortOption>
BOOST_ASIO_SYNC_OP_VOID serial_port::get_option(GettableSerialPortOption& option,
                             boost::system::error_code& ec) const
{
  return impl_.get_option(option, ec);
}

template void serial_port::set_option(const serial_port::baud_rate &);
template void serial_port::set_option(const serial_port::flow_control &);
template void serial_port::set_option(const serial_port::parity &);
template void serial_port::set_option(const serial_port::stop_bits &);
template void serial_port::set_option(const serial_port::character_size &);

template BOOST_ASIO_SYNC_OP_VOID serial_port::set_option(const serial_port::baud_rate &,       system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID serial_port::set_option(const serial_port::flow_control &,    system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID serial_port::set_option(const serial_port::parity &,          system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID serial_port::set_option(const serial_port::stop_bits &,       system::error_code &);
template BOOST_ASIO_SYNC_OP_VOID serial_port::set_option(const serial_port::character_size &,  system::error_code &);

template void serial_port::get_option(serial_port::baud_rate &)      const;
template void serial_port::get_option(serial_port::flow_control &)   const;
template void serial_port::get_option(serial_port::parity &)         const;
template void serial_port::get_option(serial_port::stop_bits &)      const;
template void serial_port::get_option(serial_port::character_size &) const;

template BOOST_ASIO_SYNC_OP_VOID serial_port::get_option(serial_port::baud_rate &,       system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID serial_port::get_option(serial_port::flow_control &,    system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID serial_port::get_option(serial_port::parity &,          system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID serial_port::get_option(serial_port::stop_bits &,       system::error_code &) const;
template BOOST_ASIO_SYNC_OP_VOID serial_port::get_option(serial_port::character_size &,  system::error_code &) const;


void serial_port::async_write_some(asio::const_buffer buffer, concepts::write_handler h)
{
  impl_.async_write_some(buffer, std::move(h));
};
void serial_port::async_write_some(prepared_buffers buffer, concepts::write_handler h)
{
  impl_.async_write_some(buffer, std::move(h));
}
void serial_port::async_read_some(asio::mutable_buffer buffer,                     concepts::write_handler h)
{
  impl_.async_read_some(buffer, std::move(h));
}
void serial_port::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::write_handler h)
{
  impl_.async_read_some(buffer, std::move(h));
}
void serial_port::async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::write_handler h)
{
  impl_.async_read_some(buffer, std::move(h));
}


}

#endif //BOOST_ASYNC_IO_IMPL_SERIAL_PORT_IPP
