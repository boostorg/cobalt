//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SIGNAL_SET_HPP
#define BOOST_ASYNC_IO_SIGNAL_SET_HPP

#include <boost/asio/basic_signal_set.hpp>
#include <boost/async/io/concepts.hpp>

namespace boost::async::io
{

/// Provides signal functionality.
/**
 * The basic_signal_set class provides the ability to perform an asynchronous
 * wait for one or more signals to occur.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Example
 * Performing an asynchronous wait:
 * @code
 * void handler(
 *     const boost::system::error_code& error,
 *     int signal_number)
 * {
 *   if (!error)
 *   {
 *     // A signal occurred.
 *   }
 * }
 *
 * ...
 *
 * // Construct a signal set registered for process termination.
 * boost::asio::signal_set signals(my_context, SIGINT, SIGTERM);
 *
 * // Start an asynchronous wait for one of the signals to occur.
 * signals.async_wait(handler);
 * @endcode
 *
 * @par Queueing of signal notifications
 *
 * If a signal is registered with a signal_set, and the signal occurs when
 * there are no waiting handlers, then the signal notification is queued. The
 * next async_wait operation on that signal_set will dequeue the notification.
 * If multiple notifications are queued, subsequent async_wait operations
 * dequeue them one at a time. Signal notifications are dequeued in order of
 * ascending signal number.
 *
 * If a signal number is removed from a signal_set (using the @c remove or @c
 * erase member functions) then any queued notifications for that signal are
 * discarded.
 *
 * @par Multiple registration of signals
 *
 * The same signal number may be registered with different signal_set objects.
 * When the signal occurs, one handler is called for each signal_set object.
 *
 * Note that multiple registration only works for signals that are registered
 * using Asio. The application must not also register a signal handler using
 * functions such as @c signal() or @c sigaction().
 *
 * @par Signal masking on POSIX platforms
 *
 * POSIX allows signals to be blocked using functions such as @c sigprocmask()
 * and @c pthread_sigmask(). For signals to be delivered, programs must ensure
 * that any signals registered using signal_set objects are unblocked in at
 * least one thread.
 */
struct signal_set final : concepts::implements<concepts::cancellable>
{
  /// The underlying asio implementation type.
  typedef asio::basic_signal_set<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;

  /// Construct a signal set without adding any signals.
  /**
   * This constructor creates a signal set without registering for any signals.
   *
   * @param ex The I/O executor that the signal set will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * signal set.
   */
  explicit signal_set(const executor_type& ex);

  /// Construct a signal set without adding any signals.
  /**
   * This constructor creates a signal set without registering for any signals.
   *
   * @param context An execution context which provides the I/O executor that
   * the signal set will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the signal set.
   */
  explicit signal_set(asio::io_context& context);

  /// Construct a signal set and add one signal.
  /**
   * This constructor creates a signal set and registers for one signal.
   *
   * @param ex The I/O executor that the signal set will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * signal set.
   *
   * @param signal_number_1 The signal number to be added.
   *
   * @note This constructor is equivalent to performing:
   * @code boost::asio::signal_set signals(ex);
   * signals.add(signal_number_1); @endcode
   */
  signal_set(const executor_type& ex, int signal_number_1);

  /// Construct a signal set and add one signal.
  /**
   * This constructor creates a signal set and registers for one signal.
   *
   * @param context An execution context which provides the I/O executor that
   * the signal set will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the signal set.
   *
   * @param signal_number_1 The signal number to be added.
   *
   * @note This constructor is equivalent to performing:
   * @code boost::asio::signal_set signals(context);
   * signals.add(signal_number_1); @endcode
   */
  signal_set(asio::io_context& context, int signal_number_1);

  /// Construct a signal set and add two signals.
  /**
   * This constructor creates a signal set and registers for two signals.
   *
   * @param ex The I/O executor that the signal set will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * signal set.
   *
   * @param signal_number_1 The first signal number to be added.
   *
   * @param signal_number_2 The second signal number to be added.
   *
   * @note This constructor is equivalent to performing:
   * @code boost::asio::signal_set signals(ex);
   * signals.add(signal_number_1);
   * signals.add(signal_number_2); @endcode
   */
  signal_set(const executor_type& ex, int signal_number_1, int signal_number_2);

  /// Construct a signal set and add two signals.
  /**
   * This constructor creates a signal set and registers for two signals.
   *
   * @param context An execution context which provides the I/O executor that
   * the signal set will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the signal set.
   *
   * @param signal_number_1 The first signal number to be added.
   *
   * @param signal_number_2 The second signal number to be added.
   *
   * @note This constructor is equivalent to performing:
   * @code boost::asio::signal_set signals(context);
   * signals.add(signal_number_1);
   * signals.add(signal_number_2); @endcode
   */
  signal_set(asio::io_context& context, int signal_number_1,
                   int signal_number_2);

  /// Construct a signal set and add three signals.
  /**
   * This constructor creates a signal set and registers for three signals.
   *
   * @param ex The I/O executor that the signal set will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * signal set.
   *
   * @param signal_number_1 The first signal number to be added.
   *
   * @param signal_number_2 The second signal number to be added.
   *
   * @param signal_number_3 The third signal number to be added.
   *
   * @note This constructor is equivalent to performing:
   * @code boost::asio::signal_set signals(ex);
   * signals.add(signal_number_1);
   * signals.add(signal_number_2);
   * signals.add(signal_number_3); @endcode
   */
  signal_set(const executor_type& ex, int signal_number_1, int signal_number_2, int signal_number_3);

