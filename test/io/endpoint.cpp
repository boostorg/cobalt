//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/endpoint.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/datagram_protocol.hpp>
#include <boost/asio/local/seq_packet_protocol.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(io);

BOOST_AUTO_TEST_CASE(protocol)
{
  boost::cobalt::io::protocol_type pt{boost::asio::ip::tcp::v4()};

  BOOST_CHECK(pt == boost::cobalt::io::tcp_v4);
  pt = boost::asio::ip::tcp::v6();
  BOOST_CHECK(pt == boost::cobalt::io::tcp_v6);

  pt = boost::asio::local::datagram_protocol();
  BOOST_CHECK(pt == boost::cobalt::io::local_datagram);

  pt = boost::asio::local::seq_packet_protocol();
  BOOST_CHECK(pt == boost::cobalt::io::local_seqpacket);
}

BOOST_AUTO_TEST_CASE(endpoint)
{
  boost::cobalt::io::endpoint ep = boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), 1234);
  BOOST_CHECK(ep.protocol() == boost::cobalt::io::tcp_v4);
  BOOST_CHECK(get<boost::cobalt::io::tcp>(ep).port() == 1234);
  BOOST_CHECK(!get_if<boost::cobalt::io::tcp_v6>(&ep));
}

BOOST_AUTO_TEST_CASE(construct)
{
  boost::cobalt::io::endpoint ep{
    boost::cobalt::io::tcp_v4, "127.0.0.1", 1234
  };

  boost::cobalt::io::protocol_type pt{AF_INET, SOCK_STREAM, IPPROTO_TCP};
}

BOOST_AUTO_TEST_SUITE_END();
