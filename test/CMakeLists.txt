add_library(static_tests concepts.cpp util.cpp this_coro.cpp ops.cpp handler.cpp )

add_executable(wrappers wrappers.cpp)


add_executable(main main.cpp )
add_executable(unit_tests doctest.cpp async.cpp allocator.cpp compose.cpp basic_coro.cpp with.cpp)