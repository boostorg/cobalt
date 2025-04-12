//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/serial_port.hpp>

namespace boost::cobalt::io
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

serial_port::serial_port(const cobalt::executor & executor)
    : serial_port_(executor) {}
serial_port::serial_port(std::string_view device, const cobalt::executor & executor)
    : serial_port_(executor, std::string(device)) {}
serial_port::serial_port(native_handle_type native_handle, const cobalt::executor & executor)
    : serial_port_(executor, native_handle) {}

system::result<void> serial_port::assign(native_handle_type native_handle)
{
  system::error_code ec;
  serial_port_.assign(native_handle, ec);
  return ec ? ec : system::result<void>{};
}

[[nodiscard]] system::result<void> serial_port::open(std::string_view device)
{
  std::string dev{device};
  system::error_code ec;
  serial_port_.open(dev, ec);
  if (ec)
    return ec;
  else
    return boost::system::in_place_value;
}


void serial_port::initiate_read_some_(void * this_, mutable_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buffer)
        {
          static_cast<serial_port*>(this_)->serial_port_.async_read_some(buffer, std::move(handler));
        });
}
void serial_port::initiate_write_some_(void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buffer)
        {
          static_cast<serial_port*>(this_)->serial_port_.async_write_some(buffer, std::move(handler));
        });
}


}