  /// Construct a signal set and add three signals.
  /**
   * This constructor creates a signal set and registers for three signals.
   *
   * @param context An execution context which provides the I/O executor that
   * the signal set will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the signal set.
   *
   * @param signal_number_1 The first signal number to be added.
   *
   * @param signal_number_2 The second signal number to be added.
   *
   * @param signal_number_3 The third signal number to be added.
   *
   * @note This constructor is equivalent to performing:
   * @code boost::asio::signal_set signals(context);
   * signals.add(signal_number_1);
   * signals.add(signal_number_2);
   * signals.add(signal_number_3); @endcode
   */
  signal_set(asio::io_context& context, int signal_number_1,
             int signal_number_2, int signal_number_3);

  /// Destroys the signal set.
  /**
   * This function destroys the signal set, cancelling any outstanding
   * asynchronous wait operations associated with the signal set as if by
   * calling @c cancel.
   */
  ~signal_set();
  /// Get the executor associated with the object.
  executor_type get_executor() noexcept;

  /// Add a signal to a signal_set.
  /**
   * This function adds the specified signal to the set. It has no effect if the
   * signal is already in the set.
   *
   * @param signal_number The signal to be added to the set.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void add(int signal_number);
  /// Add a signal to a signal_set.
  /**
   * This function adds the specified signal to the set. It has no effect if the
   * signal is already in the set.
   *
   * @param signal_number The signal to be added to the set.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void add(int signal_number, boost::system::error_code& ec);
  /// Remove a signal from a signal_set.
  /**
   * This function removes the specified signal from the set. It has no effect
   * if the signal is not in the set.
   *
   * @param signal_number The signal to be removed from the set.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note Removes any notifications that have been queued for the specified
   * signal number.
   */
  void remove(int signal_number);

  /// Remove a signal from a signal_set.
  /**
   * This function removes the specified signal from the set. It has no effect
   * if the signal is not in the set.
   *
   * @param signal_number The signal to be removed from the set.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Removes any notifications that have been queued for the specified
   * signal number.
   */
  void remove(int signal_number, boost::system::error_code& ec);
  /// Remove all signals from a signal_set.
  /**
   * This function removes all signals from the set. It has no effect if the set
   * is already empty.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note Removes all queued notifications.
   */
  void clear();

  /// Remove all signals from a signal_set.
  /**
   * This function removes all signals from the set. It has no effect if the set
   * is already empty.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Removes all queued notifications.
   */
  void clear(boost::system::error_code& ec);

  /// Cancel all operations associated with the signal set.
  /**
   * This function forces the completion of any pending asynchronous wait
   * operations against the signal set. The handler for each cancelled
   * operation will be invoked with the boost::asio::error::operation_aborted
   * error code.
   *
   * Cancellation does not alter the set of registered signals.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note If a registered signal occurred before cancel() is called, then the
   * handlers for asynchronous wait operations will:
   *
   * @li have already been invoked; or
   *
   * @li have been queued for invocation in the near future.
   *
   * These handlers can no longer be cancelled, and therefore are passed an
   * error code that indicates the successful completion of the wait operation.
   */
  void cancel() override;

  /// Cancel all operations associated with the signal set.
  /**
   * This function forces the completion of any pending asynchronous wait
   * operations against the signal set. The handler for each cancelled
   * operation will be invoked with the boost::asio::error::operation_aborted
   * error code.
   *
   * Cancellation does not alter the set of registered signals.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note If a registered signal occurred before cancel() is called, then the
   * handlers for asynchronous wait operations will:
   *
   * @li have already been invoked; or
   *
   * @li have been queued for invocation in the near future.
   *
   * These handlers can no longer be cancelled, and therefore are passed an
   * error code that indicates the successful completion of the wait operation.
   */
  void cancel(boost::system::error_code& ec) override;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}
 private:
  struct wait_op_
  {
    implementation_type &  impl;
    std::optional<std::tuple<system::error_code, int>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    void await_resume_impl(boost::async::completion_handler<system::error_code, int> h);

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        await_resume_impl( {h, result});
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
    implementation_type & impl;
    system::error_code & ec;
    std::optional<std::tuple<int>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}
    void await_resume_impl(boost::async::completion_handler<int> h);

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        await_resume_impl({h, result});
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
    }
  };
 public:


  /// Start an asynchronous operation to wait for a signal to be delivered.
  /**
   * This function may be used to initiate an asynchronous wait against the
   * signal set. It is an initiating function for an @ref
   * asynchronous_operation, and always returns immediately.
   *
   * For each call to async_wait(), the completion handler will be called
   * exactly once. The completion handler will be called when:
   *
   * @li One of the registered signals in the signal set occurs; or
   *
   * @li The signal set was cancelled, in which case the handler is passed the
   * error code boost::asio::error::operation_aborted.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the wait completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const boost::system::error_code& error, // Result of operation.
   *   int signal_number // Indicates which signal occurred.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using boost::asio::post().
   *
   * @par Completion Signature
   * @code void(boost::system::error_code, int) @endcode
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following
   * boost::asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * @li @c cancellation_type::total
   */
  wait_op_ wait()
  {
    return {impl_};

  }

  /// \overload wait()
  wait_op_ec_ wait(system::error_code & ec)
  {
    return {impl_, ec};

  }

 private:
  implementation_type impl_;
};

}

#endif //BOOST_ASYNC_IO_SIGNAL_SET_HPP
