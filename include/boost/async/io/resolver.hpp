//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_RESOLVER_HPP
#define BOOST_ASYNC_IO_RESOLVER_HPP

#include <boost/async/io/endpoint.hpp>

#include <boost/async/detail/op.hpp>
#include <boost/async/promise.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/system/result.hpp>
#include <boost/url/url_view.hpp>

namespace boost::async::io
{


struct resolver
{
  using resolve_result = system::result<container::pmr::vector<endpoint>>;
  BOOST_ASYNC_DECL resolver();
  BOOST_ASYNC_DECL resolver(resolver && ) = delete;

  BOOST_ASYNC_DECL void cancel();

 private:

  struct resolve_op_ : detail::deferred_op_resource_base
  {
    using result_type = asio::ip::basic_resolver<protocol_type, executor>::results_type;

    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        initiate_(completion_handler<system::error_code, result_type>{h, result_, get_resource(h)});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    [[nodiscard]] resolve_result await_resume();
    resolve_op_(asio::ip::basic_resolver<protocol_type, executor> & resolver,
                core::string_view host, core::string_view service)
                : resolver_(resolver), host_(host), service_(service) {}
   private:
    asio::ip::basic_resolver<protocol_type, executor> & resolver_;
    core::string_view host_;
    core::string_view service_;
    std::exception_ptr error;
    std::optional<std::tuple<system::error_code, result_type>> result_;

    void initiate_(completion_handler<system::error_code, result_type>);

  };

 public:

  [[nodiscard]] resolve_op_ resolve(core::string_view host, core::string_view service)
  {
    return resolve_op_{resolver_, host, service};
  }


 private:
  asio::ip::basic_resolver<protocol_type, executor> resolver_;
};

struct lookup
{
  lookup(core::string_view host, core::string_view service)
        : host_(host), service_(service) {}
  auto operator co_await() {return resolver_.resolve(host_, service_);}
 private:
  core::string_view host_;
  core::string_view service_;
  resolver resolver_;

};

}

#endif //BOOST_ASYNC_IO_RESOLVER_HPP
