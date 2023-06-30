add_subdirectory(buffers)

add_executable(boost_async_io_tests
        endpoint.cpp
        pipe.cpp
        resolver.cpp
        sleep.cpp
        ../doctest.cpp)
target_link_libraries(boost_async_io_tests  Boost::container Boost::async OpenSSL::SSL)
add_test(NAME boost_async-io COMMAND boost_async_io_tests)
