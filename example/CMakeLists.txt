
file(GLOB_RECURSE ALL_EXAMPLES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)


foreach(SRC ${ALL_EXAMPLES})
    get_filename_component(NAME ${SRC} NAME_WLE )
    add_executable(boost_async_example_${NAME} ${SRC} )
    target_link_libraries(boost_async_example_${NAME} PUBLIC
                          Boost::async)
    target_compile_definitions(boost_async_example_${NAME} PUBLIC)
endforeach()
