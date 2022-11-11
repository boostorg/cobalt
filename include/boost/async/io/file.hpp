//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_FILE_HPP
#define BOOST_ASYNC_IO_FILE_HPP

#if defined(BOOST_ASIO_HAS_FILE)

#include <boost/asio/stream_file.hpp>
#include <boost/asio/random_access_file.hpp>
#include <boost/async/io/concepts.hpp>

namespace boost::async::io
{


/// Provides file functionality.
/**
 * The file class template provides functionality that is common to both
 * ssl-oriented and random-access files.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
struct file :
    asio::file_base,
    concepts::implements<concepts::cancellable, concepts::closable, concepts::execution_context>
{
  /// The underlying asio implementation type.
  typedef asio::basic_file<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;

  /// Get the executor associated with the object.
  executor_type get_executor() noexcept override ;


  /// The native representation of a file.
  typedef implementation_type::native_handle_type native_handle_type;

  /// Move-construct a file from another.
  /**
   * This constructor moves a file from one object to another.
   *
   * @param other The other file object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c file(const executor_type&) constructor.
   */
  file(file&& other);

  /// Move-assign a file from another.
  /**
   * This assignment operator moves a file from one object to another.
   *
   * @param other The other file object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c file(const executor_type&) constructor.
   */
  file& operator=(file&& other);

  /// Open the file using the specified path.
  /**
   * This function opens the file so that it will use the specified path.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * boost::asio::stream_file file(my_context);
   * file.open("/path/to/my/file", boost::asio::stream_file::read_only);
   * @endcode
   */
  void open(const char* path, file_base::flags open_flags);

  /// Open the file using the specified path.
  /**
   * This function opens the file so that it will use the specified path.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * @code
   * boost::asio::stream_file file(my_context);
   * boost::system::error_code ec;
   * file.open("/path/to/my/file", boost::asio::stream_file::read_only, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  BOOST_ASIO_SYNC_OP_VOID open(const char* path,
                               file_base::flags open_flags, boost::system::error_code& ec);

  /// Open the file using the specified path.
  /**
   * This function opens the file so that it will use the specified path.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @par Example
   * @code
   * boost::asio::stream_file file(my_context);
   * file.open("/path/to/my/file", boost::asio::stream_file::read_only);
   * @endcode
   */
  void open(const std::string& path, file_base::flags open_flags);

  /// Open the file using the specified path.
  /**
   * This function opens the file so that it will use the specified path.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @par Example
   * @code
   * boost::asio::stream_file file(my_context);
   * boost::system::error_code ec;
   * file.open("/path/to/my/file", boost::asio::stream_file::read_only, ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  BOOST_ASIO_SYNC_OP_VOID open(const std::string& path, file_base::flags open_flags, boost::system::error_code& ec);

  /// Assign an existing native file to the file.
  /**
   * This function opens the file to hold an existing native file.
   *
   * @param native_file A native file.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void assign(const native_handle_type& native_file);

  /// Assign an existing native file to the file.
  /**
   * This function opens the file to hold an existing native file.
   *
   * @param native_file A native file.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  void assign(const native_handle_type& native_file, boost::system::error_code& ec);

  /// Determine whether the file is open.
  bool is_open() const override;
  /// Close the file.
  /**
   * This function is used to close the file. Any asynchronous read or write
   * operations will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure. Note that, even if
   * the function indicates an error, the underlying descriptor is closed.
   */
  void close() override;

  /// Close the file.
  /**
   * This function is used to close the file. Any asynchronous read or write
   * operations will be cancelled immediately, and will complete with the
   * boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any. Note that, even if
   * the function indicates an error, the underlying descriptor is closed.
   *
   * @par Example
   * @code
   * boost::asio::stream_file file(my_context);
   * ...
   * boost::system::error_code ec;
   * file.close(ec);
   * if (ec)
   * {
   *   // An error occurred.
   * }
   * @endcode
   */
  void close(boost::system::error_code& ec) override;

  /// Release ownership of the underlying native file.
  /**
   * This function causes all outstanding asynchronous read and write
   * operations to finish immediately, and the handlers for cancelled
   * operations will be passed the boost::asio::error::operation_aborted error.
   * Ownership of the native file is then transferred to the caller.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with boost::asio::error::operation_not_supported on
   * these platforms.
   */
  native_handle_type release();

  /// Release ownership of the underlying native file.
  /**
   * This function causes all outstanding asynchronous read and write
   * operations to finish immediately, and the handlers for cancelled
   * operations will be passed the boost::asio::error::operation_aborted error.
   * Ownership of the native file is then transferred to the caller.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note This function is unsupported on Windows versions prior to Windows
   * 8.1, and will fail with boost::asio::error::operation_not_supported on
   * these platforms.
   */
  native_handle_type release(boost::system::error_code& ec);

