//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IMPL_CHANNEL_IPP
#define BOOST_ASYNC_IMPL_CHANNEL_IPP

#include <boost/async/channel.hpp>

namespace boost::async
{

bool channel<void>::  try_receive(std::optional<std::tuple<system::error_code>> & result)
{
  return impl_.try_receive([&](system::error_code ec) { result.emplace(ec);});
}

void channel<void>::async_receive(boost::async::completion_handler<system::error_code> h)
{
  impl_.async_receive(std::move(h));
}

bool channel<void>::  try_send()
{
  return impl_.try_send(system::error_code());
}

void channel<void>::async_send(boost::async::completion_handler<system::error_code> h)
{
  impl_.async_send(system::error_code(), std::move(h));
}


}

#endif //BOOST_ASYNC_IMPL_CHANNEL_IPP
