//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_CONCEPTS_HPP
#define BOOST_ASYNC_IO_CONCEPTS_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/async/io/buffer.hpp>
#include <boost/async/io/concepts.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/system/error_code.hpp>

namespace boost::async::io::concepts
{

using write_handler = boost::async::detail::completion_handler<system::error_code, std::size_t>;
using read_handler  = boost::async::detail::completion_handler<system::error_code, std::size_t>;

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
  /// The type of the executor associated with the object.
  using executor_type = asio::io_context::executor_type;
  /// Get the executor associated with the object.
  virtual executor_type get_executor() = 0;
  virtual ~execution_context() = default;
};


struct read_stream : virtual execution_context
{
 protected:
  struct read_some_op_
  {
    read_stream * impl;
    asio::mutable_buffer buffer;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->read_some(buffer, {h, result});
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
    read_stream * impl;
    asio::mutable_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->read_some(buffer, ec, {h, result});
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
  void async_read_some(beast::detail::BufferSequence<asio::mutable_buffer> buffer, read_handler h);

  virtual void async_read_some(asio::mutable_buffer buffer,                     read_handler h) = 0;
  virtual void async_read_some(static_buffer_base::mutable_buffers_type buffer, read_handler h) = 0;
  virtual void async_read_some(multi_buffer::mutable_buffers_type buffer,       read_handler h) = 0;

  [[nodiscard]] read_some_op_ read_some(asio::mutable_buffer buffer)
  {
    return read_some_op_{this, buffer};
  }
  [[nodiscard]] read_some_op_ec_ read_some(asio::mutable_buffer buffer, system::error_code & ec)
  {
    return read_some_op_ec_{this, buffer, ec};
  };

};

struct write_stream : virtual execution_context
{
 protected:

  struct write_some_op_
  {
    write_stream * impl;
    asio::const_buffer buffer;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_write_some(buffer, {h, result});
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
    write_stream * impl;
    asio::const_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_write_some(buffer, {h, result});
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

  [[nodiscard]] write_some_op_    write_some(asio::const_buffer buffer)
  {
      return write_some_op_{this, buffer};
  }
  [[nodiscard]] write_some_op_ec_ write_some(asio::const_buffer buffer, system::error_code & ec)
  {
    return  write_some_op_ec_{this, buffer, ec};
  }
  virtual void async_write_some(any_const_buffer_range buffer, write_handler h) = 0;
  virtual void async_write_some(const_buffer     buffer, write_handler h) = 0;
  virtual void async_write_some(prepared_buffers buffer, write_handler h) = 0;
};

struct stream : read_stream, write_stream {};

struct timer : virtual execution_context
{
 protected:

  struct wait_op_
  {

    timer * impl;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    bool await_ready() const {return impl->expired();}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_wait({h, result});
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
    timer * impl;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    bool await_ready() const {return impl->expired();}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_wait({h, result});
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

  virtual void async_wait(boost::async::detail::completion_handler<system::error_code> h) = 0;
  virtual bool expired() const = 0;

  [[nodiscard]] wait_op_    wait()                        {return wait_op_{this}; }
  [[nodiscard]] wait_op_ec_ wait(system::error_code & ec) { return wait_op_ec_{this, ec}; };

  virtual std::size_t cancel_one() = 0;
  virtual std::size_t cancel_one(system::error_code & ec) = 0;

  virtual std::size_t cancel() = 0;
  virtual std::size_t cancel(system::error_code & ec) = 0;
};


struct random_access_read_device : virtual execution_context
{
 protected:
  struct read_some_at_op_
  {
    random_access_read_device * impl;
    std::uint64_t offset;
    asio::mutable_buffer buffer;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_read_some_at(offset, buffer, {h, result});
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

    random_access_read_device * impl;
    std::uint64_t offset;
    asio::mutable_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_read_some_at(offset, buffer, {h, result});
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

  [[nodiscard]] read_some_at_op_    read_some(std::uint64_t offset, asio::mutable_buffer buffer)
  {
    return read_some_at_op_{this, offset, buffer};
  }
  [[nodiscard]] read_some_at_op_ec_ read_some(std::uint64_t offset, asio::mutable_buffer buffer, system::error_code & ec)
  {
    return read_some_at_op_ec_{this, offset, buffer, ec};
  };
  virtual void async_read_some_at(std::uint64_t offset, asio::mutable_buffer buffer, read_handler h) = 0;
};

struct random_access_write_device : virtual execution_context
{
 protected:

  struct write_some_at_op_
  {
    random_access_write_device * impl;
    std::uint64_t offset;
    asio::const_buffer buffer;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_write_some_at(buffer, {h, result});
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

