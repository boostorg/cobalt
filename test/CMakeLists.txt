add_library(static_tests concepts.cpp util.cpp this_coro.cpp handler.cpp )

add_executable(wrappers wrappers.cpp)


add_executable(main main.cpp ../include/coro/thread.hpp ../include/coro/op.hpp ../include/coro/this_thread.hpp)
add_executable(unit_tests doctest.cpp async.cpp with.cpp thread.cpp op.cpp)