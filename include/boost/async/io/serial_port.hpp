//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SERIAL_PORT_HPP
#define BOOST_ASYNC_IO_SERIAL_PORT_HPP

#include <boost/asio/basic_serial_port.hpp>
#include <boost/async/io/concepts.hpp>

namespace boost::async::io
{


/// Provides serial port functionality.
/**
 * The serial_port class provides a wrapper over serial port
 * functionality.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
struct serial_port final
    : concepts::implements<concepts::cancellable, concepts::closable, concepts::stream>,
      asio::serial_port_base
{
  /// The underlying asio implementation type.
  typedef asio::basic_serial_port<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;

  /// The native representation of a serial port.
  typedef implementation_type::native_handle_type native_handle_type;

  /// A basic_serial_port is always the lowest layer.
  typedef implementation_type lowest_layer_type;

  /// Construct a serial_port without opening it.
  /**
   * This constructor creates a serial port without opening it.
   *
   * @param ex The I/O executor that the serial port will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * serial port.
   */
  explicit serial_port(const executor_type& ex);

  /// Construct a serial_port without opening it.
  /**
   * This constructor creates a serial port without opening it.
   *
   * @param context An execution context which provides the I/O executor that
   * the serial port will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the serial port.
   */
  explicit serial_port(asio::io_context& context);

  /// Construct and open a serial_port.
  /**
   * This constructor creates and opens a serial port for the specified device
   * name.
   *
   * @param ex The I/O executor that the serial port will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * serial port.
   *
   * @param device The platform-specific device name for this serial
   * port.
   */
  serial_port(const executor_type& ex, const char* device);

  /// Construct and open a serial_port.
  /**
   * This constructor creates and opens a serial port for the specified device
   * name.
   *
   * @param context An execution context which provides the I/O executor that
   * the serial port will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the serial port.
   *
   * @param device The platform-specific device name for this serial
   * port.
   */
  serial_port(asio::io_context& context, const char* device);

  /// Construct and open a serial_port.
  /**
   * This constructor creates and opens a serial port for the specified device
   * name.
   *
   * @param ex The I/O executor that the serial port will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * serial port.
   *
   * @param device The platform-specific device name for this serial
   * port.
   */
  serial_port(const executor_type& ex, const std::string& device);
  /// Construct and open a serial_port.
  /**
   * This constructor creates and opens a serial port for the specified device
   * name.
   *
   * @param context An execution context which provides the I/O executor that
   * the serial port will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the serial port.
   *
   * @param device The platform-specific device name for this serial
   * port.
   */
  serial_port(asio::io_context& context, const std::string& device);

  /// Construct a serial_port on an existing native serial port.
  /**
   * This constructor creates a serial port object to hold an existing native
   * serial port.
   *
   * @param ex The I/O executor that the serial port will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * serial port.
   *
   * @param native_serial_port A native serial port.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  serial_port(const executor_type& ex,
              const native_handle_type& native_serial_port);

  /// Construct a serial_port on an existing native serial port.
  /**
   * This constructor creates a serial port object to hold an existing native
   * serial port.
   *
   * @param context An execution context which provides the I/O executor that
   * the serial port will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the serial port.
   *
   * @param native_serial_port A native serial port.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  serial_port(asio::io_context& context,
              const native_handle_type& native_serial_port);

  /// Move-construct a serial_port from another.
  /**
   * This constructor moves a serial port from one object to another.
   *
   * @param other The other serial_port object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c serial_port(const executor_type&)
   * constructor.
   */
  serial_port(serial_port&& other);

  /// Move-assign a serial_port from another.
  /**
   * This assignment operator moves a serial port from one object to another.
   *
   * @param other The other serial_port object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c serial_port(const executor_type&)
   * constructor.
   */
  serial_port& operator=(serial_port&& other);
  /// Destroys the serial port.
  /**
   * This function destroys the serial port, cancelling any outstanding
   * asynchronous wait operations associated with the serial port as if by
   * calling @c cancel.
   */
  ~serial_port();

