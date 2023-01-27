//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_CANCELLATION_HPP
#define BOOST_ASYNC_CANCELLATION_HPP

#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_state.hpp>

namespace boost::async
{

using asio::cancellation_type;
using asio::cancellation_signal;

// Requests cancellation where a successful cancellation results
// in no apparent side effects and where the op can re-awaited.
constexpr asio::cancellation_type interrupt_await{8u};



}

#endif //BOOST_ASYNC_CANCELLATION_HPP
