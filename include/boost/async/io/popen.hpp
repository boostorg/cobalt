//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_POPEN_HPP
#define BOOST_ASYNC_IO_POPEN_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/process.hpp>
#include <boost/process/v2/popen.hpp>
#include <boost/async/io/stream.hpp>

namespace boost::async::io
{

struct popen : stream
{
  using wait_result = system::result<int>;
  using handle_type = typename boost::process::v2::basic_process<executor>::handle_type;
  using native_handle_type = typename boost::process::v2::basic_process<executor>::native_handle_type;

  BOOST_ASYNC_DECL popen(boost::process::v2::filesystem::path executable,
                         std::initializer_list<core::string_view> args,
                         process_initializer initializer = {},
                         const async::executor & executor = this_thread::get_executor());


  BOOST_ASYNC_DECL popen(boost::process::v2::filesystem::path executable,
                         std::span<core::string_view> args,
                         process_initializer initializer = {},
                         const async::executor & executor = this_thread::get_executor());

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> interrupt();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> request_exit();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> suspend();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> resume();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> terminate();
  [[nodiscard]] BOOST_ASYNC_DECL handle_type detach();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> running();


  [[nodiscard]] pid_type id() const;

  [[nodiscard]] system::result<void> close() override;
  [[nodiscard]] system::result<void> cancel() override;
  [[nodiscard]] bool is_open() const override;

 private:
  void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h)override;
  void async_write_some_impl_(buffers::const_buffer_subspan buffer,  async::completion_handler<system::error_code, std::size_t> h) override;
 public:
  [[nodiscard]] process::wait_op_ wait() { return process::wait_op_{popen_}; }
  process::wait_op_ operator co_await () { return wait(); }
 private:
  boost::process::v2::basic_popen<executor> popen_;
};


}

#endif //BOOST_ASYNC_IO_POPEN_HPP