  /// Get the executor associated with the object.
  executor_type get_executor() noexcept override;

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * layers. Since a serial_port cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A reference to the lowest layer in the stack of layers. Ownership
   * is not transferred to the caller.
   */
  lowest_layer_type& lowest_layer();

  /// Get a const reference to the lowest layer.
  /**
   * This function returns a const reference to the lowest layer in a stack of
   * layers. Since a serial_port cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A const reference to the lowest layer in the stack of layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type& lowest_layer() const;

  /// Open the serial port using the specified device name.
  /**
   * This function opens the serial port for the specified device name.
   *
   * @param device The platform-specific device name.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void open(const std::string& device);

  /// Open the serial port using the specified device name.
  /**
   * This function opens the serial port using the given platform-specific
   * device name.
   *
   * @param device The platform-specific device name.
   *
   * @param ec Set the indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID open(const std::string& device,
                               boost::system::error_code& ec);

  /// Assign an existing native serial port to the serial port.
  /*
   * This function opens the serial port to hold an existing native serial port.
   *
   * @param native_serial_port A native serial port.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void assign(const native_handle_type& native_serial_port);

  /// Assign an existing native serial port to the serial port.
  /*
   * This function opens the serial port to hold an existing native serial port.
   *
   * @param native_serial_port A native serial port.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID assign(const native_handle_type& native_serial_port,
                                 boost::system::error_code& ec);

  /// Determine whether the serial port is open.
  bool is_open() const override ;
  /// Close the serial port.
  /**
   * This function is used to close the serial port. Any asynchronous read or
   * write operations will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void close() override ;

  /// Close the serial port.
  /**
   * This function is used to close the serial port. Any asynchronous read or
   * write operations will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void close(boost::system::error_code& ec) override;

  /// Get the native serial port representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * serial port. This is intended to allow access to native serial port
   * functionality that is not otherwise provided.
   */
  native_handle_type native_handle();

  /// Cancel all asynchronous operations associated with the serial port.
  /**
   * This function causes all outstanding asynchronous read or write operations
   * to finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void cancel() override;

  /// Cancel all asynchronous operations associated with the serial port.
  /**
   * This function causes all outstanding asynchronous read or write operations
   * to finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void cancel(boost::system::error_code& ec) override;

  /// Send a break sequence to the serial port.
  /**
   * This function causes a break sequence of platform-specific duration to be
   * sent out the serial port.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void send_break();

  /// Send a break sequence to the serial port.
  /**
   * This function causes a break sequence of platform-specific duration to be
   * sent out the serial port.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void send_break(boost::system::error_code& ec);

  /// Set an option on the serial port.
  /**
   * This function is used to set an option on the serial port.
   *
   * @param option The option value to be set on the serial port.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @sa SettableSerialPortOption @n
   * boost::asio::serial_port_base::baud_rate @n
   * boost::asio::serial_port_base::flow_control @n
   * boost::asio::serial_port_base::parity @n
   * boost::asio::serial_port_base::stop_bits @n
   * boost::asio::serial_port_base::character_size
   */
  template <typename SettableSerialPortOption>
  void set_option(const SettableSerialPortOption& option);

  /// Set an option on the serial port.
  /**
   * This function is used to set an option on the serial port.
   *
   * @param option The option value to be set on the serial port.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa SettableSerialPortOption @n
   * boost::asio::serial_port_base::baud_rate @n
   * boost::asio::serial_port_base::flow_control @n
   * boost::asio::serial_port_base::parity @n
   * boost::asio::serial_port_base::stop_bits @n
   * boost::asio::serial_port_base::character_size
   */
  template <typename SettableSerialPortOption>
  BOOST_ASIO_SYNC_OP_VOID set_option(const SettableSerialPortOption& option,
                                     boost::system::error_code& ec);

