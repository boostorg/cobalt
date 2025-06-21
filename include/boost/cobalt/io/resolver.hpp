//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


#ifndef BOOST_COBALT_RESOLVER_HPP
#define BOOST_COBALT_RESOLVER_HPP

#include <boost/cobalt/io/endpoint.hpp>
#include <boost/cobalt/io/ops.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/promise.hpp>

#include <boost/asio/ip/tcp.hpp>


#include <boost/system/result.hpp>
#include <boost/url/url_view.hpp>

namespace boost::cobalt::io
{

struct resolver
{
  resolver(const executor & exec = this_thread::get_executor());
  resolver(resolver && ) = delete;

  void cancel();

 private:
  struct [[nodiscard]] resolve_op_ final : op<system::error_code, endpoint_sequence>
  {
    BOOST_COBALT_IO_DECL
    void initiate(completion_handler<system::error_code, endpoint_sequence> h) override;

    resolve_op_(asio::ip::basic_resolver<protocol_type, executor> & resolver,
                std::string_view host, std::string_view service)
        : resolver_(resolver), host_(host), service_(service) {}
   private:
    asio::ip::basic_resolver<protocol_type, executor> & resolver_;
    std::string_view host_;
    std::string_view service_;

  };

 public:
  [[nodiscard]] auto resolve(std::string_view host, std::string_view service)
  {
    return resolve_op_{resolver_, host, service};
  }


 private:
  asio::ip::basic_resolver<protocol_type, executor> resolver_;
};

struct lookup : op<system::error_code, endpoint_sequence>
{
  lookup(std::string_view host, std::string_view service,
         const executor & exec = this_thread::get_executor())
      : host_(host), service_(service), resolver_{exec} {}
  BOOST_COBALT_IO_DECL void initiate(completion_handler<system::error_code, endpoint_sequence> h) final override;

 private:
  std::string_view host_;
  std::string_view service_;
  asio::ip::basic_resolver<protocol_type, executor> resolver_;
};

}

#endif //BOOST_COBALT_RESOLVER_HPP
