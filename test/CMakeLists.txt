add_library(static_tests concepts.cpp util.cpp this_coro.cpp handler.cpp )
target_link_libraries(static_tests Boost::container)

add_executable(wrappers wrappers.cpp)
target_link_libraries(wrappers Boost::container)

add_executable(main main.cpp)
add_executable(unit_tests doctest.cpp async.cpp with.cpp thread.cpp op.cpp)

target_link_libraries(main Boost::container)
target_link_libraries(unit_tests Boost::container)