  /// Get an option from the serial port.
  /**
   * This function is used to get the current value of an option on the serial
   * port.
   *
   * @param option The option value to be obtained from the serial port.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @sa GettableSerialPortOption @n
   * boost::asio::serial_port_base::baud_rate @n
   * boost::asio::serial_port_base::flow_control @n
   * boost::asio::serial_port_base::parity @n
   * boost::asio::serial_port_base::stop_bits @n
   * boost::asio::serial_port_base::character_size
   */
  template <typename GettableSerialPortOption>
  void get_option(GettableSerialPortOption& option) const;

  /// Get an option from the serial port.
  /**
   * This function is used to get the current value of an option on the serial
   * port.
   *
   * @param option The option value to be obtained from the serial port.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa GettableSerialPortOption @n
   * boost::asio::serial_port_base::baud_rate @n
   * boost::asio::serial_port_base::flow_control @n
   * boost::asio::serial_port_base::parity @n
   * boost::asio::serial_port_base::stop_bits @n
   * boost::asio::serial_port_base::character_size
   */
  template <typename GettableSerialPortOption>
  BOOST_ASIO_SYNC_OP_VOID get_option(GettableSerialPortOption& option,
                                     boost::system::error_code& ec) const;

  /// Start an asynchronous write.
  /**
   * This function is used to asynchronously write data to the serial port.
   * It is an initiating function for an @ref asynchronous_operation, and always
   * returns immediately.
   *
   * @param buffers One or more data buffers to be written to the serial port.
   * Although the buffers object may be copied as necessary, ownership of the
   * underlying memory blocks is retained by the caller, which must guarantee
   * that they remain valid until the completion handler is called.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the write completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const boost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred // Number of bytes written.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using boost::asio::post().
   *
   * @par Completion Signature
   * @code void(boost::system::error_code, std::size_t) @endcode
   *
   * @note The write operation may not transmit all of the data to the peer.
   * Consider using the @ref async_write function if you need to ensure that all
   * data is written before the asynchronous operation completes.
   *
   * @par Example
   * To write a single data buffer use the @ref buffer function as follows:
   * @code
   * serial_port.async_write_some(
   *     boost::asio::buffer(data, size), handler);
   * @endcode
   * See the @ref buffer documentation for information on writing multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   *
   * @par Per-Operation Cancellation
   * On POSIX or Windows operating systems, this asynchronous operation supports
   * cancellation for the following boost::asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * @li @c cancellation_type::total
   */
  void async_write_some(asio::const_buffer buffer, concepts::write_handler h) final;
  void async_write_some(prepared_buffers, concepts::write_handler h) final;
  void async_write_some(any_const_buffer_range buffer, concepts::write_handler h) final;

  /// Start an asynchronous read.
  /**
   * This function is used to asynchronously read data from the serial port.
   * It is an initiating function for an @ref asynchronous_operation, and always
   * returns immediately.
   *
   * @param buffers One or more buffers into which the data will be read.
   * Although the buffers object may be copied as necessary, ownership of the
   * underlying memory blocks is retained by the caller, which must guarantee
   * that they remain valid until the completion handler is called.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the read completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const boost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred // Number of bytes read.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using boost::asio::post().
   *
   * @par Completion Signature
   * @code void(boost::system::error_code, std::size_t) @endcode
   *
   * @note The read operation may not read all of the requested number of bytes.
   * Consider using the @ref async_read function if you need to ensure that the
   * requested amount of data is read before the asynchronous operation
   * completes.
   *
   * @par Example
   * To read into a single data buffer use the @ref buffer function as follows:
   * @code
   * serial_port.async_read_some(
   *     boost::asio::buffer(data, size), handler);
   * @endcode
   * See the @ref buffer documentation for information on reading into multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   *
   * @par Per-Operation Cancellation
   * On POSIX or Windows operating systems, this asynchronous operation supports
   * cancellation for the following boost::asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * @li @c cancellation_type::total
   */
  void async_read_some(asio::mutable_buffer buffer,                     concepts::write_handler h) final;
  void async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::write_handler h) final;
  void async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::write_handler h) final;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  implementation_type impl_;
};

}

#endif //BOOST_ASYNC_IO_SERIAL_PORT_HPP
