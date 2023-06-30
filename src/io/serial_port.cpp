//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/serial_port.hpp>
#include <boost/async/io/detail/duplicate.hpp>

namespace boost::async::io
{

system::result<void> serial_port::close()
{
  system::error_code ec;
  serial_port_.close(ec);
  return ec ? ec : system::result<void>();
}

system::result<void> serial_port::cancel()
{
  system::error_code ec;
  serial_port_.cancel(ec);
  return ec ? ec : system::result<void>();
}

bool serial_port::is_open() const { return serial_port_.is_open(); }

[[nodiscard]] system::result<void> serial_port::send_break()
{
  system::error_code ec;
  serial_port_.send_break(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void>     serial_port::set_baud_rate(unsigned rate)
{
  system::error_code ec;
  serial_port_.set_option(asio::serial_port_base::baud_rate(rate), ec);
  return ec ? ec : system::result<void>();
}

system::result<unsigned> serial_port::get_baud_rate()
{
  system::error_code ec;
  asio::serial_port_base::baud_rate br;
  serial_port_.get_option(br, ec);
  return ec ? ec : system::result<unsigned>(br.value());
}

system::result<void>     serial_port::set_character_size(unsigned rate)
{
  system::error_code ec;
  serial_port_.set_option(asio::serial_port_base::character_size(rate), ec);
  return ec ? ec : system::result<void>();
}

system::result<unsigned> serial_port::get_character_size()
{
  system::error_code ec;
  asio::serial_port_base::character_size br;
  serial_port_.get_option(br, ec);
  return ec ? ec : system::result<unsigned>(br.value());
}


system::result<void>     serial_port::set_flow_control(flow_control rate)
{
  system::error_code ec;
  serial_port_.set_option(asio::serial_port_base::flow_control(rate), ec);
  return ec ? ec : system::result<void>();
}

auto serial_port::get_flow_control() -> system::result<flow_control>
{
  system::error_code ec;
  asio::serial_port_base::flow_control br;
  serial_port_.get_option(br, ec);
  return ec ? ec : system::result<flow_control>(br.value());
}


system::result<void>     serial_port::set_parity(parity rate)
{
  system::error_code ec;
  serial_port_.set_option(asio::serial_port_base::parity(rate), ec);
  return ec ? ec : system::result<void>();
}

auto serial_port::get_parity() -> system::result<parity>
{
  system::error_code ec;
  asio::serial_port_base::parity br;
  serial_port_.get_option(br, ec);
  return ec ? ec : system::result<parity>(br.value());
}

serial_port::serial_port()
    : serial_port_(this_thread::get_executor()) {}
serial_port::serial_port(core::string_view device)
    : serial_port_(this_thread::get_executor(), std::string(device)) {}
serial_port::serial_port(native_handle_type native_handle)
    : serial_port_(this_thread::get_executor(), native_handle) {}

system::result<void> serial_port::assign(native_handle_type native_handle)
{
  system::error_code ec;
  serial_port_.assign(native_handle, ec);
  return ec ? ec : system::result<void>{};
}

[[nodiscard]] system::result<void> serial_port::open(core::string_view device)
{
  std::string dev{device};
  system::error_code ec;
  serial_port_.open(dev, ec);
  if (ec)
    return ec;
  else
    return system::in_place_value;
}


void serial_port::async_read_some_impl_(
    buffers::mutable_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  serial_port_.async_read_some(buffer, std::move(h));
}

void serial_port::async_write_some_impl_(
    buffers::const_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  serial_port_.async_write_some(buffer, std::move(h));
}

auto serial_port::release() -> system::result<native_handle_type>
{
  // ain't done in asio, for some reason.
  constexpr static source_location loc{BOOST_CURRENT_LOCATION};
  system::error_code ec{asio::error::operation_not_supported, &loc};
  return ec;
}
auto serial_port::duplicate() -> system::result<serial_port>
{
  auto fd = detail::duplicate_handle(serial_port_.native_handle());
  if (fd.has_error())
    return fd.error();
  return serial_port(*fd);
}

}