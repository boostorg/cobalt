//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/resolver.hpp>

namespace boost::async::io
{


resolver::resolver() : resolver_(this_thread::get_executor()) {}
void resolver::cancel() { resolver_.cancel(); }

void resolver::resolve_op_::initiate(completion_handler<system::error_code, resolve_result::value_type> h)
{
  using results_type = typename asio::ip::basic_resolver_results<protocol_type>;
  resolver_.async_resolve(
      async::io::ip, host_, service_,
      asio::deferred([](system::error_code ec, results_type res)
      {
        container::pmr::vector<endpoint> r{this_thread::get_allocator()};
        r.assign(res.begin(), res.end());
        return asio::deferred.values(ec, std::move(r));
      }))(std::move(h));
}

}