//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_ACCEPTOR_HPP
#define BOOST_ASYNC_IO_ACCEPTOR_HPP

#include <boost/async/io/stream_socket.hpp>
#include <boost/async/io/seq_packet_socket.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>


namespace boost::async::io
{

struct acceptor
{
  BOOST_ASYNC_DECL acceptor();
  BOOST_ASYNC_DECL acceptor(endpoint ep);
  BOOST_ASYNC_DECL system::result<void> bind(endpoint ep);
  BOOST_ASYNC_DECL system::result<void> listen(int backlog = asio::socket_base::max_listen_connections); // int backlog = asio::max_backlog()
  BOOST_ASYNC_DECL endpoint local_endpoint();

 private:

  struct accept_op_ : detail::deferred_op_resource_base
  {
    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        acceptor_.template async_accept(socket_.socket_,  completion_handler<system::error_code>{h, result_, get_resource(h)});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    [[nodiscard]] system::result<stream_socket> await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      auto [ec] = result_.value_or(std::make_tuple(system::error_code{}));
      if (ec)
        return ec;
      else
        return std::move(socket_);
    }
    accept_op_(    asio::basic_socket_acceptor<protocol_type, executor_type> & acceptor)
      : acceptor_(acceptor) {}
   private:
    asio::basic_socket_acceptor<protocol_type, executor_type> & acceptor_;
    stream_socket socket_;
    std::exception_ptr error;
    std::optional<std::tuple<system::error_code>> result_;
  };

  struct accept_seq_op_ : detail::deferred_op_resource_base
  {
    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        acceptor_.template async_accept(socket_.socket_,  completion_handler<system::error_code>{h, result_, get_resource(h)});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    [[nodiscard]] system::result<seq_packet_socket> await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      auto [ec] = result_.value_or(std::make_tuple(system::error_code{}));
      if (ec)
        return ec;
      else
        return std::move(socket_);
    }
    accept_seq_op_(asio::basic_socket_acceptor<protocol_type, executor_type> & acceptor)
        : acceptor_(acceptor) {}
   private:
    asio::basic_socket_acceptor<protocol_type, executor_type> & acceptor_;
    seq_packet_socket socket_;
    std::exception_ptr error;
    std::optional<std::tuple<system::error_code>> result_;
  };
 public:
  [[nodiscard]] BOOST_ASYNC_DECL accept_op_     accept();
  [[nodiscard]] BOOST_ASYNC_DECL accept_seq_op_ accept_seq_packet();
 private:
  asio::basic_socket_acceptor<protocol_type, executor_type> acceptor_;
};

}

#endif //BOOST_ASYNC_IO_ACCEPTOR_HPP
