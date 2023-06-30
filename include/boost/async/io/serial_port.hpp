//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SERIAL_PORT_HPP
#define BOOST_ASYNC_IO_SERIAL_PORT_HPP

#include <boost/async/io/stream.hpp>

#include <boost/asio/basic_serial_port.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost::async::io
{

struct [[nodiscard]] serial_port final : stream
{
  system::result<void> close() override;
  system::result<void> cancel() override;
  bool is_open() const override;

  [[nodiscard]] system::result<void> send_break();

  [[nodiscard]] system::result<void>     set_baud_rate(unsigned rate);
  [[nodiscard]] system::result<unsigned> get_baud_rate();

  [[nodiscard]] system::result<void>     set_character_size(unsigned rate);
  [[nodiscard]] system::result<unsigned> get_character_size();

  using flow_control = asio::serial_port_base::flow_control::type;

  [[nodiscard]] system::result<void>         set_flow_control(flow_control rate);
  [[nodiscard]] system::result<flow_control> get_flow_control();

  using parity = asio::serial_port_base::parity::type;

  [[nodiscard]] system::result<void>   set_parity(parity rate);
  [[nodiscard]] system::result<parity> get_parity();

  using native_handle_type = typename asio::basic_serial_port<asio::io_context::executor_type>::native_handle_type;
  native_handle_type native_handle() {return serial_port_.native_handle();}

  serial_port();
  serial_port(serial_port && lhs) = default;
  serial_port(core::string_view device);
  serial_port(native_handle_type native_handle);

  [[nodiscard]] system::result<void> assign(native_handle_type native_handle);
  [[nodiscard]] system::result<native_handle_type> release();
  [[nodiscard]] system::result<serial_port> duplicate();

  [[nodiscard]] system::result<void> open(core::string_view device);

 private:
  void async_read_some_impl_(buffers::mutable_buffer_span buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  void async_write_some_impl_(buffers::const_buffer_span buffer, async::completion_handler<system::error_code, std::size_t> h) override;

  asio::basic_serial_port<executor_type> serial_port_;
};


}

#endif //BOOST_ASYNC_IO_SERIAL_PORT_HPP
