//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_PIPE_HPP
#define BOOST_ASYNC_IO_PIPE_HPP

#include <boost/asio/readable_pipe.hpp>
#include <boost/asio/writable_pipe.hpp>
#include <boost/asio/connect_pipe.hpp>
#include <boost/async/io/concepts.hpp>

namespace boost::async::io
{


/// Provides pipe functionality.
/**
 * The readable_pipe class provides a wrapper over pipe
 * functionality.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
struct readable_pipe final : concepts::implements<concepts::cancellable, concepts::closable, concepts::read_stream>
{
  /// The underlying asio implementation type.
  typedef asio::basic_readable_pipe<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;

  /// The native representation of a pipe.
  typedef implementation_type::native_handle_type native_handle_type;

  /// A readable_pipe is always the lowest layer.
  typedef implementation_type lowest_layer_type;

  /// Construct a readable_pipe without opening it.
  /**
   * This constructor creates a pipe without opening it.
   *
   * @param ex The I/O executor that the pipe will use, by default, to dispatch
   * handlers for any asynchronous operations performed on the pipe.
   */
  explicit readable_pipe(const executor_type &ex);
  /// Construct a readable_pipe without opening it.
  /**
   * This constructor creates a pipe without opening it.
   *
   * @param context An execution context which provides the I/O executor that
   * the pipe will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the pipe.
   */
  explicit readable_pipe(asio::io_context &context);

  /// Construct a readable_pipe on an existing native pipe.
  /**
   * This constructor creates a pipe object to hold an existing native
   * pipe.
   *
   * @param ex The I/O executor that the pipe will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  readable_pipe(const executor_type &ex,
                const native_handle_type &native_pipe);

  /// Construct a readable_pipe on an existing native pipe.
  /**
   * This constructor creates a pipe object to hold an existing native
   * pipe.
   *
   * @param context An execution context which provides the I/O executor that
   * the pipe will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  readable_pipe(asio::io_context &context,
                const native_handle_type &native_pipe);

  /// Move-construct a readable_pipe from another.
  /**
   * This constructor moves a pipe from one object to another.
   *
   * @param other The other readable_pipe object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c readable_pipe(const executor_type&)
   * constructor.
   */
  readable_pipe(readable_pipe &&other);

  /// Move-assign a readable_pipe from another.
  /**
   * This assignment operator moves a pipe from one object to another.
   *
   * @param other The other readable_pipe object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c readable_pipe(const executor_type&)
   * constructor.
   */
  readable_pipe &operator=(readable_pipe &&other);

  /// Destroys the pipe.
  /**
   * This function destroys the pipe, cancelling any outstanding
   * asynchronous wait operations associated with the pipe as if by
   * calling @c cancel.
   */
  ~readable_pipe();

  /// Get the executor associated with the object.
  executor_type get_executor() noexcept override;

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * layers. Since a readable_pipe cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A reference to the lowest layer in the stack of layers. Ownership
   * is not transferred to the caller.
   */
  lowest_layer_type &lowest_layer();

  /// Get a const reference to the lowest layer.
  /**
   * This function returns a const reference to the lowest layer in a stack of
   * layers. Since a readable_pipe cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A const reference to the lowest layer in the stack of layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type &lowest_layer() const;

  /// Assign an existing native pipe to the pipe.
  /**
   * This function opens the pipe to hold an existing native pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void assign(const native_handle_type &native_pipe);

  /// Assign an existing native pipe to the pipe.
  /**
   * This function opens the pipe to hold an existing native pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void assign(const native_handle_type &native_pipe,
              boost::system::error_code &ec);

  /// Determine whether the pipe is open.
  bool is_open() const override;

  /// Close the pipe.
  /**
   * This function is used to close the pipe. Any asynchronous read operations
   * will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void close() override;

  /// Close the pipe.
  /**
   * This function is used to close the pipe. Any asynchronous read operations
   * will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void close(boost::system::error_code &ec) override;

  /// Release ownership of the underlying native pipe.
  /**
   * This function causes all outstanding asynchronous read operations to
   * finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error. Ownership of the
   * native pipe is then transferred to the caller.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with boost::asio::error::operation_not_supported on
   * these platforms.
   */
  native_handle_type release();

