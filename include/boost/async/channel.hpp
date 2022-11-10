//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_CHANNEL_HPP
#define BOOST_ASYNC_CHANNEL_HPP

#include <boost/asio/experimental/channel.hpp>
#include <boost/async/detail/handler.hpp>
#include <optional>

namespace boost::async
{


/// A channel for messages.
/**
 * The basic_channel class template is used for sending messages between
 * different parts of the same application. A <em>message</em> is defined as a
 * collection of arguments to be passed to a completion handler, and the set of
 * messages supported by a channel is specified by its @c Traits and
 * <tt>Signatures...</tt> template parameters. Messages may be sent and received
 * using asynchronous or non-blocking synchronous operations.
 *
 * Unless customising the traits, applications will typically use the @c
 * experimental::channel alias template. For example:
 * @code void send_loop(int i, steady_timer& timer,
 *     channel<void(error_code, int)>& ch)
 * {
 *   if (i < 10)
 *   {
 *     timer.expires_after(chrono::seconds(1));
 *     timer.async_wait(
 *         [i, &timer, &ch](error_code error)
 *         {
 *           if (!error)
 *           {
 *             ch.async_send(error_code(), i,
 *                 [i, &timer, &ch](error_code error)
 *                 {
 *                   if (!error)
 *                   {
 *                     send_loop(i + 1, timer, ch);
 *                   }
 *                 });
 *           }
 *         });
 *   }
 *   else
 *   {
 *     ch.close();
 *   }
 * }
 *
 * void receive_loop(channel<void(error_code, int)>& ch)
 * {
 *   ch.async_receive(
 *       [&ch](error_code error, int i)
 *       {
 *         if (!error)
 *         {
 *           std::cout << "Received " << i << "\n";
 *           receive_loop(ch);
 *         }
 *       });
 * } @endcode
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * The basic_channel class template is not thread-safe, and would typically be
 * used for passing messages between application code that runs on the same
 * thread or in the same strand. Consider using @ref basic_concurrent_channel,
 * and its alias template @c experimental::concurrent_channel, to pass messages
 * between code running in different threads.
 */
template<typename T = void>
struct channel
{

  /// The underlying asio implementation type.
  typedef asio::experimental::channel<asio::io_context::executor_type, void(system::error_code, T)> implementation_type;

  /// The type of the executor associated with the channel.
  typedef typename asio::io_context::executor_type executor_type;

  /// Construct a basic_channel.
  /**
   * This constructor creates and channel.
   *
   * @param ex The I/O executor that the channel will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the channel.
   *
   * @param max_buffer_size The maximum number of messages that may be buffered
   * in the channel.
   */
  channel(const executor_type& ex, std::size_t max_buffer_size = 0) : impl_(ex, max_buffer_size)
  {
  }

  /// Construct and open a basic_channel.
  /**
   * This constructor creates and opens a channel.
   *
   * @param context An execution context which provides the I/O executor that
   * the channel will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the channel.
   *
   * @param max_buffer_size The maximum number of messages that may be buffered
   * in the channel.
   */
  channel(asio::io_context& context, std::size_t max_buffer_size = 0) : impl_(context, max_buffer_size) {}

  /// Move-construct a basic_channel from another.
  /**
   * This constructor moves a channel from one object to another.
   *
   * @param other The other basic_channel object from which the move will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_channel(const executor_type&) constructor.
   */
  channel(channel&& other) = default;
  /// Move-assign a basic_channel from another.
  /**
   * This assignment operator moves a channel from one object to another.
   * Cancels any outstanding asynchronous operations associated with the target
   * object.
   *
   * @param other The other basic_channel object from which the move will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_channel(const executor_type&)
   * constructor.
   */
  channel& operator=(channel&& other) = default;

  /// Destructor.
  ~channel() = default;

  /// Get the executor associated with the object.
  const executor_type& get_executor() noexcept
  {
    return impl_.get_executor();
  }

  /// Get the capacity of the channel's buffer.
  std::size_t capacity() noexcept
  {
    return impl_.capacity();
  }

  /// Determine whether the channel is open.
  bool is_open() const noexcept
  {
    return impl_.is_open();
  }

  /// Reset the channel to its initial state.
  void reset()
  {
    impl_.reset();
  }

  /// Close the channel.
  void close()
  {
    impl_.close();
  }

  /// Cancel all asynchronous operations waiting on the channel.
  /**
   * All outstanding send operations will complete with the error
   * @c boost::asio::experimental::error::channel_cancelled. Outstanding receive
   * operations complete with the result as determined by the channel traits.
   */
  void cancel()
  {
    impl_.cancel();
  }

  /// Determine whether a message can be received without blocking.
  bool ready() const noexcept
  {
    return impl_.ready();
  }

  bool   try_receive(std::optional<std::tuple<system::error_code, T>> &  result);
  void async_receive(boost::async::detail::completion_handler<system::error_code, T> h);

 protected:

  struct receive_op_
  {
    channel * chan;
    std::optional<std::tuple<system::error_code, T>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_receive(result);}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_receive({h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    T await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));