  /// Get the native file representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * file. This is intended to allow access to native file functionality
   * that is not otherwise provided.
   */
  native_handle_type native_handle();

  /// Cancel all asynchronous operations associated with the file.
  /**
   * This function causes all outstanding asynchronous read and write
   * operations to finish immediately, and the handlers for cancelled
   * operations will be passed the boost::asio::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note Calls to cancel() will always fail with
   * boost::asio::error::operation_not_supported when run on Windows XP, Windows
   * Server 2003, and earlier versions of Windows, unless
   * BOOST_ASIO_ENABLE_CANCELIO is defined. However, the CancelIo function has
   * two issues that should be considered before enabling its use:
   *
   * @li It will only cancel asynchronous operations that were initiated in the
   * current thread.
   *
   * @li It can appear to complete without error, but the request to cancel the
   * unfinished operations may be silently ignored by the operating system.
   * Whether it works or not seems to depend on the drivers that are installed.
   *
   * For portable cancellation, consider using the close() function to
   * simultaneously cancel the outstanding operations and close the file.
   *
   * When running on Windows Vista, Windows Server 2008, and later, the
   * CancelIoEx function is always used. This function does not have the
   * problems described above.
   */
  void cancel() override;

  /// Cancel all asynchronous operations associated with the file.
  /**
   * This function causes all outstanding asynchronous read and write
   * operations to finish immediately, and the handlers for cancelled
   * operations will be passed the boost::asio::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls to cancel() will always fail with
   * boost::asio::error::operation_not_supported when run on Windows XP, Windows
   * Server 2003, and earlier versions of Windows, unless
   * BOOST_ASIO_ENABLE_CANCELIO is defined. However, the CancelIo function has
   * two issues that should be considered before enabling its use:
   *
   * @li It will only cancel asynchronous operations that were initiated in the
   * current thread.
   *
   * @li It can appear to complete without error, but the request to cancel the
   * unfinished operations may be silently ignored by the operating system.
   * Whether it works or not seems to depend on the drivers that are installed.
   *
   * For portable cancellation, consider using the close() function to
   * simultaneously cancel the outstanding operations and close the file.
   *
   * When running on Windows Vista, Windows Server 2008, and later, the
   * CancelIoEx function is always used. This function does not have the
   * problems described above.
   */
  void cancel(boost::system::error_code& ec) override;

  /// Get the size of the file.
  /**
   * This function determines the size of the file, in bytes.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  uint64_t size() const;

  /// Get the size of the file.
  /**
   * This function determines the size of the file, in bytes.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  uint64_t size(boost::system::error_code& ec) const;

  /// Alter the size of the file.
  /**
   * This function resizes the file to the specified size, in bytes. If the
   * current file size exceeds @c n then any extra data is discarded. If the
   * current size is less than @c n then the file is extended and filled with
   * zeroes.
   *
   * @param n The new size for the file.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void resize(uint64_t n);

  /// Alter the size of the file.
  /**
   * This function resizes the file to the specified size, in bytes. If the
   * current file size exceeds @c n then any extra data is discarded. If the
   * current size is less than @c n then the file is extended and filled with
   * zeroes.
   *
   * @param n The new size for the file.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID resize(uint64_t n, boost::system::error_code& ec);

  /// Synchronise the file to disk.
  /**
   * This function synchronises the file data and metadata to disk. Note that
   * the semantics of this synchronisation vary between operation systems.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void sync_all();

  /// Synchronise the file to disk.
  /**
   * This function synchronises the file data and metadata to disk. Note that
   * the semantics of this synchronisation vary between operation systems.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID sync_all(boost::system::error_code& ec);

  /// Synchronise the file data to disk.
  /**
   * This function synchronises the file data to disk. Note that the semantics
   * of this synchronisation vary between operation systems.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void sync_data();

  /// Synchronise the file data to disk.
  /**
   * This function synchronises the file data to disk. Note that the semantics
   * of this synchronisation vary between operation systems.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID sync_data(boost::system::error_code& ec);

  file(const file&) = delete;
  file& operator=(const file&) = delete;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return *impl_;}

 protected:

  file();

  /// Protected destructor to prevent deletion through this type.
  /**
   * This function destroys the file, cancelling any outstanding asynchronous
   * operations associated with the file as if by calling @c cancel.
   */
  ~file();

  implementation_type * impl_;
};


