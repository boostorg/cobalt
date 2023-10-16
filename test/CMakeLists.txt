enable_testing()

add_library(static_tests concepts.cpp util.cpp )
target_link_libraries(static_tests Boost::headers)

add_executable(boost_cobalt_main         main.cpp)
add_executable(boost_cobalt_main_compile main_compile.cpp)
add_executable(boost_cobalt_basic_tests
      async_for.cpp doctest.cpp promise.cpp with.cpp op.cpp handler.cpp join.cpp race.cpp this_coro.cpp leaf.cpp
      channel.cpp generator.cpp run.cpp task.cpp gather.cpp wait_group.cpp wrappers.cpp left_race.cpp
      strand.cpp fork.cpp thread.cpp any_completion_handler.cpp detached.cpp monotonic_resource.cpp sbo_resource.cpp)

target_link_libraries(boost_cobalt_main         Boost::cobalt OpenSSL::SSL)
target_link_libraries(boost_cobalt_main_compile Boost::cobalt OpenSSL::SSL)
target_link_libraries(boost_cobalt_basic_tests  Boost::cobalt OpenSSL::SSL)

add_test(NAME boost_cobalt-main COMMAND boost_cobalt_main)
add_test(NAME boost_cobalt-basic_tests COMMAND boost_cobalt_basic_tests)

file(GLOB ALL_PUBLIC_HEADERS RELATIVE ${CMAKE_SOURCE_DIR}/include/cobalt ${CMAKE_SOURCE_DIR}/include/boost/cobalt/*.hpp)
set(ALL_PUBLIC_HEADER_TESTS "")
foreach(HEADER ${ALL_PUBLIC_HEADERS})
    get_filename_component(NAME ${HEADER} NAME_WLE)
    set(FNAME "${CMAKE_CURRENT_BINARY_DIR}/test_include_${NAME}.cpp")
    file(GENERATE OUTPUT ${FNAME}
            CONTENT "#include <boost/cobalt/${NAME}.hpp>")
    list(APPEND ALL_PUBLIC_HEADER_TESTS ${FNAME})
endforeach()

add_library(boost_cobalt_test_all_public_headers_test OBJECT ${ALL_PUBLIC_HEADER_TESTS})
target_link_libraries(boost_cobalt_test_all_public_headers_test PUBLIC Boost::headers)