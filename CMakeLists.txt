# Copyright 2020-2023 Peter Dimov
# Copyright 2023 Alexander Grund
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.12...3.20)

if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_COBALT_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_cobalt VERSION "${BOOST_COBALT_VERSION}" LANGUAGES CXX)

set(BOOST_COBALT_IS_ROOT OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(BOOST_COBALT_IS_ROOT ON)
endif()

set(BOOST_COBALT_USE_BOOST_CONTAINER_DEF OFF)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CLANG_VERSION_MAJOR LESS 16)
    set(BOOST_COBALT_USE_BOOST_CONTAINER_DEF ON)
endif()

option(BOOST_COBALT_USE_BOOST_CONTAINER "Use boost.container instead of std::pmr" ${BOOST_COBALT_USE_BOOST_CONTAINER_DEF})


if (NOT BOOST_COBALT_IS_ROOT)

    add_library(boost_cobalt
            src/channel.cpp
            src/error.cpp
            src/main.cpp
            src/this_thread.cpp
            src/thread.cpp
    )

    add_library(Boost::cobalt ALIAS boost_cobalt)

    target_include_directories(boost_cobalt PUBLIC include)

    target_link_libraries(boost_cobalt
            PUBLIC
            Boost::asio
            Boost::circular_buffer
            Boost::config
            Boost::core
            Boost::intrusive
            Boost::leaf
            Boost::mp11
            Boost::preprocessor
            Boost::smart_ptr
            Boost::system
            Boost::throw_exception
            Boost::variant2
    )

    target_compile_definitions(boost_cobalt
            PUBLIC BOOST_COBALT_NO_LIB
            PRIVATE BOOST_COBALT_SOURCE
    )
    target_compile_features(boost_cobalt PUBLIC cxx_std_20)

    if (BOOST_COBALT_USE_BOOST_CONTAINER)
        target_link_libraries(boost_cobalt PUBLIC Boost::container)
        target_compile_definitions(boost_cobalt PUBLIC BOOST_COBALT_USE_BOOST_CONTAINER_PMR=1 )
    endif()

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(boost_cobalt PUBLIC BOOST_COBALT_DYN_LINK)
    else()
        target_compile_definitions(boost_cobalt PUBLIC BOOST_COBALT_STATIC_LINK)
    endif()

    if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
        add_subdirectory(test)
    endif()
else()
    option(BOOST_COBALT_BUILD_INLINE "Configure as if part of the boost source tree" OFF)

    file(GLOB_RECURSE ADOC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.adoc)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html
                       COMMAND asciidoctor ${CMAKE_CURRENT_SOURCE_DIR}/doc/index.adoc --require asciidoctor-diagram --require asciidoctor-multipage -b multipage_html5  -o ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html
                       DEPENDS ${ADOC_FILES})

    add_custom_target(boost_cobalt_doc DEPENDS  ${CMAKE_CURRENT_BINARY_DIR}/doc/index.html)


    include(CTest)
    option(BOOST_COBALT_INSTALL "Install boost::cobalt files" ON)
    option(BOOST_COBALT_BUILD_TESTS "Build boost::cobalt tests" ${BUILD_TESTING})
    option(BOOST_COBALT_BUILD_EXAMPLES "Build boost::cobalt examples" ON)
    option(BOOST_COBALT_BUILD_BENCHMARKS "Build boost::cobalt benchmarks" OFF)

    if(BOOST_COBALT_BUILD_INLINE)
        #
        # Building inside Boost tree, but as a separate project e.g. on Travis or
        # other CI, or when producing Visual Studio Solution and Projects.

        set(BOOST_INCLUDE_LIBRARIES cobalt)
        set(BOOST_EXCLUDE_LIBRARIES cobalt)

        set(CMAKE_FOLDER _deps)
        add_subdirectory(../.. _deps/boost EXCLUDE_FROM_ALL)
        unset(CMAKE_FOLDER)
    endif()

    if (NOT BOOST_COBALT_BUILD_INLINE)
        find_package(Threads REQUIRED)
        # Boost 1.82 is the first with a Boost.ASIO with necessary support
        find_package(Boost 1.82 REQUIRED COMPONENTS system OPTIONAL_COMPONENTS json context url)
        if (BOOST_COBALT_USE_BOOST_CONTAINER)
            find_package(Boost REQUIRED container)
        endif()
    endif()

    add_library(boost_cobalt
                src/detail/exception.cpp
                src/detail/util.cpp
                src/error.cpp
                src/channel.cpp
                src/main.cpp
                src/this_thread.cpp
                src/thread.cpp)

    target_include_directories(boost_cobalt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_link_libraries(boost_cobalt PUBLIC
            Boost::system
            Threads::Threads)
    target_compile_definitions(boost_cobalt PRIVATE BOOST_COBALT_SOURCE=1 )

    if (BOOST_COBALT_USE_BOOST_CONTAINER)
        target_link_libraries(boost_cobalt PUBLIC Boost::container)
        target_compile_definitions(boost_cobalt PUBLIC BOOST_COBALT_USE_BOOST_CONTAINER_PMR=1 )
    endif()

    target_compile_features(boost_cobalt PUBLIC cxx_std_20)
    add_library(Boost::cobalt ALIAS boost_cobalt)

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(boost_cobalt PUBLIC BOOST_COBALT_DYN_LINK=1)
    else()
        target_compile_definitions(boost_cobalt PUBLIC BOOST_COBALT_STATIC_LINK=1)
    endif()

    if(BOOST_COBALT_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
        install(TARGETS boost_cobalt
                RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
                LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                )
    endif()

    if(BOOST_COBALT_BUILD_TESTS)
        add_subdirectory(test)
    endif()

    if(BOOST_COBALT_BUILD_EXAMPLES)
        add_subdirectory(example)
        set_target_properties(boost_cobalt PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    if(BOOST_COBALT_BUILD_BENCHMARKS)
        add_subdirectory(bench)
    endif()
endif()

