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
  BOOST_ASYNC_DECL system::result<void> close() override;
  BOOST_ASYNC_DECL system::result<void> cancel() override;
  BOOST_ASYNC_DECL bool is_open() const override;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> send_break();

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>     set_baud_rate(unsigned rate);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<unsigned> get_baud_rate();

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>     set_character_size(unsigned rate);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<unsigned> get_character_size();

  using flow_control = asio::serial_port_base::flow_control::type;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>         set_flow_control(flow_control rate);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<flow_control> get_flow_control();

  using parity = asio::serial_port_base::parity::type;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void>   set_parity(parity rate);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<parity> get_parity();

  using native_handle_type = typename asio::basic_serial_port<executor_type>::native_handle_type;
  native_handle_type native_handle() {return serial_port_.native_handle();}

  BOOST_ASYNC_DECL serial_port();
  BOOST_ASYNC_DECL serial_port(serial_port && lhs) = default;
  BOOST_ASYNC_DECL serial_port(core::string_view device);
  BOOST_ASYNC_DECL serial_port(native_handle_type native_handle);

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> assign(native_handle_type native_handle);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<native_handle_type> release();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<serial_port> duplicate();

  BOOST_ASYNC_DECL [[nodiscard]] system::result<void> open(core::string_view device);

 private:
  BOOST_ASYNC_DECL void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  BOOST_ASYNC_DECL void async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;

  asio::basic_serial_port<executor_type> serial_port_;
};


}

#endif //BOOST_ASYNC_IO_SERIAL_PORT_HPP
