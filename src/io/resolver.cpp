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


[[nodiscard]] resolver::resolve_result resolver::resolve_op_::await_resume()
{
  if (error)
    std::rethrow_exception(std::exchange(error, nullptr));

  auto [ec, rr] = result_.value();
  if (ec)
    return ec;
  else
  {
    container::pmr::vector<endpoint> r{this_thread::get_allocator()};
    r.assign(rr.begin(), rr.end());
    return r;
  }
}

void resolver::resolve_op_::initiate_(completion_handler<system::error_code, result_type> h)
{
  resolver_.async_resolve(async::io::ip, host_, service_, std::move(h));
}

}