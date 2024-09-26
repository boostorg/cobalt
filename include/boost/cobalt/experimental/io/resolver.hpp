//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_RESOLVER_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_RESOLVER_HPP

#include <boost/cobalt/experimental/io/endpoint.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/promise.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/system/result.hpp>
#include <boost/url/url_view.hpp>

namespace boost::cobalt::experimental::io
{



struct resolver
{
  resolver();
  resolver(resolver && ) = delete;

  void cancel();

 private:

  struct resolve_op_ final : op<system::error_code, pmr::vector<endpoint>>
  {
    BOOST_COBALT_DECL
    void initiate(completion_handler<system::error_code, pmr::vector<endpoint>> h) override;

    resolve_op_(asio::ip::basic_resolver<protocol_type, executor> & resolver,
                core::string_view host, core::string_view service)
                : resolver_(resolver), host_(host), service_(service) {}
   private:
    asio::ip::basic_resolver<protocol_type, executor> & resolver_;
    core::string_view host_;
    core::string_view service_;

  };

 public:

  [[nodiscard]] io_op<system::error_code, pmr::vector<endpoint>> auto resolve(core::string_view host, core::string_view service)
  {
    return resolve_op_{resolver_, host, service};
  }


 private:
  asio::ip::basic_resolver<protocol_type, executor> resolver_;
};

// NOTE: Doesn't need to be a promise, can be optimized.
struct lookup
{
  lookup(core::string_view host, core::string_view service)
        : host_(host), service_(service) {}
  auto operator co_await() && {return std::move(op_.emplace(resolver_.resolve(host_, service_))).operator co_await();}
 private:
  core::string_view host_;
  core::string_view service_;
  resolver resolver_;
  std::optional<decltype(resolver_.resolve(host_, service_))> op_;
};

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_RESOLVER_HPP
