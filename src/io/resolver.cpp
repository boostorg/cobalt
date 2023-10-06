//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/resolver.hpp>

#include <boost/asio/deferred.hpp>
#include <boost/system/result.hpp>

namespace boost::async::io
{


resolver::resolver(const async::executor & exec) : resolver_(exec) {}
void resolver::cancel() { resolver_.cancel(); }


void do_initiate(asio::ip::basic_resolver<protocol_type, executor> & resolver_,
                 core::string_view host, core::string_view service,
                 completion_handler<system::error_code, resolver::resolve_result::value_type> h)
{
  using results_type = typename asio::ip::basic_resolver_results<protocol_type>;
  resolver_.async_resolve(
      async::io::ip, host, service,
      asio::deferred([](system::error_code ec, results_type res)
                     {
                       pmr::vector<endpoint> r{this_thread::get_allocator()};
                       r.assign(res.begin(), res.end());
                       return asio::deferred.values(ec, std::move(r));
                     }))(std::move(h));
}


void resolver::resolve_op_::initiate(
    completion_handler<system::error_code, resolve_result::value_type> h)
{
  do_initiate(resolver_, host_, service_, std::move(h));
}

BOOST_ASYNC_DECL
void lookup::initiate(
    completion_handler<system::error_code, resolver::resolve_result::value_type> handler)
{
  auto & res = resolver_.emplace(handler.get_executor());
  do_initiate(res, host_, service_, std::move(handler));
}

}