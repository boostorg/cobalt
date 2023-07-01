cmake_minimum_required(VERSION 3.20)

set(BOOST_REQUESTS_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_REQUESTS_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_async VERSION "${BOOST_ASYNC_VERSION}" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(BOOST_ASYNC_IS_ROOT OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(BOOST_ASYNC_IS_ROOT ON)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../boost.css)
    set(BOOST_ASYNC_SHOULD_BE_INLINE ON)
else()
    set(BOOST_ASYNC_SHOULD_BE_INLINE OFF)
endif()

option(BOOST_ASYNC_BUILD_INLINE "Configure as if part of the boost source tree" ${BOOST_ASYNC_SHOULD_BE_INLINE})

file(GLOB_RECURSE ADOC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.adoc)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html
                   COMMAND asciidoctor ${CMAKE_CURRENT_SOURCE_DIR}/doc/async.adoc --require asciidoctor-diagram -o ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html
                   DEPENDS ${ADOC_FILES})

add_custom_target(boost_async_doc DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html)


if(BOOST_ASYNC_IS_ROOT)
    #include(CTest)
endif()
if(NOT BOOST_SUPERPROJECT_VERSION)
    option(BOOST_ASYNC_INSTALL "Install boost::async files" ON)
    option(BOOST_ASYNC_BUILD_TESTS "Build boost::async tests" ${BUILD_TESTING})
    option(BOOST_ASYNC_BUILD_EXAMPLES "Build boost::async examples" ${BOOST_ASYNC_IS_ROOT})
else()
    set(BOOST_ASYNC_BUILD_TESTS ${BUILD_TESTING})
endif()


if(BOOST_ASYNC_IS_ROOT AND BOOST_ASYNC_BUILD_INLINE)
    #
    # Building inside Boost tree, but as a separate project e.g. on Travis or
    # other CI, or when producing Visual Studio Solution and Projects.

    set(BOOST_INCLUDE_LIBRARIES ASYNC)
    set(BOOST_EXCLUDE_LIBRARIES ASYNC)

    set(CMAKE_FOLDER _deps)
    add_subdirectory(../.. _deps/boost EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif()

if (NOT BOOST_ASYNC_BUILD_INLINE)
    find_package(Threads REQUIRED)
    find_package(Boost REQUIRED container system json filesystem)
    include_directories(include)
endif()

find_package(OpenSSL)
if (NOT MSVC)
    link_libraries(${OPENSSL_LIBRARIES})
endif()

add_library(boost_async
        src/detail/exception.cpp
        src/channel.cpp
        src/main.cpp
        src/this_thread.cpp
        src/io/buffers/circular_buffer.cpp
        src/io/buffers/const_buffer_pair.cpp
        src/io/buffers/const_buffer_subspan.cpp
        src/io/buffers/mutable_buffer_pair.cpp
        src/io/buffers/mutable_buffer_subspan.cpp
        src/io/detail/duplicate.cpp
        src/io/detail/stream.cpp
        src/io/acceptor.cpp
        src/io/datagram_socket.cpp
        src/io/endpoint.cpp
        src/io/pipe.cpp
        src/io/resolver.cpp
        src/io/serial_port.cpp
        src/io/signal_set.cpp
        src/io/seq_packet_socket.cpp
        src/io/socket.cpp
        src/io/ssl.cpp
        src/io/steady_timer.cpp
        src/io/stream_socket.cpp
        src/io/system_timer.cpp
        src/io/stream_file.cpp
        src/io/random_access_file.cpp
        src/io/file.cpp
        src/io/read.cpp
        src/io/read_at.cpp
        src/io/read_until.cpp
        src/io/popen.cpp
        src/io/process.cpp
        src/io/buffers/register.cpp
        src/io/write.cpp
        src/io/write_at.cpp
        src/io/detail/random_access_device.cpp
        src/io/copy.cpp)

target_include_directories(boost_async PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(boost_async PUBLIC
        Boost::container Boost::system
        Threads::Threads)
target_compile_definitions(boost_async PRIVATE BOOST_ASYNC_SOURCE=1)
target_compile_definitions(boost_async PUBLIC BOOST_PROCESS_USE_STD_FS=1)

add_library(Boost::async ALIAS boost_async)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_async PUBLIC BOOST_ASYNC_DYN_LINK=1)
else()
    target_compile_definitions(boost_async PUBLIC BOOST_ASYNC_STATIC_LINK=1)
endif()

if(BOOST_ASYNC_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
    install(TARGETS boost_async
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            )
endif()


if(BOOST_ASYNC_BUILD_TESTS)
    add_subdirectory(test)
endif()


if(BOOST_ASYNC_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
