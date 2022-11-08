add_library(static_tests concepts.cpp util.cpp this_coro.cpp handler.cpp )

add_executable(wrappers wrappers.cpp)


add_executable(main main.cpp ../include/boost/async/thread.hpp ../include/boost/async/op.hpp ../include/boost/async/this_thread.hpp ../include/boost/async/impl/this_thread.ipp ../include/boost/async/src.hpp)
add_executable(unit_tests doctest.cpp async.cpp with.cpp thread.cpp op.cpp)