#
# Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/CPPAlliance/buffers
#

set(PFILES
    CMakeLists.txt
    Jamfile
    test_helpers.hpp
    algorithm.cpp
    any_dynamic_buffer.cpp
    buffer.cpp
    buffer_copy.cpp
    buffer_size.cpp
    buffers.cpp
    circular_buffer.cpp
    const_buffer.cpp
    const_buffer_pair.cpp
    const_buffer_span.cpp
    const_buffer_subspan.cpp
    flat_buffer.cpp
    mutable_buffer.cpp
    mutable_buffer_pair.cpp
    mutable_buffer_span.cpp
    mutable_buffer_subspan.cpp
    range.cpp
    string_buffer.cpp
    tag_invoke.cpp
    test_main.cpp
    type_traits.cpp
    )

add_executable(boost_async_io_buffers_tests ${PFILES} ${EXTRAFILES})

add_test(NAME boost_async_io-buffers_tests COMMAND boost_async_io_buffers_tests)

target_include_directories(boost_async_io_buffers_tests PRIVATE . )
target_link_libraries(
    boost_async_io_buffers_tests PRIVATE
    boost_async
    )
