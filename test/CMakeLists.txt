add_library(static_tests concepts.cpp util.cpp this_coro.cpp handler.cpp )
target_link_libraries(static_tests Boost::container)

add_executable(wrappers wrappers.cpp)
target_link_libraries(wrappers Boost::container)

add_executable(main main.cpp ../include/boost/async.hpp ../include/boost/async/io/buffer.hpp ../include/boost/async/io/concepts.hpp ../include/boost/async/io/timer/deadline_timer.hpp ../include/boost/async/io.hpp ../include/boost/async/io/timer/impl/deadline_timer.ipp ../include/boost/async/io/timer.hpp ../include/boost/async/io/timer/impl/high_resolution_timer.ipp ../include/boost/async/io/timer/high_resolution_timer.hpp ../include/boost/async/io/signal_set.hpp ../include/boost/async/io/impl/signal_set.ipp ../include/boost/async/io/pipe.hpp ../include/boost/async/io/impl/pipe.ipp ../include/boost/async/io/detail/completion_condition.hpp ../include/boost/async/io/read.hpp ../include/boost/async/io/impl/read.hpp ../include/boost/async/io/detail/match_condition.hpp ../include/boost/async/io/impl/read_until.hpp)
add_executable(unit_tests doctest.cpp async.cpp with.cpp thread.cpp op.cpp pipe.cpp)

target_link_libraries(main Boost::container)
target_link_libraries(unit_tests Boost::container)