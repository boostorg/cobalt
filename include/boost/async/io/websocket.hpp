//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_WEBSOCKET_HPP
#define BOOST_ASYNC_IO_WEBSOCKET_HPP

#include <boost/beast/websocket/option.hpp>
#include <boost/beast/websocket/stream.hpp>

namespace boost::async::io
{

//--------------------------------------------------------------------

/** Provides message-oriented functionality using WebSocket.

    The @ref websocket class template provides asynchronous and blocking
    message-oriented functionality necessary for clients and servers
    to utilize the WebSocket protocol.

    For asynchronous operations, the application must ensure
    that they are are all performed within the same implicit
    or explicit strand.

    @par Thread Safety
    @e Distinct @e objects: Safe.@n
    @e Shared @e objects: Unsafe.
    The application must also ensure that all asynchronous
    operations are performed within the same implicit or explicit strand.

    @par Example
    To declare the @ref websocket object with a @ref tcp_websocket in a
    multi-threaded asynchronous program using a strand, you may write:
    @code
    websocket::websocket<tcp_websocket> ws{net::make_strand(ioc)};
    @endcode
    Alternatively, for a single-threaded or synchronous application
    you may write:
    @code
    websocket::websocket<tcp_websocket> ws(ioc);
    @endcode

    @tparam NextLayer The type representing the next layer, to which
    data will be read and written during operations. For synchronous
    operations, the type must support the <em>SyncStream</em> concept.
    For asynchronous operations, the type must support the
    <em>AsyncStream</em> concept.

    @tparam deflateSupported A `bool` indicating whether or not the
    websocket will be capable of negotiating the permessage-deflate websocket
    extension. Note that even if this is set to `true`, the permessage
    deflate options (set by the caller at runtime) must still have the
    feature enabled for a successful negotiation to occur.

    @note A websocket object must not be moved or destroyed while there
    are pending asynchronous operations associated with it.

    @par Concepts
        @li <em>AsyncStream</em>
        @li <em>DynamicBuffer</em>
        @li <em>SyncStream</em>

    @see
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">Websocket Close (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">WebSocket Ping (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">WebSocket Pong (RFC6455)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
        @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
*/
template<class NextLayer>
struct websocket final :
    concepts::implements<concepts::stream, concepts::message_stream>,
    beast::websocket::stream_base
{
  /// Indicates if the permessage-deflate extension is supported
  using is_deflate_supported = std::true_type;

  /// The underlying asio implementation type.
  typedef beast::websocket::stream<typename NextLayer::implementation_type &> implementation_type;

  /// The type of the next layer.
  using next_layer_type = typename std::remove_reference<NextLayer>::type;

  /// The type of the executor associated with the object.
  using executor_type = beast::executor_type<next_layer_type>;

  /// The resposne type
  typedef beast::websocket::response_type response_type;

  /** Destructor

      Destroys the websocket and all associated resources.

      @note A websocket object must not be destroyed while there
      are pending asynchronous operations associated with it.
  */
  ~websocket();

  /** Constructor

      If `NextLayer` is move constructible, this function
      will move-construct a new websocket from the existing websocket.

      After the move, the only valid operation on the moved-from
      object is destruction.
  */
  websocket(websocket&&) = default;

  /// Move assignment (deleted)
  websocket& operator=(websocket&&) = default;

  /** Constructor

      This constructor creates a websocket websocket and initializes
      the next layer object.

      @throws Any exceptions thrown by the NextLayer constructor.

      @param args The arguments to be passed to initialize the
      next layer object. The arguments are forwarded to the next
      layer's constructor.
  */
  template<class... Args>
  explicit
  websocket(Args&&... args) : next_layer_(std::forward<Args>(args)...) {}

  //--------------------------------------------------------------------------

  /** Get the executor associated with the object.

      This function may be used to obtain the executor object that the
      websocket uses to dispatch handlers for asynchronous operations.

      @return A copy of the executor that websocket will use to dispatch handlers.
  */
  executor_type
  get_executor() noexcept override;

  /** Get a reference to the next layer

      This function returns a reference to the next layer
      in a stack of websocket layers.

      @return A reference to the next layer in the stack of
      websocket layers.
  */
  next_layer_type&
  next_layer() noexcept;

  /** Get a reference to the next layer

      This function returns a reference to the next layer in a
      stack of websocket layers.

      @return A reference to the next layer in the stack of
      websocket layers.
  */
  next_layer_type const&
  next_layer() const noexcept;

  //--------------------------------------------------------------------------
  //
  // Observers
  //
  //--------------------------------------------------------------------------

  /** Returns `true` if the websocket is open.

      The websocket is open after a successful handshake, and when
      no error has occurred.
  */
  bool
  is_open() const noexcept;

  /** Returns `true` if the latest message data indicates binary.

      This function informs the caller of whether the last
      received message frame represents a message with the
      binary opcode.

      If there is no last message frame, the return value is
      undefined.
  */
  bool
  got_binary() const noexcept;

  /** Returns `true` if the latest message data indicates text.

      This function informs the caller of whether the last
      received message frame represents a message with the
      text opcode.

      If there is no last message frame, the return value is
      undefined.
  */
  bool
  got_text() const
  {
    return ! got_binary();
  }

  /// Returns `true` if the last completed read finished the current message.
  bool
  is_message_done() const noexcept;

  /** Returns the close reason received from the remote peer.

      This is only valid after a read completes with error::closed.
  */
  beast::websocket::close_reason const&
  reason() const noexcept;

  /** Returns a suggested maximum buffer size for the next call to read.

      This function returns a reasonable upper limit on the number
      of bytes for the size of the buffer passed in the next call
      to read. The number is determined by the state of the current
      frame and whether or not the permessage-deflate extension is
      enabled.

      @param initial_size A non-zero size representing the caller's
      desired buffer size for when there is no information which may
      be used to calculate a more specific value. For example, when
      reading the first frame header of a message.
  */
  std::size_t read_size_hint( std::size_t initial_size) const;
  std::size_t read_size_hint() const;

  /** Returns a suggested maximum buffer size for the next call to read.

      This function returns a reasonable upper limit on the number
      of bytes for the size of the buffer passed in the next call
      to read. The number is determined by the state of the current
      frame and whether or not the permessage-deflate extension is
      enabled.

      @param buffer The buffer which will be used for reading. The
      implementation will query the buffer to obtain the optimum
      size of a subsequent call to `buffer.prepare` based on the
      state of the current frame, if any.
  */
  std::size_t read_size_hint(flat_static_buffer_base &)    const;
  std::size_t read_size_hint(static_buffer_base &)         const;
  std::size_t read_size_hint(flat_buffer &)                const;
  std::size_t read_size_hint(multi_buffer &)               const;
  std::size_t read_size_hint(std::string  &)               const;
  std::size_t read_size_hint(std::vector<unsigned char> &) const;
  std::size_t read_size_hint(streambuf &)                  const;

  //--------------------------------------------------------------------------
  //
  // Settings
  //
  //--------------------------------------------------------------------------

  void set_option(decorator opt);

  /** Set the timeout option

      @throws system_error on failure to reset the
      timer.
  */
  void
  set_option(timeout const& opt);

  /// Get the timeout option
  void
  get_option(timeout& opt);

  /** Set the permessage-deflate extension options

      @throws invalid_argument if `deflateSupported == false`, and either
      `client_enable` or `server_enable` is `true`.
  */
  void
  set_option(beast::websocket::permessage_deflate const& o);

  /// Get the permessage-deflate extension options
  void
  get_option(beast::websocket::permessage_deflate& o);

  /** Set the automatic fragmentation option.

      Determines if outgoing message payloads are broken up into
      multiple pieces.

      When the automatic fragmentation size is turned on, outgoing
      message payloads are broken up into multiple frames no larger
      than the write buffer size.

      The default setting is to fragment messages.

      @param value A `bool` indicating if auto fragmentation should be on.

      @par Example
      Setting the automatic fragmentation option:
      @code
          ws.auto_fragment(true);
      @endcode
  */
  void
  auto_fragment(bool value);

  /// Returns `true` if the automatic fragmentation option is set.
  bool
  auto_fragment() const;

  /** Set the binary message write option.

      This controls whether or not outgoing message opcodes
      are set to binary or text. The setting is only applied
      at the start when a caller begins a new message. Changing
      the opcode after a message is started will only take effect
      after the current message being sent is complete.

      The default setting is to send text messages.

      @param value `true` if outgoing messages should indicate
      binary, or `false` if they should indicate text.

      @par Example
      Setting the message type to binary.
      @code
          ws.binary(true);
      @endcode
      */
  void
  binary(bool value);

  /// Returns `true` if the binary message write option is set.
  bool
  binary() const;

  /** Set a callback to be invoked on each incoming control frame.

      Sets the callback to be invoked whenever a ping, pong,
      or close control frame is received during a call to one
      of the following functions:

      @li @ref beast::websocket::websocket::read
      @li @ref beast::websocket::websocket::read_some
      @li @ref beast::websocket::websocket::async_read
      @li @ref beast::websocket::websocket::async_read_some

      Unlike completion handlers, the callback will be invoked
      for each control frame during a call to any synchronous
      or asynchronous read function. The operation is passive,
      with no associated error code, and triggered by reads.

      For close frames, the close reason code may be obtained by
      calling the function @ref reason.

      @param cb The function object to call, which must be
      invocable with this equivalent signature:
      @code
      void
      callback(
          frame_type kind,       // The type of frame
          string_view payload    // The payload in the frame
      );
      @endcode
      The implementation type-erases the callback which may require
      a dynamic allocation. To prevent the possibility of a dynamic
      allocation, use `std::ref` to wrap the callback.
      If the read operation which receives the control frame is
      an asynchronous operation, the callback will be invoked using
      the same method as that used to invoke the final handler.

      @note Incoming ping and close frames are automatically
      handled. Pings are responded to with pongs, and a close frame
      is responded to with a close frame leading to the closure of
      the websocket. It is not necessary to manually send pings, pongs,
      or close frames from inside the control callback.
      Attempting to manually send a close frame from inside the
      control callback after receiving a close frame will result
      in undefined behavior.
  */
  void
  control_callback(std::function<void(beast::websocket::frame_type, beast::string_view)> cb);

  /** Reset the control frame callback.

      This function removes any previously set control frame callback.
  */
  void
  control_callback();

  /** Set the maximum incoming message size option.

      Sets the largest permissible incoming message size. Message
      frame fields indicating a size that would bring the total
      message size over this limit will cause a protocol failure.

      The default setting is 16 megabytes. A value of zero indicates
      a limit of the maximum value of a `std::uint64_t`.

      @par Example
      Setting the maximum read message size.
      @code
          ws.read_message_max(65536);
      @endcode

      @param amount The limit on the size of incoming messages.
  */
  void
  read_message_max(std::size_t amount);

  /// Returns the maximum incoming message size setting.
  std::size_t
  read_message_max() const;

  /** Set whether the PRNG is cryptographically secure

      This controls whether or not the source of pseudo-random
      numbers used to produce the masks required by the WebSocket
      protocol are of cryptographic quality. When the setting is
      `true`, a strong algorithm is used which cannot be guessed
      by observing outputs. When the setting is `false`, a much
      faster algorithm is used.
      Masking is only performed by websockets operating in the client
      mode. For websockets operating in the server mode, this setting
      has no effect.
      By default, newly constructed websockets use a secure PRNG.

      If the WebSocket websocket is used with an encrypted SSL or TLS
      next layer, if it is known to the application that intermediate
      proxies are not vulnerable to cache poisoning, or if the
      application is designed such that an attacker cannot send
      arbitrary inputs to the websocket interface, then the faster
      algorithm may be used.

      For more information please consult the WebSocket protocol RFC.

      @param value `true` if the PRNG algorithm should be
      cryptographically secure.
  */
  void
  secure_prng(bool value);

  /** Set the write buffer size option.

      Sets the size of the write buffer used by the implementation to
      send frames. The write buffer is needed when masking payload data
      in the client role, compressing frames, or auto-fragmenting message
      data.

      Lowering the size of the buffer can decrease the memory requirements
      for each connection, while increasing the size of the buffer can reduce
      the number of calls made to the next layer to write data.

      The default setting is 4096. The minimum value is 8.

      The write buffer size can only be changed when the websocket is not
      open. Undefined behavior results if the option is modified after a
      successful WebSocket handshake.

      @par Example
      Setting the write buffer size.
      @code
          ws.write_buffer_bytes(8192);
      @endcode

      @param amount The size of the write buffer in bytes.
  */
  void
  write_buffer_bytes(std::size_t amount);

  /// Returns the size of the write buffer.
  std::size_t
  write_buffer_bytes() const;

  /** Set the text message write option.

      This controls whether or not outgoing message opcodes
      are set to binary or text. The setting is only applied
      at the start when a caller begins a new message. Changing
      the opcode after a message is started will only take effect
      after the current message being sent is complete.

      The default setting is to send text messages.

      @param value `true` if outgoing messages should indicate
      text, or `false` if they should indicate binary.

      @par Example
      Setting the message type to text.
      @code
          ws.text(true);
      @endcode
  */
  void
  text(bool value);

  /// Returns `true` if the text message write option is set.
  bool
  text() const;

  /** Set the compress message write option.

      This controls whether or not outgoing messages should be
      compressed. The setting is only applied when

      @li The template parameter `deflateSupported` is true
      @li Compression is enable. This is controlled with `websocket::set_option`
      @li Client and server have negotiated permessage-deflate settings
      @li The message is larger than `permessage_deflate::msg_size_threshold`

      This function permits adjusting per-message compression.
      Changing the opcode after a message is started will only take effect
      after the current message being sent is complete.

      The default setting is to compress messages whenever the conditions
      above are true.

      @param value `true` if outgoing messages should be compressed

      @par Example
      Disabling compression for a single message.
      @code
          ws.compress(false);
          ws.write(net::buffer(s), ec);
          ws.compress(true);
      @endcode
  */
  void
  compress(bool value);

  /// Returns `true` if the compress message write option is set.
  bool
  compress() const;


  //--------------------------------------------------------------------------
  //
  // Handshaking (Client)
  //
  //--------------------------------------------------------------------------

 private:
  struct handshake_op_
  {
    websocket * impl;
    string_view host;
    string_view target;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_handshake(host, target, {h, result});
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

  struct handshake_op_ec_
  {
    websocket * impl;
    string_view host;
    string_view target;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_handshake(host, target, {h, result});
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

  struct handshake_res_op_
  {
    websocket * impl;
    beast::websocket::response_type& res;
    string_view host;
    string_view target;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_handshake(res, host, target, {h, result});
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

  struct handshake_res_op_ec_
  {
    websocket * impl;
    beast::websocket::response_type& res;
    string_view host;
    string_view target;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_handshake(res, host, target, {h, result});
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

  /** Perform the WebSocket handshake in the client role.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is sent and the response is received.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      The handshake is successful if the received HTTP response
      indicates the upgrade was accepted by the server, represented by a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols.

      @param host The name of the remote host. This is required by
      the HTTP protocol to set the "Host" header field.

      @param target The request-target, in origin-form. The server may use the
      target to distinguish different services on the same listening port.

      @throws system_error Thrown on failure.

      @par Example
      @code
      ws.handshake("localhost", "/");
      @endcode

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
  */
  handshake_op_
  handshake(
      string_view host,
      string_view target) { return {this, host, target}; }

  /** Perform the WebSocket handshake in the client role.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is sent and the response is received.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      The handshake is successful if the received HTTP response
      indicates the upgrade was accepted by the server, represented by a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols.

      @param res The HTTP Upgrade response returned by the remote
      endpoint. The caller may use the response to access any
      additional information sent by the server. Note that the response object
      referenced by this parameter will be updated as long as the websocket has
      received a valid HTTP response. If not (for example because of a communications
      error), the response contents will be undefined except for the result() which
      will bet set to 500, Internal Server Error.

      @param host The name of the remote host. This is required by
      the HTTP protocol to set the "Host" header field.

      @param target The request-target, in origin-form. The server may use the
      target to distinguish different services on the same listening port.

      @throws system_error Thrown on failure.

      @par Example
      @code
      response_type res;
      ws.handshake(res, "localhost", "/");
      std::cout << res;
      @endcode

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
  */
  handshake_res_op_
  handshake(
      beast::websocket::response_type& res,
      string_view host,
      string_view target)
  {
    return handshake_res_op_{this, res, host, target};
  }

  /** Perform the WebSocket handshake in the client role.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is sent and the response is received.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      The handshake is successful if the received HTTP response
      indicates the upgrade was accepted by the server, represented by a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols.

      @param host The name of the remote host. This is required by
      the HTTP protocol to set the "Host" header field.

      @param target The request-target, in origin-form. The server may use the
      target to distinguish different services on the same listening port.

      @param ec Set to indicate what error occurred, if any.

      @par Example
      @code
      error_code ec;
      ws.handshake("localhost", "/", ec);
      @endcode

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
  */
  handshake_op_ec_
  handshake(
      string_view host,
      string_view target,
      system::error_code& ec)
  {
    return {this, host, target, ec};
  }

  /** Perform the WebSocket handshake in the client role.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is sent and the response is received.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      The handshake is successful if the received HTTP response
      indicates the upgrade was accepted by the server, represented by a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols.

      @param res The HTTP Upgrade response returned by the remote
      endpoint. The caller may use the response to access any
      additional information sent by the server.

      @param host The name of the remote host. This is required by
      the HTTP protocol to set the "Host" header field.

      @param target The request-target, in origin-form. The server may use the
      target to distinguish different services on the same listening port.

      @param ec Set to indicate what error occurred, if any.

      @par Example
      @code
      error_code ec;
      response_type res;
      ws.handshake(res, "localhost", "/", ec);
      if(! ec)
          std::cout << res;
      @endcode

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
  */
  handshake_res_op_ec_
  handshake(
      beast::websocket::response_type& res,
      string_view host,
      string_view target,
      system::error_code& ec)
  {
    return {this, res, host, target, ec};
  }

  /** Perform the WebSocket handshake asynchronously in the client role.

      This initiating function is used to asynchronously begin performing the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The request is sent and the response is received.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. No other operation may be performed
      on the websocket until this operation completes.

      The handshake is successful if the received HTTP response
      indicates the upgrade was accepted by the server, represented by a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols.

      @param host The name of the remote host. This is required by
      the HTTP protocol to set the "Host" header field.
      The implementation will not access the string data after the
      initiating function returns.

      @param target The request-target, in origin-form. The server may use the
      target to distinguish different services on the same listening port.
      The implementation will not access the string data after the
      initiating function returns.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec    // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Example
      @code
      ws.async_handshake("localhost", "/",
          [](error_code ec)
          {
              if(ec)
                  std::cerr << "Error: " << ec.message() << "\n";
          });
      @endcode

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
  */
  void async_handshake(
      beast::string_view host,
      beast::string_view target,
      boost::async::completion_handler<system::error_code> h);

  /** Perform the WebSocket handshake asynchronously in the client role.

      This initiating function is used to asynchronously begin performing the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The request is sent and the response is received.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. No other operation may be performed
      on the websocket until this operation completes.

      The handshake is successful if the received HTTP response
      indicates the upgrade was accepted by the server, represented by a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols.

      @param res The HTTP Upgrade response returned by the remote
      endpoint. The caller may use the response to access any
      additional information sent by the server. This object will
      be assigned before the completion handler is invoked.

      @param host The name of the remote host. This is required by
      the HTTP protocol to set the "Host" header field.
      The implementation will not access the string data after the
      initiating function returns.

      @param target The request-target, in origin-form. The server may use the
      target to distinguish different services on the same listening port.
      The implementation will not access the string data after the
      initiating function returns.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec    // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Example
      @code
      response_type res;
      ws.async_handshake(res, "localhost", "/",
          [&res](error_code ec)
          {
              if(ec)
                  std::cerr << "Error: " << ec.message() << "\n";
              else
                  std::cout << res;

          });
      @endcode

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.1">Websocket Opening Handshake Client Requirements (RFC6455)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.4">Host field (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-3.1.1">request-target (RFC7230)</a>
      @li <a href="https://tools.ietf.org/html/rfc7230#section-5.3.1">origin-form (RFC7230)</a>
  */
  void async_handshake(
      response_type& res,
      string_view host,
      string_view target,boost::async::completion_handler<system::error_code> h);

  //--------------------------------------------------------------------------
  //
  // Handshaking (Server)
  //
  //--------------------------------------------------------------------------

 private:
  struct accept_op_
  {
    websocket * impl;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_accept({h, result});
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

  struct accept_op_ec_
  {
    websocket * impl;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_accept({h, result});
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

  struct accept_buffer_op_
  {
    websocket * impl;
    asio::const_buffer buffers;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_accept(buffers, {h, result});
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

  struct accept_buffer_op_ec_
  {
    websocket * impl;
    asio::const_buffer buffers;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_accept(buffers, {h, result});
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


  template<typename Body>
  struct accept_body_op_
  {
    websocket * impl;
    const Body & body;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_accept(body, {h, result});
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

  template<typename Body>
  struct accept_body_op_ec_
  {
    websocket * impl;
    const Body & body;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_accept(body, {h, result});
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
  
  /** Perform the WebSocket handshake in the server role.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      If the request size exceeds the capacity of the websocket's
      internal buffer, the error @ref error::buffer_overflow will be
      indicated. To handle larger requests, an application should
      read the HTTP request directly using @ref http::read and then
      pass the request to the appropriate overload of @ref accept or
      @ref async_accept

      @throws system_error Thrown on failure.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  accept_op_ accept() { return {this}; }

  /** Read and respond to a WebSocket HTTP Upgrade request.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      If the request size exceeds the capacity of the websocket's
      internal buffer, the error @ref error::buffer_overflow will be
      indicated. To handle larger requests, an application should
      read the HTTP request directly using @ref http::read and then
      pass the request to the appropriate overload of @ref accept or
      @ref async_accept

      @param ec Set to indicate what error occurred, if any.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  accept_op_ec_ accept(system::error_code& ec) {return {this, ec};}

  /** Read and respond to a WebSocket HTTP Upgrade request.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      If the request size exceeds the capacity of the websocket's
      internal buffer, the error @ref error::buffer_overflow will be
      indicated. To handle larger requests, an application should
      read the HTTP request directly using @ref http::read and then
      pass the request to the appropriate overload of @ref accept or
      @ref async_accept

      @param buffers Caller provided data that has already been
      received on the websocket. The implementation will copy the
      caller provided data before the function returns.

      @throws system_error Thrown on failure.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  accept_buffer_op_ accept(asio::const_buffer const& buffers) { return {this, buffers}; }

  /** Read and respond to a WebSocket HTTP Upgrade request.

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      If the request size exceeds the capacity of the websocket's
      internal buffer, the error @ref error::buffer_overflow will be
      indicated. To handle larger requests, an application should
      read the HTTP request directly using @ref http::read and then
      pass the request to the appropriate overload of @ref accept or
      @ref async_accept

      @param buffers Caller provided data that has already been
      received on the websocket. The implementation will copy the
      caller provided data before the function returns.

      @param ec Set to indicate what error occurred, if any.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  accept_buffer_op_ec_ accept(asio::const_buffer const& buffers, system::error_code& ec)
  {
    return {this, buffers, ec};
  }

  /** Respond to a WebSocket HTTP Upgrade request

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      @param req An object containing the HTTP Upgrade request.
      Ownership is not transferred, the implementation will not
      access this object from other threads.

      @throws system_error Thrown on failure.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  template<typename Body>
  accept_body_op_<Body> accept(http::request<Body> const& req)
  {
    return {this, req};
  }

  /** Respond to a WebSocket HTTP Upgrade request

      This function is used to perform the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      The call blocks until one of the following conditions is true:

      @li The response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `read_some` and `write_some`
      functions.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      @param req An object containing the HTTP Upgrade request.
      Ownership is not transferred, the implementation will not
      access this object from other threads.

      @param ec Set to indicate what error occurred, if any.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  template<class Body>
  accept_body_op_ec_<Body>
  accept(http::request<Body> const& req, system::error_code& ec)
  {
    return {this, req, ec};
  }

  /** Perform the WebSocket handshake asynchronously in the server role.

      This initiating function is used to asynchronously begin performing the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. No other operation may be performed
      on the websocket until this operation completes.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      If the request size exceeds the capacity of the websocket's
      internal buffer, the error @ref error::buffer_overflow will be
      indicated. To handle larger requests, an application should
      read the HTTP request directly using @ref http::async_read and then
      pass the request to the appropriate overload of @ref accept or
      @ref async_accept

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec    // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  void async_accept(boost::async::completion_handler<system::error_code> h);

  /** Perform the WebSocket handshake asynchronously in the server role.

      This initiating function is used to asynchronously begin performing the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. No other operation may be performed
      on the websocket until this operation completes.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      If the request size exceeds the capacity of the websocket's
      internal buffer, the error @ref error::buffer_overflow will be
      indicated. To handle larger requests, an application should
      read the HTTP request directly using @ref http::async_read and then
      pass the request to the appropriate overload of @ref accept or
      @ref async_accept

      @param buffers Caller provided data that has already been
      received on the websocket. This may be used for implementations
      allowing multiple protocols on the same websocket. The
      buffered data will first be applied to the handshake, and
      then to received WebSocket frames. The implementation will
      copy the caller provided data before the function returns.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec    // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  void async_accept(const const_buffer & buffers, boost::async::completion_handler<system::error_code> h);

  /** Perform the WebSocket handshake asynchronously in the server role.

      This initiating function is used to asynchronously begin performing the
      <a href="https://en.wikipedia.org/wiki/WebSocket#Protocol_handshake">WebSocket handshake</a>,
      required before messages can be sent and received. During the handshake,
      the client sends the Websocket Upgrade HTTP request, and the server
      replies with an HTTP response indicating the result of the handshake.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The request is received and the response is sent.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. No other operation may be performed
      on the websocket until this operation completes.

      If a valid upgrade request is received, an HTTP response with a
      <a href="https://tools.ietf.org/html/rfc7230#section-3.1.2">status-code</a>
      of @ref beast::http::status::switching_protocols is sent to
      the peer, otherwise a non-successful error is associated with
      the operation.

      @param req An object containing the HTTP Upgrade request.
      Ownership is not transferred, the implementation will not access
      this object from other threads.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec    // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-4.2">Websocket Opening Handshake Server Requirements (RFC6455)</a>
  */
  void async_accept(const http::request<http::buffer_body> &msg,  boost::async::completion_handler<system::error_code> h);
  void async_accept(const http::request<http::empty_body>  &msg,  boost::async::completion_handler<system::error_code> h);
  void async_accept(const http::request<http::file_body>   &msg,  boost::async::completion_handler<system::error_code> h);
  void async_accept(const http::request<http::string_body> &msg,  boost::async::completion_handler<system::error_code> h);
  void async_accept(const http::request<http::vector_body> &msg,  boost::async::completion_handler<system::error_code> h);

  //--------------------------------------------------------------------------
  //
  // Close Frames
  //
  //--------------------------------------------------------------------------

 private:

  struct close_op_
  {
    websocket * impl;
    beast::websocket::close_reason const& cr;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_close(cr, {h, result});
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

  struct close_op_ec_
  {
    websocket * impl;
    beast::websocket::close_reason const& cr;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_close(cr, {h, result});
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

  /** Send a websocket close control frame.

      This function is used to send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close frame</a>,
      which begins the websocket closing handshake. The session ends when
      both ends of the connection have sent and received a close frame.

      The call blocks until one of the following conditions is true:

      @li The close frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `write_some` function.

      After beginning the closing handshake, the program should not write
      further message data, pings, or pongs. Instead, the program should
      continue reading message data until an error occurs. A read returning
      @ref error::closed indicates a successful connection closure.

      @param cr The reason for the close.
      If the close reason specifies a close code other than
      @ref beast::websocket::close_code::none, the close frame is
      sent with the close code and optional reason string. Otherwise,
      the close frame is sent with no payload.

      @throws system_error Thrown on failure.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
  */
  close_op_
  close(beast::websocket::close_reason const& cr) {return {this, cr};}

  /** Send a websocket close control frame.

      This function is used to send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close frame</a>,
      which begins the websocket closing handshake. The session ends when
      both ends of the connection have sent and received a close frame.

      The call blocks until one of the following conditions is true:

      @li The close frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `write_some` function.

      After beginning the closing handshake, the program should not write
      further message data, pings, or pongs. Instead, the program should
      continue reading message data until an error occurs. A read returning
      @ref error::closed indicates a successful connection closure.

      @param cr The reason for the close.
      If the close reason specifies a close code other than
      @ref beast::websocket::close_code::none, the close frame is
      sent with the close code and optional reason string. Otherwise,
      the close frame is sent with no payload.

      @param ec Set to indicate what error occurred, if any.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
  */
  close_op_ec_
  close(beast::websocket::close_reason const& cr, system::error_code& ec) {return {this, cr, ec}; }

  /** Send a websocket close control frame asynchronously.

      This function is used to asynchronously send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.1">close frame</a>,
      which begins the websocket closing handshake. The session ends when
      both ends of the connection have sent and received a close frame.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The close frame finishes sending.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_write_some`
      function. No other operations except for message reading operations
      should be initiated on the websocket after a close operation is started.

      After beginning the closing handshake, the program should not write
      further message data, pings, or pongs. Instead, the program should
      continue reading message data until an error occurs. A read returning
      @ref error::closed indicates a successful connection closure.

      @param cr The reason for the close.
      If the close reason specifies a close code other than
      @ref beast::websocket::close_code::none, the close frame is
      sent with the close code and optional reason string. Otherwise,
      the close frame is sent with no payload.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec     // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

       @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      @note `terminal` cancellation will may close the underlying socket.

      @see
      @li <a href="https://tools.ietf.org/html/rfc6455#section-7.1.2">Websocket Closing Handshake (RFC6455)</a>
  */
  void async_close(
      beast::websocket::close_reason const& cr,
      boost::async::completion_handler<system::error_code> h);

  //--------------------------------------------------------------------------
  //
  // Ping/Pong Frames
  //
  //--------------------------------------------------------------------------

 private:

  struct ping_op_
  {
    websocket * impl;
    beast::websocket::ping_data const& ping_data;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_ping(ping_data, {h, result});
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

  struct ping_op_ec_
  {
    websocket * impl;
    beast::websocket::ping_data const& ping_data;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_ping(ping_data, {h, result});
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
  /** Send a websocket ping control frame.

      This function is used to send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping frame</a>,
      which usually elicits an automatic pong control frame response from
      the peer.

      The call blocks until one of the following conditions is true:

      @li The ping frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `write_some` function.

      @param payload The payload of the ping message, which may be empty.

      @throws system_error Thrown on failure.
  */
  ping_op_
  ping(beast::websocket::ping_data const& payload) { return {this, payload};}

  /** Send a websocket ping control frame.

      This function is used to send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping frame</a>,
      which usually elicits an automatic pong control frame response from
      the peer.

      The call blocks until one of the following conditions is true:

      @li The ping frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `write_some` function.

      @param payload The payload of the ping message, which may be empty.

      @param ec Set to indicate what error occurred, if any.
  */
  ping_op_ec_
  ping(beast::websocket::ping_data const& payload, system::error_code& ec) {return {this, payload, ec};}

  /** Send a websocket ping control frame asynchronously.

      This function is used to asynchronously send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.2">ping frame</a>,
      which usually elicits an automatic pong control frame response from
      the peer.

      @li The ping frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_write_some`
      function. The program must ensure that no other calls to @ref ping,
      @ref pong, @ref async_ping, or @ref async_pong are performed until
      this operation completes.

      If a close frame is sent or received before the ping frame is
      sent, the error received by this completion handler will be
      `net::error::operation_aborted`.

      @param payload The payload of the ping message, which may be empty.
      The implementation will not access the contents of this object after
      the initiating function returns.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec     // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      `terminal` cancellation leaves the websocket in an undefined state,
      so that only closing it is guaranteed to succeed.
  */
  void async_ping(
      beast::websocket::ping_data const& payload,
      boost::async::completion_handler<system::error_code> h);


 private:

  struct pong_op_
  {
    websocket * impl;
    beast::websocket::ping_data const& ping_data;
    std::optional<std::tuple<system::error_code, std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_pong(ping_data, {h, result});
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

  struct pong_op_ec_
  {
    websocket * impl;
    beast::websocket::ping_data const& ping_data;
    system::error_code & ec;
    std::optional<std::tuple<system::error_code,std::size_t>> result;
    std::exception_ptr error;

    constexpr static bool await_ready() {return false;}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        impl->async_pong(ping_data, {h, result});
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

  /** Send a websocket pong control frame.

      This function is used to send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong frame</a>,
      which is usually sent automatically in response to a ping frame
      from the remote peer.

      The call blocks until one of the following conditions is true:

      @li The pong frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `write_some` function.

      WebSocket allows pong frames to be sent at any time, without first
      receiving a ping. An unsolicited pong sent in this fashion may
      indicate to the remote peer that the connection is still active.

      @param payload The payload of the pong message, which may be empty.

      @throws system_error Thrown on failure.
  */
  pong_op_
  pong(beast::websocket::ping_data const& payload) {return {this, payload};}


  /** Send a websocket pong control frame.

      This function is used to send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong frame</a>,
      which is usually sent automatically in response to a ping frame
      from the remote peer.

      The call blocks until one of the following conditions is true:

      @li The pong frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed operation</em>, is implemented
      in terms of calls to the next layer's `write_some` function.

      WebSocket allows pong frames to be sent at any time, without first
      receiving a ping. An unsolicited pong sent in this fashion may
      indicate to the remote peer that the connection is still active.

      @param payload The payload of the pong message, which may be empty.

      @param ec Set to indicate what error occurred, if any.
  */
  pong_op_ec_
  pong(beast::websocket::ping_data const& payload, system::error_code& ec) {return {this, payload, ec};}

  /** Send a websocket pong control frame asynchronously.

      This function is used to asynchronously send a
      <a href="https://tools.ietf.org/html/rfc6455#section-5.5.3">pong frame</a>,
      which is usually sent automatically in response to a ping frame
      from the remote peer.

      @li The pong frame is written.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_write_some`
      function. The program must ensure that no other calls to @ref ping,
      @ref pong, @ref async_ping, or @ref async_pong are performed until
      this operation completes.

      If a close frame is sent or received before the pong frame is
      sent, the error received by this completion handler will be
      `net::error::operation_aborted`.

      WebSocket allows pong frames to be sent at any time, without first
      receiving a ping. An unsolicited pong sent in this fashion may
      indicate to the remote peer that the connection is still active.

      @param payload The payload of the pong message, which may be empty.
      The implementation will not access the contents of this object after
      the initiating function returns.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec     // Result of operation
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      `terminal` cancellation leaves the websocket in an undefined state,
      so that only closing it is guaranteed to succeed.
  */
  void async_pong(
      beast::websocket::ping_data const& payload,
      boost::async::completion_handler<system::error_code> h);


  /** Read a complete message asynchronously.

      This function is used to asynchronously read a complete message.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li A complete message is received.

      @li A close frame is received. In this case the error indicated by
          the function will be @ref error::closed.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. The program must ensure that no other
      calls to @ref read, @ref read_some, @ref async_read, or @ref async_read_some
      are performed until this operation completes.

      Received message data is appended to the buffer.
      The functions @ref got_binary and @ref got_text may be used
      to query the websocket and determine the type of the last received message.

      Until the operation completes, the implementation will read incoming
      control frames and handle them automatically as follows:

      @li The @ref control_callback will be invoked for each control frame.

      @li For each received ping frame, a pong frame will be
          automatically sent.

      @li If a close frame is received, the WebSocket close procedure is
          performed. In this case, when the function returns, the error
          @ref error::closed will be indicated.

      Pong frames and close frames sent by the implementation while the
      read operation is outstanding do not prevent the application from
      also writing message data, sending pings, sending pongs, or sending
      close frames.

      @param buffer A dynamic buffer to append message data to.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec,       // Result of operation
          std::size_t bytes_written   // Number of bytes appended to buffer
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      `terminal` cancellation leaves the websocket in an undefined state,
      so that only closing it is guaranteed to succeed.
  */

  void async_read(flat_static_buffer_base &buffer,    concepts::read_handler rh) override;
  void async_read(static_buffer_base &buffer,         concepts::read_handler rh) override;
  void async_read(flat_buffer &buffer,                concepts::read_handler rh) override;
  void async_read(multi_buffer &buffer,               concepts::read_handler rh) override;
  void async_read(asio::dynamic_string_buffer<char,  std::char_traits<char>, std::allocator<char>> &buffer, concepts::read_handler rh) override;
  void async_read(asio::dynamic_vector_buffer<unsigned char, std::allocator<unsigned char>> &buffer,        concepts::read_handler rh) override;
  void async_read(streambuf &buffer,                  concepts::read_handler rh) override;

  /** Read some message data asynchronously.

      This function is used to asynchronously read some message data.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li Some message data is received.

      @li A close frame is received. In this case the error indicated by
          the function will be @ref error::closed.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's `async_read_some`
      and `async_write_some` functions. The program must ensure that no other
      calls to @ref read, @ref read_some, @ref async_read, or @ref async_read_some
      are performed until this operation completes.

      Received message data is appended to the buffer.
      The functions @ref got_binary and @ref got_text may be used
      to query the websocket and determine the type of the last received message.

      Until the operation completes, the implementation will read incoming
      control frames and handle them automatically as follows:

      @li The @ref control_callback will be invoked for each control frame.

      @li For each received ping frame, a pong frame will be
          automatically sent.

      @li If a close frame is received, the WebSocket close procedure is
          performed. In this case, when the function returns, the error
          @ref error::closed will be indicated.

      Pong frames and close frames sent by the implementation while the
      read operation is outstanding do not prevent the application from
      also writing message data, sending pings, sending pongs, or sending
      close frames.

      @param buffers A buffer sequence to write message data into.
      The previous contents of the buffers will be overwritten, starting
      from the beginning.
      The implementation will make copies of this object as needed, but
      but ownership of the underlying memory is not transferred. The
      caller is responsible for ensuring that the memory locations
      pointed to by the buffer sequence remain valid until the
      completion handler is called.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec,       // Result of operation
          std::size_t bytes_written   // Number of bytes written to the buffers
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.


      @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      `terminal` cancellation leaves the websocket in an undefined state,
      so that only closing it is guaranteed to succeed.
  */
  void async_read_some(asio::mutable_buffer buffer,                     concepts::read_handler h) override;
  void async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::read_handler h) override;
  void async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::read_handler h) override;

  /** Write a complete message asynchronously.

      This function is used to asynchronously write a complete message.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The complete message is written.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's
      `async_write_some` function. The program must ensure that no other
      calls to @ref write, @ref write_some, @ref async_write, or
      @ref async_write_some are performed until this operation completes.

      The current setting of the @ref binary option controls
      whether the message opcode is set to text or binary. If the
      @ref auto_fragment option is set, the message will be split
      into one or more frames as necessary. The actual payload contents
      sent may be transformed as per the WebSocket protocol settings.

      @param buffers A buffer sequence containing the entire message
      payload. The implementation will make copies of this object
      as needed, but ownership of the underlying memory is not
      transferred. The caller is responsible for ensuring that
      the memory locations pointed to by buffers remains valid
      until the completion handler is called.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec,           // Result of operation
          std::size_t bytes_transferred   // Number of bytes sent from the
                                          // buffers. If an error occurred,
                                          // this will be less than the buffer_size.
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      `terminal` cancellation leaves the websocket in an undefined state,
      so that only closing it is guaranteed to succeed.
  */
  void async_write(any_const_buffer_range buffer, concepts::write_handler h) override;
  void async_write(const_buffer           buffer, concepts::write_handler h) override;
  void async_write(prepared_buffers       buffer, concepts::write_handler h) override;

  /** Write some message data asynchronously.

      This function is used to asynchronously write part of a message.

      This call always returns immediately. The asynchronous operation
      will continue until one of the following conditions is true:

      @li The message data is written.

      @li An error occurs.

      The algorithm, known as a <em>composed asynchronous operation</em>,
      is implemented in terms of calls to the next layer's
      `async_write_some` function. The program must ensure that no other
      calls to @ref write, @ref write_some, @ref async_write, or
      @ref async_write_some are performed until this operation completes.

      If this is the beginning of a new message, the message opcode
      will be set to text or binary based on the current setting of
      the @ref binary (or @ref text) option. The actual payload sent
      may be transformed as per the WebSocket protocol settings.

      @param fin `true` if this is the last part of the message.

      @param buffers The buffers containing the message part to send.
      The implementation will make copies of this object
      as needed, but ownership of the underlying memory is not
      transferred. The caller is responsible for ensuring that
      the memory locations pointed to by buffers remains valid
      until the completion handler is called.

      @param handler The completion handler to invoke when the operation
      completes. The implementation takes ownership of the handler by
      performing a decay-copy. The equivalent function signature of
      the handler must be:
      @code
      void handler(
          error_code const& ec,           // Result of operation
          std::size_t bytes_transferred   // Number of bytes sent from the
                                          // buffers. If an error occurred,
                                          // this will be less than the buffer_size.
      );
      @endcode
      Regardless of whether the asynchronous operation completes
      immediately or not, the handler will not be invoked from within
      this function. Invocation of the handler will be performed in a
      manner equivalent to using `net::post`.

      @par Per-Operation Cancellation

      This asynchronous operation supports cancellation for the following
      net::cancellation_type values:

      @li @c net::cancellation_type::terminal
      @li @c net::cancellation_type::total

      `total` cancellation succeeds if the operation is suspended due to ongoing
      control operations such as a ping/pong.
      `terminal` cancellation succeeds when supported by the underlying websocket.

      `terminal` cancellation leaves the websocket in an undefined state,
      so that only closing it is guaranteed to succeed.
  */
  void async_write_some(any_const_buffer_range buffer, concepts::write_handler h) override;
  void async_write_some(const_buffer           buffer, concepts::write_handler h) override;
  void async_write_some(prepared_buffers       buffer, concepts::write_handler h) override;


  /// Get the underlying asio implementation.
  implementation_type & implementation() {return impl_;}

 private:
  NextLayer next_layer_;
  implementation_type impl_{next_layer_.implementation()};
};


}

#endif //BOOST_ASYNC_IO_WEBSOCKET_HPP
