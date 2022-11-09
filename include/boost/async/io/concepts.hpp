//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_CONCEPTS_HPP
#define BOOST_ASYNC_IO_CONCEPTS_HPP

#include <boost/system/error_code.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/async/io/buffer.hpp>
#include <boost/async/detail/handler.hpp>

namespace boost::async::io::concepts
{


struct closable
{
  virtual void close() = 0;
  virtual void close(system::error_code & ec) = 0;
  virtual bool is_open() const = 0;
  virtual ~closable() = default;
};

struct cancellable
{
  virtual void cancel() = 0;
  virtual void cancel(system::error_code & ec) = 0;
  virtual ~cancellable() = default;
};


struct execution_context
{
  using executor_type = asio::io_context::executor_type;
  virtual executor_type get_executor() = 0;
  virtual ~execution_context() = default;
};


struct read_stream : virtual execution_context
{
 protected:
  struct read_some_op_base
  {
    virtual void await_suspend(void * p, asio::mutable_buffer buffer,
                               boost::async::detail::completion_handler<system::error_code, std::size_t> h) const = 0;
  };
  struct read_some_op_
  {
    void * impl;
    asio::mutable_buffer buffer;
    const read_some_op_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl, buffer, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));

      return std::get<1>(*result);
    }
  };

  struct read_some_op_ec_
  {
    void * impl;
    asio::mutable_buffer buffer;
    system::error_code & ec;
    const read_some_op_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, ec, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      ec = std::get<0>(*result);
      return std::get<1>(*result);
    }
  };

 public:
  virtual read_some_op_ read_some(asio::mutable_buffer buffer) = 0;
  virtual read_some_op_ec_ read_some(asio::mutable_buffer buffer, system::error_code & ec) = 0;

};

struct write_stream : virtual execution_context
{
 protected:

  struct write_some_op_base
  {
    virtual void await_suspend(void * p, asio::const_buffer,
                               boost::async::detail::completion_handler<system::error_code, std::size_t> h) const = 0;
  };


  struct write_some_op_
  {
    void * impl;
    asio::const_buffer buffer;
    const write_some_op_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));

      return std::get<1>(*result);
    }
  };

  struct write_some_op_ec_
  {
    struct base
    {
      virtual void await_suspend(void * p, asio::const_buffer, system::error_code & ec,
                                boost::async::detail::completion_handler<std::size_t> h) const = 0;
    };
    void * impl;
    asio::const_buffer buffer;
    system::error_code & ec;
    const write_some_op_base & methods;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, ec, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      ec = std::get<0>(*result);

      return std::get<1>(*result);
    }
  };
 public:

  virtual write_some_op_    write_some(asio::const_buffer buffer) = 0;
  virtual write_some_op_ec_ write_some(asio::const_buffer buffer, system::error_code & ec) = 0;
};

struct timer : virtual execution_context
{
 protected:

  struct wait_op_base
  {
    virtual bool await_ready(void * p) const = 0;
    virtual void await_suspend(void * p,
                               boost::async::detail::completion_handler<system::error_code> h) const = 0;
  };
  struct wait_op_
  {

    void * impl;
    const wait_op_base & methods;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    bool await_ready() const {return methods.await_ready(impl);}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    void await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));
    }
  };

  struct wait_op_ec_
  {
    void * impl;
    system::error_code & ec;
    const wait_op_base & methods;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    bool await_ready() const {return methods.await_ready(impl);}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  ec, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    void await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      ec = std::get<0>(*result);
    }
  };
 public:
  virtual wait_op_    wait() = 0;
  virtual wait_op_ec_ wait(system::error_code & ec) = 0;

  virtual std::size_t cancel_one() = 0;
  virtual std::size_t cancel_one(system::error_code & ec) = 0;

  virtual std::size_t cancel() = 0;
  virtual std::size_t cancel(system::error_code & ec) = 0;
};


struct random_access_read_device : virtual execution_context
{
 protected:
  struct read_some_at_base
  {
    virtual void await_suspend(void * p, std::uint64_t offset, asio::mutable_buffer buffer,
                               boost::async::detail::completion_handler<system::error_code, std::size_t> h) const = 0;
  };
  struct read_some_at_op_
  {
    void * impl;
    std::uint64_t offset;
    asio::mutable_buffer buffer;
    const read_some_at_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));

      return std::get<1>(*result);
    }
  };

  struct read_some_at_op_ec_
  {

    void * impl;
    std::uint64_t offset;
    asio::mutable_buffer buffer;
    system::error_code & ec;
    const read_some_at_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, ec, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      ec = std::get<0>(*result);
      return std::get<1>(*result);
    }
  };

  virtual read_some_at_op_    read_some(std::uint64_t offset, asio::mutable_buffer buffer) = 0;
  virtual read_some_at_op_ec_ read_some(std::uint64_t offset, asio::mutable_buffer buffer, system::error_code & ec) = 0;
};

struct random_access_write_device : virtual execution_context
{
 protected:

  struct write_some_at_base
  {
    virtual void await_suspend(void * p, std::uint64_t offset, asio::const_buffer,
                               boost::async::detail::completion_handler<system::error_code, std::size_t> h) const = 0;
  };
  struct write_some_at_op_
  {
    void * impl;
    std::uint64_t offset;
    asio::const_buffer buffer;
    const write_some_at_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));

      return std::get<1>(*result);
    }
  };

  struct write_some_at_op_ec_
  {

    void * impl;
    std::uint64_t offset;
    asio::const_buffer buffer;
    system::error_code & ec;
    const write_some_at_base & methods;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  buffer, ec, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    std::size_t await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      ec = std::get<0>(*result);
      return std::get<1>(*result);
    }
  };
 public:

  virtual write_some_at_op_    write_some(std::uint64_t offset, asio::const_buffer buffer) = 0;
  virtual write_some_at_op_ec_ write_some(std::uint64_t offset, asio::const_buffer buffer, system::error_code & ec) = 0;

};

struct async_waitable_device : virtual execution_context
{
  /// Wait types.
  /**
   * For use with descriptor::wait() and descriptor::async_wait().
   */
  enum wait_type
  {
    /// Wait for a descriptor to become ready to read.
    wait_read,

    /// Wait for a descriptor to become ready to write.
    wait_write,

    /// Wait for a descriptor to have error conditions pending.
    wait_error
  };

 protected:
  struct wait_op_base
  {
    virtual bool await_ready(void * p) const = 0;
    virtual void await_suspend(void * p, wait_type wt,
                               boost::async::detail::completion_handler<system::error_code> h) const = 0;
  };
  struct wait_op_
  {

    void * impl;
    wait_type wt;
    const wait_op_base & methods;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl,  wt, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    void await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));
    }
  };

  struct wait_op_ec_
  {
    void * impl;
    wait_type wt;
    system::error_code & ec;
    const wait_op_base & methods;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        methods.await_suspend(impl, wt, ec, {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    void await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      ec = std::get<0>(*result);
    }
  };
 public:
  virtual wait_op_    wait(wait_type wt) = 0;
  virtual wait_op_ec_ wait(wait_type wt, system::error_code & ec) = 0;
};

template<typename ... Args>
struct implements : Args ...
{

};


}

#endif //BOOST_ASYNC_IO_CONCEPTS_HPP
