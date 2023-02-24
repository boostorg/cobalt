enable_testing()

add_library(static_tests concepts.cpp util.cpp this_coro.cpp  )
target_link_libraries(static_tests Boost::container)

add_executable(main main.cpp)
add_executable(unit_tests doctest.cpp promise.cpp with.cpp thread.cpp op.cpp handler.cpp select.cpp
                          channel.cpp generator.cpp run.cpp task.cpp wait.cpp wait_group.cpp wrappers.cpp)

target_link_libraries(main       Boost::container Boost::filesystem Boost::async OpenSSL::SSL)
target_link_libraries(unit_tests Boost::container Boost::filesystem Boost::async OpenSSL::SSL)

add_test(NAME boost_async-main COMMAND main)
add_test(NAME boost_async-unit_tests COMMAND unit_tests)

