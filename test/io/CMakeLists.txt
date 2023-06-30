add_subdirectory(buffers)

add_executable(boost_async_io_tests
        acceptor.cpp
        datagram_socket.cpp
        stream_socket.cpp
        seq_packet_socket.cpp
        endpoint.cpp
        pipe.cpp
        resolver.cpp
        sleep.cpp
        ssl.cpp
        ../doctest.cpp)
target_link_libraries(boost_async_io_tests  Boost::container Boost::async OpenSSL::SSL)
add_test(NAME boost_async-io COMMAND boost_async_io_tests)