      return std::move(std::get<1>(*result));
    }
  };

  struct receive_op_ec_
  {
    channel * chan;
    system::error_code & ec;

    std::optional<std::tuple<system::error_code, T>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_receive(result);}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_receive({h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    T await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        ec = std::get<0>(*result);

      return std::move(std::get<1>(*result));
    }
  };
 public:
  [[nodiscard]] receive_op_    receive()                       {return {this};}
  [[nodiscard]] receive_op_ec_ receive(system::error_code &ec) {return {this, ec};}

  bool   try_send(T && value);
  void async_send(T && value, boost::async::detail::completion_handler<system::error_code> h);

 private:

  struct send_op_
  {
    channel * chan;
    T value;

    std::optional<std::tuple<system::error_code, T>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_send(std::move(value));}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_send(std::move(value), {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    T await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        throw system::system_error(std::get<0>(*result));

      return std::move(std::get<1>(*result));
    }
  };

  struct send_op_ec_
  {
    channel * chan;
    T value;
    system::error_code & ec;

    std::optional<std::tuple<system::error_code, T>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_send(std::move(value));}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_send(std::move(value), {h, result});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    T await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));

      if (std::get<0>(*result))
        ec = std::get<0>(*result);

      return std::move(std::get<1>(*result));
    }
  };
 public:


  [[nodiscard]] send_op_    send(T && t)                         {return {this, std::forward<T>(t)};}
  [[nodiscard]] send_op_ec_ send(T && t, system::error_code &ec) {return {this, std::forward<T>(t), ec};}


  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}
 private:
  implementation_type impl_;
};

template<typename T>
bool channel<T>::  try_receive(std::optional<std::tuple<system::error_code, T>> & result)
{
  return impl_.try_receive([&](system::error_code ec, T && value) { result.emplace(ec, std::move(value));});
}

template<typename T>
void channel<T>::async_receive(boost::async::detail::completion_handler<system::error_code, T> h)
{
  impl_.async_receive(std::move(h));
}

template<typename T>
bool channel<T>::  try_send(T && value)
{
  return impl_.try_send(system::error_code(), std::move(value));
}

template<typename T>
void channel<T>::async_send(T && value, boost::async::detail::completion_handler<system::error_code> h)
{
  impl_.async_send(system::error_code(), std::move(value), std::move(h));
}


template<>
struct channel<void>
{

  /// The underlying asio implementation type.
  typedef asio::experimental::channel<asio::io_context::executor_type, void(system::error_code)> implementation_type;

  /// The type of the executor associated with the channel.
  typedef typename asio::io_context::executor_type executor_type;

  /// Construct a basic_channel.
  /**
   * This constructor creates and channel.
   *
   * @param ex The I/O executor that the channel will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the channel.
   *
   * @param max_buffer_size The maximum number of messages that may be buffered
   * in the channel.
   */
  channel(const executor_type& ex, std::size_t max_buffer_size = 0) : impl_(ex, max_buffer_size)
  {
  }

  /// Construct and open a basic_channel.
  /**
   * This constructor creates and opens a channel.
   *
   * @param context An execution context which provides the I/O executor that
   * the channel will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the channel.
   *
   * @param max_buffer_size The maximum number of messages that may be buffered
   * in the channel.
   */
  channel(asio::io_context& context, std::size_t max_buffer_size = 0) : impl_(context, max_buffer_size) {}

  /// Move-construct a basic_channel from another.
  /**
   * This constructor moves a channel from one object to another.
   *
   * @param other The other basic_channel object from which the move will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_channel(const executor_type&) constructor.
   */
  channel(channel&& other) = default;
  /// Move-assign a basic_channel from another.
  /**
   * This assignment operator moves a channel from one object to another.
   * Cancels any outstanding asynchronous operations associated with the target
   * object.
   *
   * @param other The other basic_channel object from which the move will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c basic_channel(const executor_type&)
   * constructor.
   */
  channel& operator=(channel&& other) = default;

  /// Destructor.
  ~channel() = default;

  /// Get the executor associated with the object.
  const executor_type& get_executor() noexcept
  {
    return impl_.get_executor();
  }

  /// Get the capacity of the channel's buffer.
  std::size_t capacity() noexcept
  {
    return impl_.capacity();
  }

  /// Determine whether the channel is open.
  bool is_open() const noexcept
  {
    return impl_.is_open();
  }

  /// Reset the channel to its initial state.
  void reset()
  {
    impl_.reset();
  }

  /// Close the channel.
  void close()
  {
    impl_.close();
  }

  /// Cancel all asynchronous operations waiting on the channel.
  /**
   * All outstanding send operations will complete with the error
   * @c boost::asio::experimental::error::channel_cancelled. Outstanding receive
   * operations complete with the result as determined by the channel traits.
   */
  void cancel()
  {
    impl_.cancel();
  }

  /// Determine whether a message can be received without blocking.
  bool ready() const noexcept
  {
    return impl_.ready();
  }

  bool   try_receive(std::optional<std::tuple<system::error_code>> &  result);
  void async_receive(boost::async::detail::completion_handler<system::error_code> h);

 protected:

  struct receive_op_
  {
    channel * chan;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_receive(result);}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_receive({h, result});
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
    channel * chan;
    system::error_code & ec;

    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_receive(result);}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_receive({h, result});
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
        ec = std::get<0>(*result);
    }
  };
 public:
  [[nodiscard]] receive_op_    receive()                       {return {this};}
  [[nodiscard]] receive_op_ec_ receive(system::error_code &ec) {return {this, ec};}

  bool   try_send();
  void async_send(boost::async::detail::completion_handler<system::error_code> h);

 private:

  struct send_op_
  {
    channel * chan;

    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_send();}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_send({h, result});
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
    channel * chan;
    system::error_code & ec;

    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    bool await_ready() {return chan->try_send();}


    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        chan->async_send({h, result});
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
        ec = std::get<0>(*result);
    }
  };
 public:

  [[nodiscard]] send_op_    send()                       {return {this};}
  [[nodiscard]] send_op_ec_ send(system::error_code &ec) {return {this, ec};}


  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}
 private:
  implementation_type impl_;
};


}

#endif //BOOST_ASYNC_CHANNEL_HPP
