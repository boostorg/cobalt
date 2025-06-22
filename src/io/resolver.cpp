//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.asio)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/resolver.hpp>

#include <boost/asio/deferred.hpp>

namespace boost::cobalt::io
{


resolver::resolver(const cobalt::executor & executor) : resolver_(executor) {}
void resolver::cancel() { resolver_.cancel(); }

struct transformer
{
  template<typename RR>
  auto operator()(system::error_code ec, RR rr)
  {
#if !defined(BOOST_COBALT_NO_PMR)
    endpoint_sequence r{this_thread::get_allocator()};
#else
    endpoint_sequence r{};
#endif
    r.assign(rr.begin(), rr.end());

    return asio::deferred.values(ec, std::move(r));
  }
};


void resolver::resolve_op_::initiate(completion_handler<system::error_code, endpoint_sequence> h)
{
  resolver_.async_resolve(
      cobalt::io::ip, host_, service_, flags_,
      asio::deferred(transformer{}))(std::move(h));
}

void lookup::initiate(completion_handler<system::error_code, endpoint_sequence> h)
{
  resolver_.async_resolve(
      cobalt::io::ip, host_, service_, flags_,
      asio::deferred(transformer{}))(std::move(h));
}


}