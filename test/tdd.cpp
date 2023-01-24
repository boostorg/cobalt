//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#include <boost/asio.hpp>

#include <boost/async/async.hpp>
#include <boost/async/main.hpp>
#include <boost/async/op.hpp>

using namespace boost;

template<typename Timer>
struct wait_op : async::enable_op<wait_op<Timer>>
{
  Timer & tim;

  wait_op(Timer & tim) : tim(tim) {}

  bool ready(system::error_code & ) { return tim.expiry() < Timer::clock_type::now();}
  void initiate(async::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};

template<typename Timer>
struct wait_ec_op
{
  Timer & tim;
  system::error_code &ec;

  bool ready() { return tim.expires() < Timer::clock_type::now();}
  void initiate(async::completion_handler<> complete)
  {
    tim.async_wait(asio::redirect_error(std::move(complete), ec));
  }
};

template<typename Socket>
struct read_op
{
  Socket & sock;

  asio::mutable_buffer buffer;

  void initiate(async::completion_handler<system::error_code> complete)
  {
    sock.async_read_some(std::move(complete));
  }
};

template<typename Channel, typename ... Args>
struct channel_receive_op
{
  Channel & chan;

  void ready(async::handler<system::error_code> complete)
  {
    return chan.try_receive(std::move(complete));
  }
  void initiate(async::completion_handler<system::error_code> complete)
  {
    chan.async_receive(std::move(complete));
  }
};


async::main co_main(int argc, char * argv[])
{
  // co_await (foo{}, 42);
  asio::steady_timer  tim{co_await asio::this_coro::executor};
  co_await tim.async_wait(asio::deferred);
  using wait_type = decltype(wait_op{tim});
  static_assert(async::detail::awaitable<wait_type>);

  co_await wait_op{tim};

  co_return 0;
}