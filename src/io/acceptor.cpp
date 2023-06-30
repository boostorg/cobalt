//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/acceptor.hpp>

namespace boost::async::io
{
acceptor::acceptor() : acceptor_{this_thread::get_executor()} {}
acceptor::acceptor(endpoint ep) : acceptor_{this_thread::get_executor(), ep} {}


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

auto acceptor::accept() -> accept_op_ { return accept_op_{acceptor_};}
auto acceptor::accept_seq_packet() -> accept_seq_op_ { return accept_seq_op_{acceptor_};}

}