enable_testing()

add_library(static_tests concepts.cpp util.cpp )
target_link_libraries(static_tests Boost::container)

add_executable(boost_async_main         main.cpp)
add_executable(boost_async_main_compile main_compile.cpp)
add_executable(boost_async_basic_tests
      async_for.cpp doctest.cpp promise.cpp with.cpp op.cpp handler.cpp join.cpp select.cpp this_coro.cpp leaf.cpp
      channel.cpp generator.cpp run.cpp task.cpp gather.cpp wait_group.cpp wrappers.cpp async_for.cpp)

target_link_libraries(boost_async_main         Boost::container Boost::async OpenSSL::SSL)
target_link_libraries(boost_async_main_compile Boost::container Boost::async OpenSSL::SSL)
target_link_libraries(boost_async_basic_tests  Boost::container Boost::async OpenSSL::SSL)

add_test(NAME boost_async-main COMMAND boost_async_main)
add_test(NAME boost_async-basic_tests COMMAND boost_async_basic_tests)
add_test(NAME boost_async-io-buffers COMMAND boost_async_io_buffers_tests)

file(GLOB ALL_PUBLIC_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/include/async ${CMAKE_SOURCE_DIR}/include/boost/async/*.hpp)
set(ALL_PUBLIC_HEADER_TESTS "")
foreach(HEADER ${ALL_PUBLIC_HEADERS})
    get_filename_component(NAME ${HEADER} NAME_WLE)
    set(FNAME "${CMAKE_CURRENT_BINARY_DIR}/test_include_${NAME}.cpp")
    file(GENERATE OUTPUT ${FNAME}
            CONTENT "#include <boost/async/${NAME}.hpp>")
    list(APPEND ALL_PUBLIC_HEADER_TESTS ${FNAME})
endforeach()

add_library(boost_async_test_all_public_headers_test OBJECT ${ALL_PUBLIC_HEADER_TESTS})
target_link_libraries(boost_async_test_all_public_headers_test PUBLIC Boost::headers)