  /// Release ownership of the underlying native pipe.
  /**
   * This function causes all outstanding asynchronous read operations to
   * finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error. Ownership of the
   * native pipe is then transferred to the caller.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with boost::asio::error::operation_not_supported on
   * these platforms.
   */
  native_handle_type release(boost::system::error_code &ec);

  /// Get the native pipe representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * pipe. This is intended to allow access to native pipe
   * functionality that is not otherwise provided.
   */
  native_handle_type native_handle();

  /// Cancel all asynchronous operations associated with the pipe.
  /**
   * This function causes all outstanding asynchronous read operations to finish
   * immediately, and the handlers for cancelled operations will be passed the
   * boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void cancel() override;

  /// Cancel all asynchronous operations associated with the pipe.
  /**
   * This function causes all outstanding asynchronous read operations to finish
   * immediately, and the handlers for cancelled operations will be passed the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void cancel(boost::system::error_code &ec) override;

  /// Read some data from the pipe.
  /**
   * This function is used to read data from the pipe. The function call will
   * block until one or more bytes of data has been read successfully, or until
   * an error occurs.
   *
   * @param buffers One or more buffers into which the data will be read.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The number of bytes read. Returns 0 if an error occurred.
   *
   * @note The read_some operation may not read all of the requested number of
   * bytes. Consider using the @ref read function if you need to ensure that
   * the requested amount of data is read before the blocking operation
   * completes.
   */
  void async_read_some(asio::mutable_buffer buffer,                     concepts::write_handler h) final;
  void async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::write_handler h) final;
  void async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::write_handler h) final;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  implementation_type impl_;
};


