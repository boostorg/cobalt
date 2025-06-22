//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt.hpp>
#include <boost/cobalt/main.hpp>
#include <boost/cobalt/io/stream_socket.hpp>

#include <boost/cobalt/io/acceptor.hpp>
#include <boost/cobalt/io/write.hpp>
#include <list>

// tag::decls[]
namespace cobalt = boost::cobalt;
namespace this_coro = boost::cobalt::this_coro;
//end::decls[]

// tag::echo[]
cobalt::promise<void> echo(cobalt::io::stream_socket socket)
{
  try // <1>
  {
    char data[4096];
    while (socket.is_open()) // <2>
    {
      std::size_t n = co_await socket.read_some(boost::asio::buffer(data)); // <3>
      co_await cobalt::io::write(socket, boost::asio::buffer(data, n)); // <4>
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo: exception: %s\n", e.what());
  }
}
// end::echo[]


// tag::listen[]
cobalt::generator<cobalt::io::stream_socket> listen()
{
  cobalt::io::acceptor acceptor(cobalt::io::endpoint{cobalt::io::tcp_v4, "0.0.0.0", 55555});
  for (;;) // <1>
  {
    cobalt::io::stream_socket sock = co_await acceptor.accept(); // <2>
    co_yield std::move(sock); // <3>
  }
  co_return cobalt::io::stream_socket{}; // <4>
}
// end::listen[]

// tag::run_server[]
cobalt::promise<void> run_server(cobalt::wait_group & workers)
{
  auto l = listen(); // <1>
  while (true)
  {
    if (workers.size() == 10u)
      co_await workers.wait_one();  // <2>
    else
      workers.push_back(echo(co_await l)); // <3>
  }
}
// end::run_server[]

// tag::main[]
cobalt::main co_main(int argc, char ** argv)
{
  co_await cobalt::with(cobalt::wait_group(), &run_server); // <1>
  co_return 0u;
}
// end::main[]
