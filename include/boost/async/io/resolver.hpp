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
  using resolve_result = system::result<pmr::vector<endpoint>>;

  BOOST_ASYNC_DECL resolver(const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL resolver(resolver && ) = delete;

  BOOST_ASYNC_DECL void cancel();

 private:

  struct resolve_op_ final : result_op<resolve_result::value_type>
  {
    resolve_op_(asio::ip::basic_resolver<protocol_type, executor> & resolver,
                core::string_view host, core::string_view service)
                : resolver_(resolver), host_(host), service_(service) {}
    BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, resolve_result::value_type>);
   private:
    asio::ip::basic_resolver<protocol_type, executor> & resolver_;
    core::string_view host_;
    core::string_view service_;
  };
 public:

  [[nodiscard]] resolve_op_ resolve(core::string_view host, core::string_view service)
  {
    return resolve_op_{resolver_, host, service};
  }
 private:
  asio::ip::basic_resolver<protocol_type, executor> resolver_;
};

struct lookup final : result_op<resolver::resolve_result::value_type>
{
  lookup(core::string_view host, core::string_view service)
        : host_(host), service_(service) {}

  BOOST_ASYNC_DECL
  void initiate(completion_handler<system::error_code, resolver::resolve_result::value_type>);

  private:
  core::string_view host_;
  core::string_view service_;
  std::optional<asio::ip::basic_resolver<protocol_type, executor>> resolver_;
};

}

#endif //BOOST_ASYNC_IO_RESOLVER_HPP