/// Provides pipe functionality.
/**
 * The writable_pipe class provides a wrapper over pipe
 * functionality.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
struct writable_pipe final : concepts::implements<concepts::cancellable, concepts::closable, concepts::write_stream>
{
  /// The underlying asio implementation type.
  typedef asio::basic_writable_pipe<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;

  /// The native representation of a pipe.
  typedef implementation_type::native_handle_type native_handle_type;

  /// A writable_pipe is always the lowest layer.
  typedef implementation_type lowest_layer_type;

  /// Construct a writable_pipe without opening it.
  /**
   * This constructor creates a pipe without opening it.
   *
   * @param ex The I/O executor that the pipe will use, by default, to dispatch
   * handlers for any asynchronous operations performed on the pipe.
   */
  explicit writable_pipe(const executor_type& ex);

  /// Construct a writable_pipe without opening it.
  /**
   * This constructor creates a pipe without opening it.
   *
   * @param context An execution context which provides the I/O executor that
   * the pipe will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the pipe.
   */
  explicit writable_pipe(asio::io_context& context);

  /// Construct a writable_pipe on an existing native pipe.
  /**
   * This constructor creates a pipe object to hold an existing native
   * pipe.
   *
   * @param ex The I/O executor that the pipe will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  writable_pipe(const executor_type& ex,
                const native_handle_type& native_pipe);
  /// Construct a writable_pipe on an existing native pipe.
  /**
   * This constructor creates a pipe object to hold an existing native
   * pipe.
   *
   * @param context An execution context which provides the I/O executor that
   * the pipe will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  writable_pipe(asio::io_context& context,
                      const native_handle_type& native_pipe);

  /// Move-construct a writable_pipe from another.
  /**
   * This constructor moves a pipe from one object to another.
   *
   * @param other The other writable_pipe object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c writable_pipe(const executor_type&)
   * constructor.
   */
  writable_pipe(writable_pipe&& other);

  /// Move-assign a writable_pipe from another.
  /**
   * This assignment operator moves a pipe from one object to another.
   *
   * @param other The other writable_pipe object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c writable_pipe(const executor_type&)
   * constructor.
   */
  writable_pipe& operator=(writable_pipe&& other);

  /// Destroys the pipe.
  /**
   * This function destroys the pipe, cancelling any outstanding
   * asynchronous wait operations associated with the pipe as if by
   * calling @c cancel.
   */
  ~writable_pipe();
  /// Get the executor associated with the object.
  executor_type get_executor() noexcept override;

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * layers. Since a writable_pipe cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A reference to the lowest layer in the stack of layers. Ownership
   * is not transferred to the caller.
   */
  lowest_layer_type& lowest_layer();

  /// Get a const reference to the lowest layer.
  /**
   * This function returns a const reference to the lowest layer in a stack of
   * layers. Since a writable_pipe cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A const reference to the lowest layer in the stack of layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type& lowest_layer() const;

  /// Assign an existing native pipe to the pipe.
  /**
   * This function opens the pipe to hold an existing native pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void assign(const native_handle_type& native_pipe);

  /// Assign an existing native pipe to the pipe.
  /**
   * This function opens the pipe to hold an existing native pipe.
   *
   * @param native_pipe A native pipe.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void assign(const native_handle_type& native_pipe,
              boost::system::error_code& ec);

  /// Determine whether the pipe is open.
  bool is_open() const override;

  /// Close the pipe.
  /**
   * This function is used to close the pipe. Any asynchronous write operations
   * will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void close() override;

  /// Close the pipe.
  /**
   * This function is used to close the pipe. Any asynchronous write operations
   * will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void close(boost::system::error_code& ec) override;
  /// Release ownership of the underlying native pipe.
  /**
   * This function causes all outstanding asynchronous write operations to
   * finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error. Ownership of the
   * native pipe is then transferred to the caller.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with boost::asio::error::operation_not_supported on
   * these platforms.
   */
  native_handle_type release();

  /// Release ownership of the underlying native pipe.
  /**
   * This function causes all outstanding asynchronous write operations to
   * finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error. Ownership of the
   * native pipe is then transferred to the caller.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with boost::asio::error::operation_not_supported on
   * these platforms.
   */
  native_handle_type release(boost::system::error_code& ec);

  /// Get the native pipe representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * pipe. This is intended to allow access to native pipe
   * functionality that is not otherwise provided.
   */
  native_handle_type native_handle();

  /// Cancel all asynchronous operations associated with the pipe.
  /**
   * This function causes all outstanding asynchronous write operations to
   * finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void cancel() override;

  /// Cancel all asynchronous operations associated with the pipe.
  /**
   * This function causes all outstanding asynchronous write operations to
   * finish immediately, and the handlers for cancelled operations will be
   * passed the boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void cancel(boost::system::error_code& ec) override;

  void async_write_some(asio::const_buffer buffer, concepts::write_handler h) final;
  void async_write_some(prepared_buffers, concepts::write_handler h) final;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  implementation_type impl_;
};


/// Connect two pipe ends using an anonymous pipe.
/**
 * @param read_end The read end of the pipe.
 *
 * @param write_end The write end of the pipe.
 *
 * @throws boost::system::system_error Thrown on failure.
 */
void connect_pipe(readable_pipe& read_end,
                  writable_pipe& write_end);

/// Connect two pipe ends using an anonymous pipe.
/**
 * @param read_end The read end of the pipe.
 *
 * @param write_end The write end of the pipe.
 *
 * @throws boost::system::system_error Thrown on failure.
 *
 * @param ec Set to indicate what error occurred, if any.
 */
void connect_pipe(readable_pipe& read_end,
                  writable_pipe& write_end,
                  boost::system::error_code& ec);


/// Connect two pipe ends using an anonymous pipe.
/// And return them as a pair.
/**
 * @param executor The executor tbe used by the pipes.
 *
 * @throws boost::system::system_error Thrown on failure.
 */
auto connect_pipe(asio::io_context::executor_type executor = this_thread::get_executor())
                  -> std::pair<readable_pipe, writable_pipe>;

/// Connect two pipe ends using an anonymous pipe.
/**
 * @param executor The executor tbe used by the pipes.
 *
 * @param ec Set to indicate what error occurred, if any.
 */
auto connect_pipe(asio::io_context::executor_type executor,
                  boost::system::error_code& ec) -> std::pair<readable_pipe, writable_pipe>;

}

#endif //BOOST_ASYNC_IO_PIPE_HPP