/// Provides ssl-oriented file functionality.
/**
 * The stream_file class template provides asynchronous and blocking
 * ssl-oriented file functionality.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * AsyncReadStream, AsyncWriteStream, Stream, SyncReadStream, SyncWriteStream.
 */
struct stream_file final : concepts::implements<concepts::cancellable, concepts::closable, concepts::stream>, file
{
  /// The underlying asio implementation type.
  typedef asio::basic_stream_file<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;

  typedef typename implementation_type::native_handle_type  native_handle_type;

  /// Construct a stream_file without opening it.
  /**
   * This constructor initialises a file without opening it. The file needs to
   * be opened before data can be read from or or written to it.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   */
  explicit stream_file(const executor_type& ex);

  /// Construct a stream_file without opening it.
  /**
   * This constructor initialises a file without opening it. The file needs to
   * be opened before data can be read from or or written to it.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   */
  explicit stream_file(asio::io_context& context);
  /// Construct and open a stream_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  stream_file(const executor_type& ex,
              const char* path, asio::file_base::flags open_flags);

  /// Construct and open a stream_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  stream_file(asio::io_context& context,
                    const char* path, asio::file_base::flags open_flags);
  /// Construct and open a stream_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  stream_file(const executor_type& ex,
              const std::string& path, asio::file_base::flags open_flags);

  /// Construct and open a stream_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  stream_file(asio::io_context& context,
              const std::string& path, asio::file_base::flags open_flags);
  /// Construct a stream_file on an existing native file.
  /**
   * This constructor initialises a ssl file object to hold an existing
   * native file.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   *
   * @param native_file The new underlying file implementation.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  stream_file(const executor_type& ex,
              const native_handle_type& native_file);
  /// Construct a stream_file on an existing native file.
  /**
   * This constructor initialises a ssl file object to hold an existing
   * native file.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   *
   * @param native_file The new underlying file implementation.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  stream_file(asio::io_context& context,
              const native_handle_type& native_file);
  /// Move-construct a stream_file from another.
  /**
   * This constructor moves a ssl file from one object to another.
   *
   * @param other The other stream_file object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c stream_file(const executor_type&)
   * constructor.
   */
  stream_file(stream_file&& other) noexcept;

  /// Move-assign a stream_file from another.
  /**
   * This assignment operator moves a ssl file from one object to another.
   *
   * @param other The other stream_file object from which the move
   * will occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c stream_file(const executor_type&)
   * constructor.
   */
  stream_file& operator=(stream_file&& other);
  /// Destroys the file.
  /**
   * This function destroys the file, cancelling any outstanding asynchronous
   * operations associated with the file as if by calling @c cancel.
   */
  ~stream_file();

  /// Seek to a position in the file.
  /**
   * This function updates the current position in the file.
   *
   * @param offset The requested position in the file, relative to @c whence.
   *
   * @param whence One of @c seek_set, @c seek_cur or @c seek_end.
   *
   * @returns The new position relative to the beginning of the file.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  uint64_t seek(int64_t offset, asio::file_base::seek_basis whence);

  /// Seek to a position in the file.
  /**
   * This function updates the current position in the file.
   *
   * @param offset The requested position in the file, relative to @c whence.
   *
   * @param whence One of @c seek_set, @c seek_cur or @c seek_end.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The new position relative to the beginning of the file.
   */
  uint64_t seek(int64_t offset, asio::file_base::seek_basis whence,
                boost::system::error_code& ec);

  /// Start an asynchronous write.
  /**
   * This function is used to asynchronously write data to the ssl file.
   * It is an initiating function for an @ref asynchronous_operation, and always
   * returns immediately.
   *
   * @param buffers One or more data buffers to be written to the file.
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
   * file.async_write_some(boost::asio::buffer(data, size), handler);
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
  void async_write_some(asio::const_buffer, concepts::write_handler h) override;
  void async_write_some(prepared_buffers,   concepts::write_handler h) override;
  void async_write_some(any_const_buffer_range buffer, concepts::write_handler h) override;

  /// Start an asynchronous read.
  /**
   * This function is used to asynchronously read data from the ssl file.
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
   * file.async_read_some(boost::asio::buffer(data, size), handler);
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
  void async_read_some(asio::mutable_buffer buffer,                     concepts::write_handler h) override;
  void async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::write_handler h) override;
  void async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::write_handler h) override;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  implementation_type impl_;
};


/// Provides random-access file functionality.
/**
 * The random_access_file class template provides asynchronous and
 * blocking random-access file functionality.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * Synchronous @c read_some_at and @c write_some_at operations are thread safe
 * with respect to each other, if the underlying operating system calls are
 * also thread safe. This means that it is permitted to perform concurrent
 * calls to these synchronous operations on a single file object. Other
 * synchronous operations, such as @c open or @c close, are not thread safe.
 */
