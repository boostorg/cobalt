//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_PROCESS_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_PROCESS_HPP


#include <boost/cobalt/config.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>

#include <boost/process/v2/process.hpp>
#include <boost/process/v2/stdio.hpp>
#include <boost/process/v2/start_dir.hpp>
#include <boost/process/v2/environment.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/system/result.hpp>

#include <initializer_list>

namespace boost::cobalt::experimental::io
{

using boost::process::v2::pid_type;

struct process_initializer
{
  process::v2::process_stdio       stdio;
  process::v2::process_start_dir   start_dir{process::v2::filesystem::current_path()};
  process::v2::process_environment env{process::v2::environment::current()};
};

struct process
{
  using wait_result = system::result<int>;
  using handle_type = typename boost::process::v2::basic_process<executor>::handle_type;
  using native_handle_type = typename boost::process::v2::basic_process<executor>::native_handle_type;

  BOOST_COBALT_DECL process(boost::process::v2::filesystem::path executable,
                           std::initializer_list<core::string_view> args,
                           process_initializer initializer = {},
                           const cobalt::executor & executor = this_thread::get_executor());


  BOOST_COBALT_DECL process(boost::process::v2::filesystem::path executable,
                           std::span<core::string_view> args,
                           process_initializer initializer = {},
                           const cobalt::executor & executor = this_thread::get_executor());

  BOOST_COBALT_DECL process(pid_type pid, const cobalt::executor & executor = this_thread::get_executor());
  BOOST_COBALT_DECL process(pid_type pid, native_handle_type native_handle,
                           const cobalt::executor & executor = this_thread::get_executor());

  [[nodiscard]] BOOST_COBALT_DECL system::result<void> interrupt();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> request_exit();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> suspend();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> resume();
  [[nodiscard]] BOOST_COBALT_DECL system::result<void> terminate();
  [[nodiscard]] BOOST_COBALT_DECL handle_type detach();
  [[nodiscard]] BOOST_COBALT_DECL system::result<bool> running();


  [[nodiscard]] pid_type id() const;

  struct wait_op
  {
    void *this_;
    void (*implementation)(void * this_,
                           boost::cobalt::completion_handler<boost::system::error_code, int>);

    op_awaitable<wait_op, std::tuple<>, boost::system::error_code, int>
        operator co_await()
    {
      return {this};
    }
  };


  [[nodiscard]]  auto wait() { return wait_op{this, initiate_wait_}; }
 private:

  BOOST_COBALT_DECL static void initiate_wait_(void *, boost::cobalt::completion_handler<boost::system::error_code, int>);

  boost::process::v2::basic_process<executor> process_;
  friend struct popen;
};


}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_PROCESS_HPP
