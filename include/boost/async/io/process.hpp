//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_PROCESS_HPP
#define BOOST_ASYNC_IO_PROCESS_HPP

#include <boost/async/config.hpp>
#include <boost/async/op.hpp>

#include <boost/process/v2/process.hpp>
#include <boost/process/v2/stdio.hpp>
#include <boost/process/v2/start_dir.hpp>
#include <boost/process/v2/environment.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/system/result.hpp>

#include <initializer_list>

namespace boost::async::io
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

  BOOST_ASYNC_DECL process(boost::process::v2::filesystem::path executable,
                           std::initializer_list<core::string_view> args,
                           process_initializer initializer = {});


  BOOST_ASYNC_DECL process(boost::process::v2::filesystem::path executable,
                           std::span<core::string_view> args,
                           process_initializer initializer = {});

  BOOST_ASYNC_DECL process(pid_type pid);
  BOOST_ASYNC_DECL process(pid_type pid, native_handle_type native_handle);

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> interrupt();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> request_exit();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> suspend();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> resume();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> terminate();
  [[nodiscard]] BOOST_ASYNC_DECL handle_type detach();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<bool> running();


  [[nodiscard]] pid_type id() const;

 private:
  struct wait_op_ : detail::deferred_op_resource_base
  {
    constexpr static bool await_ready() { return false; }

    BOOST_ASYNC_DECL void init_op(completion_handler<system::error_code, int> handler);

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        init_op(completion_handler<system::error_code, int>{h, result_, get_resource(h)});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    [[nodiscard]] wait_result await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      auto [ec, sig] = result_.value_or(std::make_tuple(system::error_code{}, 0));
      if (ec)
        return ec;
      else
        return sig;
    }
    wait_op_(boost::process::v2::basic_process<executor> & process) : process_(process) {}
   private:
    boost::process::v2::basic_process<executor> & process_;
    std::exception_ptr error;
    std::optional<std::tuple<system::error_code, int>> result_;
    char buffer[256];
    std::optional<container::pmr::monotonic_buffer_resource> resource;
  };
 public:
  [[nodiscard]] wait_op_ wait() { return wait_op_{process_}; }
  wait_op_ operator co_await () { return wait(); }
 private:
  boost::process::v2::basic_process<executor> process_;
  friend struct popen;
};


}

#endif //BOOST_ASYNC_IO_PROCESS_HPP
