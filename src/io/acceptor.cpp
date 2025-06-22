//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/acceptor.hpp>
#include <boost/cobalt/composition.hpp>

namespace boost::cobalt::io
{

acceptor::acceptor(const cobalt::executor & exec) : acceptor_{exec} {}
acceptor::acceptor(endpoint ep, const cobalt::executor & exec) : acceptor_{exec, ep} {}

system::result<void> acceptor::bind(endpoint ep)
{
  system::error_code ec;
  acceptor_.bind(ep, ec);
  return ec ? ec : system::result<void>{};
}
system::result<void> acceptor::listen(int backlog)
{
  system::error_code ec;
  acceptor_.listen(backlog, ec);
  return ec ? ec : system::result<void>{};
}

endpoint acceptor::local_endpoint()
{
  return acceptor_.local_endpoint();
}

void acceptor::accept_op::initiate (boost::cobalt::completion_handler<system::error_code> handler)
{
  acceptor_.async_accept(sock_.socket_, std::move(handler));
}

void acceptor::accept_stream_op::initiate(completion_handler<system::error_code, stream_socket>)
{
  auto [ec] = co_await acceptor_.async_accept(sock_.socket_);
  co_return {ec, std::move(sock_)};
}

void acceptor::accept_seq_packet_op::initiate(completion_handler<system::error_code, seq_packet_socket>)
{
  auto [ec] = co_await acceptor_.async_accept(sock_.socket_);
  co_return {ec, std::move(sock_)};
}

void acceptor::wait_op::initiate(completion_handler<system::error_code> handler)
{
  acceptor_.async_wait(wt_, std::move(handler));

}





}