//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_THIS_THREAD_IPP
#define BOOST_ASYNC_THIS_THREAD_IPP

#include <memory_resource>
#include "asio/io_context.hpp"
#include <optional>

namespace boost::async::this_thread::detail
{

thread_local std::pmr::memory_resource * default_coro_memory_resource = std::pmr::get_default_resource();
thread_local std::optional<asio::io_context::executor_type> executor;

}


#endif //BOOST_ASYNC_THIS_THREAD_IPP
