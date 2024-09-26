//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_SIGNAL_SET_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_SIGNAL_SET_HPP

#include <boost/cobalt/op.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>

#include <boost/asio/basic_signal_set.hpp>
#include <boost/system/result.hpp>

#include <initializer_list>

namespace boost::cobalt::experimental::io
{

template<typename Awaitable>
concept signal_wait_op =
    awaitable<Awaitable> &&
    std::same_as<
        detail::co_await_result_t<as_tuple_t<decltype(detail::get_awaitable_type(std::declval<Awaitable>()))>>,
        std::tuple<system::error_code, int>>;

struct signal_set
{
  using wait_result = system::result<int>;

  signal_set();
  signal_set(std::initializer_list<int> sigs);

  [[nodiscard]] system::result<void> cancel();
  [[nodiscard]] system::result<void> clear();
  [[nodiscard]] system::result<void> add(int signal_number);
  [[nodiscard]] system::result<void> remove(int signal_number);


 private:
  struct wait_op_ final : cobalt::op<system::error_code, int>
  {
    BOOST_COBALT_DECL
    void initiate(completion_handler<system::error_code, int> h) final;
    wait_op_(boost::asio::basic_signal_set<cobalt::executor> & signal_set) : signal_set_(signal_set) {}
   private:
    boost::asio::basic_signal_set<cobalt::executor> & signal_set_;
  };
 public:
  [[nodiscard]] signal_wait_op auto wait() { return wait_op_{signal_set_}; }
 private:
  boost::asio::basic_signal_set<cobalt::executor> signal_set_;
};


}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_SIGNAL_SET_HPP
