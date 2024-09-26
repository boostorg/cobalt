//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/endpoint.hpp>
#include "../../test.hpp"


BOOST_AUTO_TEST_SUITE(endpoint_);

using namespace boost::cobalt::experimental::io;

/*
 use-case analyzer

  vector<endpoint> eps = resolve("http://www.google.com");
  auto tcc = get<tcp_endpoint>(eps.front());
  assert(ipa.port() == 80);

  endpoint loc{"tcp://127.0.0.1"};
  endpoint unx{"unix:/home/klemens/.local_thingy"};
  unx.set_type(SOCK_DGRAM);
  endpoint uns{local_stream, "/home/klemens/.local_thingy"};

  endpoint ep{tcp_v4, "127.0.0.1", 5000};

  auto ip = get<tcp_b4>(ep);
*/

BOOST_AUTO_TEST_CASE(any_)
{
  endpoint ep{};
  //BOOST_CHECK(ep.protocol() == any_protocol);
  //BOOST_CHECK(!holds_endpoint<local_endpoint>(ep));
}

BOOST_AUTO_TEST_CASE(unix_)
{
  endpoint ep{local_protocol, "~/thingy"};
  BOOST_CHECK(ep.protocol() == local_protocol);
  //BOOST_CHECK(ep.protocol() == any_local);
  BOOST_CHECK(get_if<local_protocol>(&ep));
  BOOST_CHECK(get_if<local_seqpacket>(&ep));
  BOOST_CHECK(get_if<local_datagram>(&ep));
  BOOST_CHECK(get_if<local_stream>(&ep));
  BOOST_CHECK(get<local_protocol>(ep).path() == "~/thingy");

  ep = endpoint{local_stream, "/home/klemens/thingy"};
  BOOST_CHECK(ep.protocol() == local_stream);
  BOOST_CHECK(get_if<local_protocol>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(get_if<local_stream>(&ep));
  BOOST_CHECK(get<local_stream>(ep).path() == "/home/klemens/thingy");
}

BOOST_AUTO_TEST_CASE(ip_v4_)
{
  endpoint ep{ip_v4, "129.168.0.1", 8080};
  BOOST_CHECK(ep.protocol() == ip_v4);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v4>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<tcp_v6>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8080);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "129.168.0.1");

  ep = endpoint{tcp_v4, "192.168.1.4", 8181};
  BOOST_CHECK(ep.protocol() == tcp_v4);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v4>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<tcp_v6>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8181);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "192.168.1.4");
}


BOOST_AUTO_TEST_CASE(ip_v6_)
{
  endpoint ep{ip_v6, "2001:db8::", 8080};
  BOOST_CHECK(ep.protocol() == ip_v6);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v6>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<tcp_v4>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8080);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "2001:db8::");

  ep = endpoint{tcp_v6, "2001:db8:1::ab9:c0a8:102", 8181};
  BOOST_CHECK(ep.protocol() == tcp_v6);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v6>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<tcp_v4>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8181);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "2001:db8:1::ab9:c0a8:102");
}


BOOST_AUTO_TEST_CASE(ip_)
{
  endpoint ep{ip, "129.168.0.1", 8080};
  BOOST_CHECK(ep.protocol() == ip_v4);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v4>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<tcp_v6>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8080);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "129.168.0.1");

  ep = endpoint{tcp, "192.168.1.4", 8181};
  BOOST_CHECK(ep.protocol() == tcp_v4);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v4>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<tcp_v6>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8181);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "192.168.1.4");

  ep = endpoint{ip, "2001:db8::", 8080};
  BOOST_CHECK(ep.protocol() == ip_v6);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v6>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<tcp_v4>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8080);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "2001:db8::");

  ep = endpoint{tcp, "2001:db8:1::ab9:c0a8:102", 8181};
  BOOST_CHECK(ep.protocol() == tcp_v6);
  BOOST_CHECK(get_if<tcp>(&ep));
  BOOST_CHECK(get_if<tcp_v6>(&ep));
  BOOST_CHECK(get_if<ip>(&ep));
  BOOST_CHECK(get_if<ip_v6>(&ep));
  BOOST_CHECK(!get_if<ip_v4>(&ep));
  BOOST_CHECK(!get_if<tcp_v4>(&ep));
  BOOST_CHECK(!get_if<local_seqpacket>(&ep));
  BOOST_CHECK(!get_if<local_datagram>(&ep));
  BOOST_CHECK(!get_if<local_stream>(&ep));
  BOOST_CHECK(get<tcp>(ep).port() == 8181);
  BOOST_CHECK(get<tcp>(ep).addr_str() == "2001:db8:1::ab9:c0a8:102");
}

BOOST_AUTO_TEST_SUITE_END();