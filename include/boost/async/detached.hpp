//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETACHED_HPP
#define BOOST_ASYNC_DETACHED_HPP

#include <boost/async/detail/detached.hpp>

namespace boost::async
{

struct detached
{
  using promise_type = detail::detached_promise;
};

inline detached detail::detached_promise::get_return_object() { return {}; }


}


#endif //BOOST_ASYNC_DETACHED_HPP
