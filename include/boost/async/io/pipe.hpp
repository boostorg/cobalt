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
struct readable_pipe : concepts::implements<concepts::cancellable, concepts::closable, concepts::read_stream>
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
  executor_type get_executor() noexcept;

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
  bool is_open() const;

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
  void cancel();

  /// Cancel all asynchronous operations associated with the pipe.
  /**
   * This function causes all outstanding asynchronous read operations to finish
   * immediately, and the handlers for cancelled operations will be passed the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void cancel(boost::system::error_code &ec);

  /// Read some data from the pipe.
  /**
   * This function is used to read data from the pipe. The function call will
   * block until one or more bytes of data has been read successfully, or until
   * an error occurs.
   *
   * @param buffers One or more buffers into which the data will be read.
   *
   * @returns The number of bytes read.
   *
   * @throws boost::system::system_error Thrown on failure. An error code of
   * boost::asio::error::eof indicates that the connection was closed by the
   * peer.
   *
   * @note The read_some operation may not read all of the requested number of
   * bytes. Consider using the @ref read function if you need to ensure that
   * the requested amount of data is read before the blocking operation
   * completes.
   *
   * @par Example
   * To read into a single data buffer use the @ref buffer function as follows:
   * @code
   * readable_pipe.read_some(boost::asio::buffer(data, size));
   * @endcode
   * See the @ref buffer documentation for information on reading into multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   */
  read_some_op_ read_some(asio::mutable_buffer buffer) override
  {
    return {&impl_, buffer, read_some_op_impl};

  }

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
  read_some_op_ec_ read_some(asio::mutable_buffer buffer, system::error_code &ec) override
  {
    return {&impl_, buffer, ec, read_some_op_impl};
  }

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  struct read_some_op_impl_ final : read_some_op_base
  {
    void await_suspend(void * p, asio::mutable_buffer buffer,
                      boost::async::detail::completion_handler<system::error_code, std::size_t> h) const final;
  };

  constexpr static read_some_op_impl_ read_some_op_impl{};


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
struct writable_pipe  : concepts::implements<concepts::cancellable, concepts::closable, concepts::write_stream>
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
  bool is_open() const;

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

 private:
  struct write_some_op_impl_ final : write_some_op_base
  {
    void await_suspend(void * p, asio::const_buffer buffer,
                      boost::async::detail::completion_handler<system::error_code, std::size_t> h) const final;
  };

  constexpr static write_some_op_impl_ write_some_op_impl{};

 public:

  /// Write some data to the pipe.
  /**
   * This function is used to write data to the pipe. The function call will
   * block until one or more bytes of the data has been written successfully,
   * or until an error occurs.
   *
   * @param buffers One or more data buffers to be written to the pipe.
   *
   * @returns The number of bytes written.
   *
   * @throws boost::system::system_error Thrown on failure. An error code of
   * boost::asio::error::eof indicates that the connection was closed by the
   * peer.
   *
   * @note The write_some operation may not transmit all of the data to the
   * peer. Consider using the @ref write function if you need to ensure that
   * all data is written before the blocking operation completes.
   *
   * @par Example
   * To write a single data buffer use the @ref buffer function as follows:
   * @code
   * pipe.write_some(boost::asio::buffer(data, size));
   * @endcode
   * See the @ref buffer documentation for information on writing multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   */
  write_some_op_ write_some(asio::const_buffer buffers) override
  {
    return {&impl_, buffers, write_some_op_impl};
  }

  /// Write some data to the pipe.
  /**
   * This function is used to write data to the pipe. The function call will
   * block until one or more bytes of the data has been written successfully,
   * or until an error occurs.
   *
   * @param buffers One or more data buffers to be written to the pipe.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The number of bytes written. Returns 0 if an error occurred.
   *
   * @note The write_some operation may not transmit all of the data to the
   * peer. Consider using the @ref write function if you need to ensure that
   * all data is written before the blocking operation completes.
   */
  write_some_op_ec_ write_some(asio::const_buffer buffers,
                                    boost::system::error_code& ec) override
  {
    return {&impl_, buffers, ec, write_some_op_impl};
  }

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