    random_access_write_device * impl;
    std::uint64_t offset;
    asio::const_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_write_some_at(buffer, ec, {h, result});
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
  [[nodiscard]] write_some_at_op_    write_some_at(std::uint64_t offset, asio::const_buffer buffer)
  {
    return write_some_at_op_{this, offset, buffer};
  }
  [[nodiscard]] write_some_at_op_ec_ write_someat(std::uint64_t offset, asio::const_buffer buffer, system::error_code & ec)
  {
    return write_some_at_op_ec_{this, offset, buffer, ec};
  };
  virtual void async_write_some_at(std::uint64_t offset, asio::const_buffer buffer, write_handler h) = 0;

};

struct random_access_device : random_access_read_device, random_access_write_device {};

struct waitable_device : virtual execution_context
{
  /// Wait types.
  using wait_type = asio::socket_base::wait_type;

 protected:

  struct wait_op_
  {

    waitable_device * impl;
    wait_type wt;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_wait(wt, {h, result});
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
    waitable_device * impl;
    wait_type wt;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_wait(wt, {h, result});
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

  virtual void async_wait(wait_type wt, boost::async::detail::completion_handler<system::error_code> h) = 0;
  [[nodiscard]] wait_op_    wait(wait_type wt)
  {
    return wait_op_{this, wt};
  }
  [[nodiscard]] wait_op_ec_ wait(wait_type wt, system::error_code & ec)
  {
    return wait_op_ec_{this, wt, ec};
  }
};

struct socket : asio::socket_base
{
  virtual bool native_non_blocking() const = 0;
  virtual void native_non_blocking(bool mode) = 0;
  virtual void native_non_blocking(bool mode, boost::system::error_code& ec) = 0;
  virtual bool non_blocking() const = 0;
  virtual void non_blocking(bool mode) = 0;
  virtual void non_blocking(bool mode, boost::system::error_code& ec) = 0;
  virtual void shutdown(shutdown_type what) = 0;
  virtual void shutdown(shutdown_type what, boost::system::error_code& ec) = 0;

 protected:

  struct send_op_
  {

    socket * impl;
    asio::const_buffer buffer;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_send(buffer, {h, result});
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

  struct send_op_ec_
  {
    socket * impl;
    asio::const_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_send(buffer, {h, result});
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


  struct send_with_flags_op_
  {
    socket * impl;
    asio::const_buffer buffer;
    message_flags flags;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_send(buffer, flags, {h, result});
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

  struct send_with_flags_op_ec_
  {
    socket * impl;
    asio::const_buffer buffer;
    message_flags flags;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_send(buffer, flags, {h, result});
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


  struct receive_op_
  {

    socket * impl;
    asio::mutable_buffer buffer;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_receive(buffer, {h, result});
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

  struct receive_op_ec_
  {
    socket * impl;
    asio::mutable_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_send(buffer, {h, result});
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


  struct receive_with_flags_op_
  {
    socket * impl;
    asio::mutable_buffer buffer;
    message_flags flags;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_receive(buffer, flags, {h, result});
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

  struct receive_with_flags_op_ec_
  {
    socket * impl;
    asio::mutable_buffer buffer;
    message_flags flags;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_receive(buffer, flags, {h, result});
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
  [[nodiscard]] send_op_    send(asio::const_buffer buffer)                          { return {this, buffer}; }
  [[nodiscard]] send_op_ec_ send(asio::const_buffer buffer, system::error_code & ec) { return {this, buffer, ec}; }
  [[nodiscard]] send_with_flags_op_    send(asio::const_buffer buffer, message_flags flags)                          { return {this, buffer, flags}; }
  [[nodiscard]] send_with_flags_op_ec_ send(asio::const_buffer buffer, message_flags flags, system::error_code & ec) { return {this, buffer, flags, ec}; }

  [[nodiscard]] receive_op_               receive(asio::mutable_buffer buffer)                          { return {this, buffer}; }
  [[nodiscard]] receive_op_ec_            receive(asio::mutable_buffer buffer, system::error_code & ec) { return {this, buffer, ec}; }
  [[nodiscard]] receive_with_flags_op_    receive(asio::mutable_buffer buffer, message_flags flags)                          { return {this, buffer, flags}; }
  [[nodiscard]] receive_with_flags_op_ec_ receive(asio::mutable_buffer buffer, message_flags flags, system::error_code & ec) { return {this, buffer, flags, ec}; }

  virtual void async_send(asio::const_buffer buffer,                      write_handler h) = 0;
  virtual void async_send(asio::const_buffer buffer, message_flags flags, write_handler h) = 0;
  virtual void async_receive(asio::mutable_buffer buffer,                      read_handler h) = 0;
  virtual void async_receive(asio::mutable_buffer buffer, message_flags flags, read_handler h) = 0;

  virtual ~socket() = default;
};


template<typename ... Args>
struct implements : virtual Args ...
{

};


}

#endif //BOOST_ASYNC_IO_CONCEPTS_HPP
