//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/endpoint.hpp>
#include <boost/asio/ip/address.hpp>

#include <boost/url/parse.hpp>
#include <boost/url/url_view.hpp>

namespace boost::async::detail
{

void
throw_bad_endpoint_access(
    source_location const &loc)
{
  throw_exception(
      async::io::bad_endpoint_access(), loc);
}

}

namespace boost::async::io
{
static_assert(std::is_standard_layout_v<endpoint>);
static_assert(std::is_standard_layout_v<local_endpoint>);
static_assert(std::is_standard_layout_v<ip_address>);
static_assert(std::is_standard_layout_v<ip_address_v4>);
static_assert(std::is_standard_layout_v<ip_address_v6>);

std::size_t tag_invoke(make_endpoint_tag<AF_UNIX>,
                       boost::asio::detail::socket_addr_type* base,
                       core::string_view sv)
{
  auto un = new (base) boost::asio::detail::sockaddr_un_type{
      .sun_family=AF_UNIX
  };

  if (sv.size() >= sizeof(un->sun_path))
    detail::throw_length_error();

  *std::copy(sv.begin(), sv.end(), un->sun_path) = '\0';
  return sizeof(un->sun_family + sv.size() + 1u);
}

const local_endpoint* tag_invoke(get_endpoint_tag<AF_UNIX>,
                                 protocol_type actual,
                                 const endpoint::addr_type * addr)
{
  if (actual.family() != AF_UNIX)
    return nullptr;
  return reinterpret_cast<const local_endpoint*>(addr);
}

static_string<15> ip_address_v4::addr_str() const
{
  char buf[16];
  inet_ntop(AF_INET, &in_.sin_addr.s_addr, buf, sizeof(buf));
  return buf;
}


std::size_t tag_invoke(make_endpoint_tag<AF_INET>,
                       boost::asio::detail::socket_addr_type* base,
                       std::uint32_t address,
                       std::uint16_t port)
{
  new (base) boost::asio::detail::sockaddr_in4_type{
      .sin_family=AF_INET,
      .sin_port=port,
      .sin_addr={address}
  };
  return sizeof(boost::asio::detail::sockaddr_in4_type);
}

std::size_t tag_invoke(make_endpoint_tag<AF_INET>,
                       boost::asio::detail::socket_addr_type* base,
                       std::string_view address,
                       std::uint16_t port)
{
  if (address.size() > 15)
    detail::throw_length_error();
  static_string<16> addr{address.begin(), address.end()};


  auto a = boost::asio::ip::address_v4::from_string(addr.c_str()).to_uint();
  auto in4 = new (base) boost::asio::detail::sockaddr_in4_type{
      .sin_family=AF_INET,
      .sin_port=port,
      .sin_addr={boost::asio::detail::socket_ops::host_to_network_long(a)}
  };
  return sizeof(boost::asio::detail::sockaddr_in4_type);
}

const ip_address_v4* tag_invoke(get_endpoint_tag<AF_INET>,
                                protocol_type actual,
                                const endpoint::addr_type * addr)
{
  if (actual.family() != AF_INET)
    return nullptr;
  return reinterpret_cast<const ip_address_v4*>(addr);
}


static_string<45> ip_address_v6::addr_str() const
{
  char buf[46];
  inet_ntop(AF_INET6, &in_.sin6_addr.s6_addr, buf, sizeof(buf));
  return buf;
}


std::size_t tag_invoke(make_endpoint_tag<AF_INET6>,
                       boost::asio::detail::socket_addr_type* base,
                       std::span<std::uint8_t, 16> address,
                       std::uint16_t port)
{
  auto in6 = new (base) boost::asio::detail::sockaddr_in6_type{
      .sin6_family=AF_INET6,
      .sin6_port=port
  };
  std::copy(address.begin(), address.end(), in6->sin6_addr.s6_addr);
  return sizeof(boost::asio::detail::sockaddr_in6_type);
}

std::size_t tag_invoke(make_endpoint_tag<AF_INET6>,
                       boost::asio::detail::socket_addr_type* base,
                       std::string_view address,
                       std::uint16_t port)
{
  if (address.size() > 45)
    detail::throw_length_error();
  static_string<46> addr{address.begin(), address.end()};

  auto a = boost::asio::ip::address_v6::from_string(addr.c_str()).to_bytes();
  auto in6 = new (base) boost::asio::detail::sockaddr_in6_type{
      .sin6_family=AF_INET6,
      .sin6_port=port
  };
  std::copy(a.begin(), a.end(), in6->sin6_addr.s6_addr);
  return sizeof(boost::asio::detail::sockaddr_in6_type);
}

const ip_address_v6* tag_invoke(get_endpoint_tag<AF_INET6>,
                                protocol_type actual,
                                const endpoint::addr_type * addr)
{
  if (actual.family() != AF_INET6)
    return nullptr;
  return reinterpret_cast<const ip_address_v6*>(addr);
}


std::array<std::uint8_t, 16u> ip_address::addr() const
{
  std::array<std::uint8_t, 16u> res;
  if (addr_.ss_family == AF_INET)
  {
    const auto & in = in_.sin_addr.s_addr;
    const auto c = reinterpret_cast<const std::uint8_t*>(&in);
    auto itr = std::fill_n(res.begin(), 12u, 0u);
    std::copy(c, c + 4, itr);
  }
  else
  {
    const auto & in = in6_.sin6_addr.s6_addr;
    std::copy(std::begin(in), std::end(in), res.begin());
  }
  return res;
}

static_string<45> ip_address::addr_str() const
{
  char buf[46];
  inet_ntop(addr_.ss_family,
            (addr_.ss_family == AF_INET6) ?
            static_cast<const void*>(&in6_.sin6_addr.s6_addr) : &in_.sin_addr.s_addr,
            buf, sizeof(buf));
  return buf;
}

std::size_t tag_invoke(make_endpoint_tag<AF_UNSPEC>,
                       boost::asio::detail::socket_addr_type* base,
                       std::string_view address,
                       std::uint16_t port)
{
  if (address.size() > 45)
    detail::throw_length_error();
  static_string<46> addr{address.begin(), address.end()};

  auto ad = boost::asio::ip::address::from_string(addr.c_str());
  if (ad.is_v4())
  {
    new (base) boost::asio::detail::sockaddr_in4_type{
        .sin_family=AF_INET,
        .sin_port=port,
        .sin_addr={boost::asio::detail::socket_ops::network_to_host_long(ad.to_v4().to_uint())}
    };
    return sizeof(boost::asio::detail::sockaddr_in4_type);
  }
  else
  {
    auto in6 = new (base) boost::asio::detail::sockaddr_in6_type{
        .sin6_family=AF_INET6,
        .sin6_port=port
    };
    auto a = ad.to_v6().to_bytes();
    std::copy(a.begin(), a.end(), in6->sin6_addr.s6_addr);
    return sizeof(boost::asio::detail::sockaddr_in4_type);
  }
}

const ip_address* tag_invoke(get_endpoint_tag<AF_UNSPEC>,
                             protocol_type actual,
                             const endpoint::addr_type * addr)
{
  if ((actual.family() != AF_INET) && (actual.family() != AF_INET6))
    return nullptr;
  return reinterpret_cast<const ip_address*>(addr);
}

}