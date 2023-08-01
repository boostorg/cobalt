//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/detail/duplicate.hpp>

#if defined(BOOST_ASIO_WINDOWS)
#include <windows.h>
#else
// implement windows shite
#include <unistd.h>
#endif

namespace boost::async::detail::io
{

#if defined(BOOST_ASIO_WINDOWS)

system::result<HANDLE> duplicate_handle(HANDLE h)
{
  HANDLE ho;
  if (!::DuplicateHandle(
        ::GetCurrentProcess(),
        h,
        ::GetCurrentProcess(),
        &ho,
        0,
        TRUE,
        DUPLICATE_SAME_ACCESS))
    return system::error_code(::GetLastError(), system::system_category());
  return ho;
}

system::result<SOCKET> duplicate_handle(SOCKET sock)
{
  WSAPROTOCOL_INFOW info;
  if (SOCKET_ERROR == ::WSADuplicateSocketW(
            sock,
            ::GetCurrentProcessId(),
            &info
            ))
    return system::error_code(::WSAGetLastError(), system::system_category());

  auto res = WSASocketW(
    FROM_PROTOCOL_INFO,
    FROM_PROTOCOL_INFO,
    FROM_PROTOCOL_INFO,
    &info,
    0,
    0);
  if (res == INVALID_SOCKET)
    return system::error_code(::WSAGetLastError(), system::system_category());


  return res;
}

#else

system::result<int> duplicate_handle(int fd)
{
  auto r = dup(fd);
  if (r == -1)
    return system::error_code(errno, system::system_category());
  return r;
}

system::result<int> duplicate_socket(int fd) {return duplicate_handle(fd);}

#endif
}

