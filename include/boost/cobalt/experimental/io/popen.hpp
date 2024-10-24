//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_EXPERIMENTAL_IO_COBALT_POPEN_HPP
#define BOOST_EXPERIMENTAL_IO_COBALT_POPEN_HPP

#include <boost/cobalt/config.hpp>
#include <boost/cobalt/experimental/io/process.hpp>
#include <boost/cobalt/experimental/io/stream.hpp>

#include <boost/process/v2/popen.hpp>

namespace boost::cobalt::experimental::io
{

struct popen
{
  using wait_result = system::result<int>;
  using handle_type = typename boost::process::v2::basic_process<executor>::handle_type;
  using native_handle_type = typename boost::process::v2::basic_process<executor>::native_handle_type;

  BOOST_COBALT_DECL popen(boost::process::v2::filesystem::path executable,
                         std::initializer_list<core::string_view> args,
                         process_initializer initializer = {},
                         const cobalt::executor & executor = this_thread::get_executor());


  BOOST_COBALT_DECL popen(boost::process::v2::filesystem::path executable,
                         std::span<core::string_view> args,
                         process_initializer initializer = {},
                         const cobalt::executor & executor = this_thread::get_executor());

  [[nodiscard]] BOOST_COBALT_DECL system::result<void> interrupt();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> request_exit();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> suspend();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> resume();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> terminate();
  [[nodiscard]] BOOST_COBALT_DECL handle_type detach();
  [[nodiscard]] BOOST_COBALT_DECL system::result<bool> running();


  [[nodiscard]] pid_type id() const;

  [[nodiscard]] system::result<void> close();
  [[nodiscard]] system::result<void> cancel();
  [[nodiscard]] bool is_open() const;

  process::wait_op wait()
  {
    return process::wait_op{this, initiate_wait_};
  }

  write_op write_some(const_buffer_sequence buffer)
  {
    return {buffer, this, initiate_write_some_};
  }
  read_op read_some(mutable_buffer_sequence buffer)
  {
    return {buffer, this, initiate_read_some_};
  }


 private:
  BOOST_COBALT_DECL static void initiate_wait_      (void *,                          completion_handler<boost::system::error_code, int>);
  BOOST_COBALT_DECL static void initiate_read_some_ (void *, mutable_buffer_sequence, completion_handler<boost::system::error_code, std::size_t>);
  BOOST_COBALT_DECL static void initiate_write_some_(void *, const_buffer_sequence,   completion_handler<boost::system::error_code, std::size_t>);

  boost::process::v2::basic_popen<executor> popen_;
};

}

#endif //BOOST_EXPERIMENTAL_IO_COBALT_POPEN_HPP
