enable_testing()

add_library(static_tests concepts.cpp util.cpp this_coro.cpp  )
target_link_libraries(static_tests Boost::container)

add_executable(wrappers wrappers.cpp)
target_link_libraries(wrappers Boost::container Boost::filesystem Boost::atomic )

add_executable(main main.cpp)
add_executable(unit_tests doctest.cpp async.cpp with.cpp thread.cpp op.cpp handler.cpp select.cpp channel.cpp)

target_link_libraries(main       Boost::container Boost::filesystem Boost::atomic uring OpenSSL::SSL)
target_link_libraries(unit_tests Boost::container Boost::filesystem Boost::atomic uring OpenSSL::SSL)

add_executable(tdd tdd.cpp)
target_link_libraries(tdd Boost::container Boost::async uring OpenSSL::SSL)

add_test(NAME boost_async-main COMMAND main)
add_test(NAME boost_async-unit_tests COMMAND unit_tests)
add_test(NAME boost_async-wrappers COMMAND wrappers)

