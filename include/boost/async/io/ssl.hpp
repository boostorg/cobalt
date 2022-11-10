//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SSL_HPP
#define BOOST_ASYNC_IO_SSL_HPP

#include <boost/asio/ssl/stream.hpp>

namespace boost::async::io
{


/// Provides ssl-oriented functionality using SSL.
/**
 * The ssl class template provides asynchronous and blocking ssl-oriented
 * functionality using SSL.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe. The application must also ensure that all
 * asynchronous operations are performed within the same implicit or explicit
 * strand.
 *
 * @par Example
 * To use the SSL ssl template with an ip::tcp::socket, you would write:
 * @code
 * boost::asio::io_context my_context;
 * boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
 * boost::asio::ssl::ssl<asio:ip::tcp::socket> sock(my_context, ctx);
 * @endcode
 *
 * @par Concepts:
 * AsyncReadStream, AsyncWriteStream, Stream, SyncReadStream, SyncWriteStream.
 */
template <typename Stream>
struct ssl :
    concepts::implements<concepts::cancellable, concepts::closable, concepts::stream>,
    asio::ssl::stream_base
{
  /// The underlying asio implementation type.
  typedef asio::ssl::stream<typename Stream::implementation_type &> implementation_type;

  /// The native handle type of the SSL ssl.
  typedef SSL* native_handle_type;

  /// The type of the next layer.
  typedef typename std::remove_reference<Stream>::type next_layer_type;

  /// The type of the lowest layer.
  typedef typename next_layer_type::lowest_layer_type lowest_layer_type;

  /// The type of the executor associated with the object.
  typedef typename lowest_layer_type::executor_type executor_type;

  /// Construct a ssl.
  /**
   * This constructor creates a ssl and initialises the underlying ssl
   * object.
   *
   * @param arg The argument to be passed to initialise the underlying ssl.
   *
   * @param ctx The SSL context to be used for the ssl.
   */
  template <typename Arg>
  ssl(Arg&& arg, asio::ssl::context& ctx)
    : next_layer_(BOOST_ASIO_MOVE_CAST(Arg)(arg)), impl_(next_layer_, ctx)
  {
  }

  /// Construct a ssl from an existing native implementation.
  /**
   * This constructor creates a ssl and initialises the underlying ssl
   * object. On success, ownership of the native implementation is transferred
   * to the ssl, and it will be cleaned up when the ssl is destroyed.
   *
   * @param arg The argument to be passed to initialise the underlying ssl.
   *
   * @param handle An existing native SSL implementation.
   */
  template <typename Arg>
  ssl(Arg&& arg, native_handle_type handle)
    : next_layer_(BOOST_ASIO_MOVE_CAST(Arg)(arg)),
      impl_(next_layer_, handle)
  {
  }

  /// Destructor.
  /**
   * @note A @c ssl object must not be destroyed while there are pending
   * asynchronous operations associated with it.
   */
  ~ssl();

  /// Get the executor associated with the object.
  /**
   * This function may be used to obtain the executor object that the ssl
   * uses to dispatch handlers for asynchronous operations.
   *
   * @return A copy of the executor that ssl will use to dispatch handlers.
   */
  executor_type get_executor() noexcept override;

  /// Get the underlying implementation in the native type.
  /**
   * This function may be used to obtain the underlying implementation of the
   * context. This is intended to allow access to context functionality that is
   * not otherwise provided.
   *
   * @par Example
   * The native_handle() function returns a pointer of type @c SSL* that is
   * suitable for passing to functions such as @c SSL_get_verify_result and
   * @c SSL_get_peer_certificate:
   * @code
   * boost::asio::ssl::ssl<asio:ip::tcp::socket> sock(my_context, ctx);
   *
   * // ... establish connection and perform handshake ...
   *
   * if (X509* cert = SSL_get_peer_certificate(sock.native_handle()))
   * {
   *   if (SSL_get_verify_result(sock.native_handle()) == X509_V_OK)
   *   {
   *     // ...
   *   }
   * }
   * @endcode
   */
  native_handle_type native_handle();

  /// Get a reference to the next layer.
  /**
   * This function returns a reference to the next layer in a stack of ssl
   * layers.
   *
   * @return A reference to the next layer in the stack of ssl layers.
   * Ownership is not transferred to the caller.
   */
  const next_layer_type& next_layer() const;

  /// Get a reference to the next layer.
  /**
   * This function returns a reference to the next layer in a stack of ssl
   * layers.
   *
   * @return A reference to the next layer in the stack of ssl layers.
   * Ownership is not transferred to the caller.
   */
  next_layer_type& next_layer();

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * ssl layers.
   *
   * @return A reference to the lowest layer in the stack of ssl layers.
   * Ownership is not transferred to the caller.
   */
  lowest_layer_type& lowest_layer()
  {
    return next_layer_.lowest_layer();
  }

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * ssl layers.
   *
   * @return A reference to the lowest layer in the stack of ssl layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type& lowest_layer() const
  {
    return next_layer_.lowest_layer();
  }

  /// Set the peer verification mode.
  /**
   * This function may be used to configure the peer verification mode used by
   * the ssl. The new mode will override the mode inherited from the context.
   *
   * @param v A bitmask of peer verification modes. See @ref verify_mode for
   * available values.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note Calls @c SSL_set_verify.
   */
  void set_verify_mode(asio::ssl::verify_mode v);

  /// Set the peer verification mode.
  /**
   * This function may be used to configure the peer verification mode used by
   * the ssl. The new mode will override the mode inherited from the context.
   *
   * @param v A bitmask of peer verification modes. See @ref verify_mode for
   * available values.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls @c SSL_set_verify.
   */
  BOOST_ASIO_SYNC_OP_VOID set_verify_mode(asio::ssl::verify_mode v, boost::system::error_code& ec);

  /// Set the peer verification depth.
  /**
   * This function may be used to configure the maximum verification depth
   * allowed by the ssl.
   *
   * @param depth Maximum depth for the certificate chain verification that
   * shall be allowed.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note Calls @c SSL_set_verify_depth.
   */
  void set_verify_depth(int depth);

  /// Set the peer verification depth.
  /**
   * This function may be used to configure the maximum verification depth
   * allowed by the ssl.
   *
   * @param depth Maximum depth for the certificate chain verification that
   * shall be allowed.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls @c SSL_set_verify_depth.
   */
  BOOST_ASIO_SYNC_OP_VOID set_verify_depth(int depth, boost::system::error_code& ec);

  /// Set the callback used to verify peer certificates.
  /**
   * This function is used to specify a callback function that will be called
   * by the implementation when it needs to verify a peer certificate.
   *
   * @param callback The function object to be used for verifying a certificate.
   * The function signature of the handler must be:
   * @code bool verify_callback(
   *   bool preverified, // True if the certificate passed pre-verification.
   *   verify_context& ctx // The peer certificate and other context.
   * ); @endcode
   * The return value of the callback is true if the certificate has passed
   * verification, false otherwise.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @note Calls @c SSL_set_verify.
   */
  template <typename VerifyCallback>
  void set_verify_callback(VerifyCallback callback)
  {
    impl_.set_verify_callback(std::move(callback));
  }

  /// Set the callback used to verify peer certificates.
  /**
   * This function is used to specify a callback function that will be called
   * by the implementation when it needs to verify a peer certificate.
   *
   * @param callback The function object to be used for verifying a certificate.
   * The function signature of the handler must be:
   * @code bool verify_callback(
   *   bool preverified, // True if the certificate passed pre-verification.
   *   verify_context& ctx // The peer certificate and other context.
   * ); @endcode
   * The return value of the callback is true if the certificate has passed
   * verification, false otherwise.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @note Calls @c SSL_set_verify.
   */
  template <typename VerifyCallback>
  BOOST_ASIO_SYNC_OP_VOID set_verify_callback(VerifyCallback callback,
                                              boost::system::error_code& ec)
  {
    impl_.set_verify_callback(std::move(callback), ec);
  }

 private:
  struct handshake_op_
  {
    ssl * sock;
    handshake_type type;

    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        sock->async_connect(type, {h, result});
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

  struct handshake_op_ec_
  {
    ssl * sock;
    handshake_type type;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        sock->async_connect(type, {h, result});
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

  struct handshake_buffer_op_
  {
    ssl * sock;
    handshake_type type;
    const_buffer buffer;

    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        sock->async_connect(type, buffer, {h, result});
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

  struct handshake_buffer_op_ec_
  {
    ssl * sock;
    handshake_type type;
    const_buffer buffer;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        sock->async_connect(type, buffer, {h, result});
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

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the ssl. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  handshake_op_ handshake(handshake_type type)
  {
    return {this, type};
  }



  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the ssl. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  handshake_op_ec_ handshake(handshake_type type, boost::system::error_code& ec)
  {
    return {this, type, ec};
  }

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the ssl. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param buffers The buffered data to be reused for the handshake.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  handshake_buffer_op_ handshake(handshake_type type, asio::const_buffer buffer)
  {
    return {this, type, buffer};
  }

  /// Perform SSL handshaking.
  /**
   * This function is used to perform SSL handshaking on the ssl. The
   * function call will block until handshaking is complete or an error occurs.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param buffers The buffered data to be reused for the handshake.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  handshake_buffer_op_ec_ handshake(handshake_type type,
                                    asio::const_buffer buffer, boost::system::error_code& ec)
  {
    return {this, type, buffer, ec};
  }
  /// Start an asynchronous SSL handshake.
  /**
   * This function is used to asynchronously perform an SSL handshake on the
   * ssl. It is an initiating function for an @ref asynchronous_operation,
   * and always returns immediately.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the handshake completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const boost::system::error_code& error // Result of operation.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using boost::asio::post().
   *
   * @par Completion Signature
   * @code void(boost::system::error_code) @endcode
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following
   * boost::asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * if they are also supported by the @c Stream type's @c async_read_some and
   * @c async_write_some operations.
   */
  void async_handshake(handshake_type type,
                       boost::async::detail::completion_handler<system::error_code> h);
  /// Start an asynchronous SSL handshake.
  /**
   * This function is used to asynchronously perform an SSL handshake on the
   * ssl. It is an initiating function for an @ref asynchronous_operation,
   * and always returns immediately.
   *
   * @param type The type of handshaking to be performed, i.e. as a client or as
   * a server.
   *
   * @param buffers The buffered data to be reused for the handshake. Although
   * the buffers object may be copied as necessary, ownership of the underlying
   * buffers is retained by the caller, which must guarantee that they remain
   * valid until the completion handler is called.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the handshake completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const boost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred // Amount of buffers used in handshake.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using boost::asio::post().
   *
   * @par Completion Signature
   * @code void(boost::system::error_code, std::size_t) @endcode
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following
   * boost::asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * if they are also supported by the @c Stream type's @c async_read_some and
   * @c async_write_some operations.
   */
  void async_handshake(handshake_type type, asio::const_buffer buffer,
                       boost::async::detail::completion_handler<system::error_code, std::size_t> h);


 private:
  struct shutdown_op_
  {
    ssl * sock;

    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        sock->async_shutdown({h, result});
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

  struct shutdown_op_ec_
  {
    ssl * sock;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code>> result;
    std::exception_ptr error;
    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        sock->async_shutdown({h, result});
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

  /// Shut down SSL on the ssl.
  /**
   * This function is used to shut down SSL on the ssl. The function call
   * will block until SSL has been shut down or an error occurs.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  shutdown_op_ shutdown() { return {this}; }

  /// Shut down SSL on the ssl.
  /**
   * This function is used to shut down SSL on the ssl. The function call
   * will block until SSL has been shut down or an error occurs.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  shutdown_op_ shutdown(system::error_code &ec) { return {this, ec}; }

  /// Asynchronously shut down SSL on the ssl.
  /**
   * This function is used to asynchronously shut down SSL on the ssl. It is
   * an initiating function for an @ref asynchronous_operation, and always
   * returns immediately.
   *
   * @param token The @ref completion_token that will be used to produce a
   * completion handler, which will be called when the shutdown completes.
   * Potential completion tokens include @ref use_future, @ref use_awaitable,
   * @ref yield_context, or a function object with the correct completion
   * signature. The function signature of the completion handler must be:
   * @code void handler(
   *   const boost::system::error_code& error // Result of operation.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the completion handler will not be invoked from within this function.
   * On immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using boost::asio::post().
   *
   * @par Completion Signature
   * @code void(boost::system::error_code) @endcode
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following
   * boost::asio::cancellation_type values:
   *
   * @li @c cancellation_type::terminal
   *
   * @li @c cancellation_type::partial
   *
   * if they are also supported by the @c Stream type's @c async_read_some and
   * @c async_write_some operations.
   */
  void async_shutdown(boost::async::detail::completion_handler<system::error_code> h);

  void async_write_some(const_buffer     buffer, concepts::write_handler h) override;
  void async_write_some(prepared_buffers buffer, concepts::write_handler h) override;
  void async_read_some(asio::mutable_buffer buffer,                     concepts::read_handler h) override;
  void async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::read_handler h) override;
  void async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::read_handler h) override;

  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  Stream next_layer_;
  implementation_type impl_{next_layer_};
};

}

#endif //BOOST_ASYNC_IO_SSL_HPP
