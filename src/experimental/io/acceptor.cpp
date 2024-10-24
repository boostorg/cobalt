//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/acceptor.hpp>

namespace boost::cobalt::experimental::io
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

void acceptor::initiate_accept_ (void * this_, socket * sock,
                                 boost::cobalt::completion_handler<boost::system::error_code> handler)
{
  return static_cast<acceptor*>(this_)->acceptor_.async_accept(sock->socket_, std::move(handler));
}

void acceptor::initiate_wait_(void * this_, wait_type wt, completion_handler<boost::system::error_code> handler)
{
  return static_cast<acceptor*>(this_)->acceptor_.async_wait(wt, std::move(handler));

}





}