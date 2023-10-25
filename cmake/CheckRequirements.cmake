# Copyright (c) 2023 Klemens D. Morgenstern
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


if(NOT cxx_std_20 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    message(STATUS "Boost.Cobalt: not building, compiler doesn't support C++20.")
    return()
endif()

if(MSVC_VERSION AND MSVC_VERSION LESS 1930)
    message(STATUS "Boost.Cobalt: not building, the lowest supported MSVC version is 1930.  ${MSVC_VERSION} is not supported")
    return()
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

try_compile(
        BOOST_COBALT_HAS_COROUTINE_INCLUDE
        SOURCE_FROM_CONTENT boost_cobalt_include_std_coroutine.cpp
        "#include <coroutine>
"
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED 20
        OUTPUT_VARIABLE TRY_COMPILE_OUTPUT)

if (NOT BOOST_COBALT_HAS_COROUTINE_INCLUDE)
    message(STATUS "Boost.Cobalt: not building, can't include <coroutine> : ${TRY_COMPILE_OUTPUT}.")
    return()
endif()

try_compile(
        BOOST_COBALT_HAS_CONCEPTS
        SOURCE_FROM_CONTENT boost_cobalt_include_std_cconcepts.cpp
        "#include <concepts>
static_assert(!std::derived_from<int, double>);
static_assert(std::same_as<int, int>);
static_assert(std::convertible_to<int, double>);
"
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED 20
        OUTPUT_VARIABLE TRY_COMPILE_OUTPUT)

if (NOT BOOST_COBALT_HAS_CONCEPTS)
    message(STATUS "Boost.Cobalt: not building, can't include <concepts> or use them: ${TRY_COMPILE_OUTPUT}.")
    return()
endif()