struct random_access_file final : 
    file, concepts::implements<concepts::cancellable, concepts::closable, concepts::random_access_device>
{
  /// The underlying implementation.
  typedef asio::basic_random_access_file<asio::io_context::executor_type> implementation_type;

  /// The type of the executor associated with the object.
  typedef asio::io_context::executor_type executor_type;


  /// The native representation of a file.
  typedef typename implementation_type::native_handle_type native_handle_type;

  /// Construct a random_access_file without opening it.
  /**
   * This constructor initialises a file without opening it. The file needs to
   * be opened before data can be read from or or written to it.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   */
  explicit random_access_file(const executor_type& ex);
  /// Construct a random_access_file without opening it.
  /**
   * This constructor initialises a file without opening it. The file needs to
   * be opened before data can be read from or or written to it.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   */
  explicit random_access_file(asio::io_context& context);
  /// Construct and open a random_access_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  random_access_file(const executor_type& ex,
                     const char* path, asio::file_base::flags open_flags);


  /// Construct and open a random_access_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  random_access_file(asio::io_context& context,
                     const char* path, asio::file_base::flags open_flags);

  /// Construct and open a random_access_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  random_access_file(const executor_type& ex,
                           const std::string& path, asio::file_base::flags open_flags);

  /// Construct and open a random_access_file.
  /**
   * This constructor initialises and opens a file.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   *
   * @param path The path name identifying the file to be opened.
   *
   * @param open_flags A set of flags that determine how the file should be
   * opened.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  random_access_file(asio::io_context& context,
                           const std::string& path, asio::file_base::flags open_flags);
  /// Construct a random_access_file on an existing native file.
  /**
   * This constructor initialises a random-access file object to hold an
   * existing native file.
   *
   * @param ex The I/O executor that the file will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the file.
   *
   * @param native_file The new underlying file implementation.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  random_access_file(const executor_type& ex, const native_handle_type& native_file);

  /// Construct a random_access_file on an existing native file.
  /**
   * This constructor initialises a random-access file object to hold an
   * existing native file.
   *
   * @param context An execution context which provides the I/O executor that
   * the file will use, by default, to dispatch handlers for any asynchronous
   * operations performed on the file.
   *
   * @param native_file The new underlying file implementation.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  random_access_file(asio::io_context& context, const native_handle_type& native_file);

  random_access_file(random_access_file&& other) noexcept;
  random_access_file& operator=(random_access_file&& other);
  /// Destroys the file.
  /**
   * This function destroys the file, cancelling any outstanding asynchronous
   * operations associated with the file as if by calling @c cancel.
   */
  ~random_access_file();
  /// Start an asynchronous write at the specified offset.
  /**
   * This function is used to asynchronously write data to the random-access
   * handle. It is an initiating function for an @ref asynchronous_operation,
   * and always returns immediately.
   *
   * @param offset The offset at which the data will be written.
   *
   * @param buffers One or more data buffers to be written to the handle.
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
   * @note The write operation may not write all of the data to the file.
   * Consider using the @ref async_write_at function if you need to ensure that
   * all data is written before the asynchronous operation completes.
   *
   * @par Example
   * To write a single data buffer use the @ref buffer function as follows:
   * @code
   * handle.async_write_some_at(42, boost::asio::buffer(data, size), handler);
   * @endcode
   * See the @ref buffer documentation for information on writing multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
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
  void async_write_some_at(std::uint64_t offset, asio::const_buffer buffer, concepts::write_handler h) override;

  /// Start an asynchronous read at the specified offset.
  /**
   * This function is used to asynchronously read data from the random-access
   * handle. It is an initiating function for an @ref asynchronous_operation,
   * and always returns immediately.
   *
   * @param offset The offset at which the data will be read.
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
   * Consider using the @ref async_read_at function if you need to ensure that
   * the requested amount of data is read before the asynchronous operation
   * completes.
   *
   * @par Example
   * To read into a single data buffer use the @ref buffer function as follows:
   * @code
   * handle.async_read_some_at(42, boost::asio::buffer(data, size), handler);
   * @endcode
   * See the @ref buffer documentation for information on reading into multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
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
  void async_read_some_at(std::uint64_t offset, asio::mutable_buffer buffer, concepts::read_handler h) override;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  implementation_type impl_;
};

}

#endif

#endif //BOOST_ASYNC_IO_FILE_HPP
