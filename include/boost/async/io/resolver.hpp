//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_RESOLVER_HPP
#define BOOST_ASYNC_IO_RESOLVER_HPP

#include <boost/async/io/endpoint.hpp>
#include <boost/async/io/result.hpp>

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
  resolver();
  resolver(resolver && ) = delete;

  void cancel();

 private:

  struct resolve_op_ final : result_op<resolve_result>
  {
    void initiate(async::completion_handler<system::error_code, resolve_result> complete) override;
    resolve_op_(asio::ip::basic_resolver<protocol_type, asio::io_context::executor_type> & resolver,
                core::string_view host, core::string_view service)
                : resolver_(resolver), host_(host), service_(service) {}
   private:
    asio::ip::basic_resolver<protocol_type, asio::io_context::executor_type> & resolver_;
    core::string_view host_;
    core::string_view service_;
  };

 public:

  [[nodiscard]] resolve_op_ resolve(core::string_view host, core::string_view service)
  {
    return resolve_op_{resolver_, host, service};
  }

 private:
  friend struct lookup;
  asio::ip::basic_resolver<protocol_type, asio::io_context::executor_type> resolver_;
};

// NOTE: Doesn't need to be a promise, can be optimized.
struct lookup
{
  lookup(core::string_view host, core::string_view service)
        : host_(host), service_(service) {}
  auto operator co_await() {return resolver_.resolve(host_, service_);}
  auto value() { return op.emplace(resolver_.resolver_, host_, service_).value(); }

 private:
  core::string_view host_;
  core::string_view service_;
  resolver resolver_;
  std::optional<resolver::resolve_op_> op;

};

}

#endif //BOOST_ASYNC_IO_RESOLVER_HPP
