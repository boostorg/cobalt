// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_TEST2_HPP
#define BOOST_ASYNC_TEST2_HPP

#include <boost/async/test.hpp>

// tag::test_case_macro[]
#define CO_TEST_CASE_IMPL(Function, ...)     \
static ::boost::async::test_case Function(); \
DOCTEST_TEST_CASE(__VA_ARGS__)               \
{                                            \
    run(Function());                         \
}                                            \
static ::boost::async::test_case Function()

#define CO_TEST_CASE(...) CO_TEST_CASE_IMPL(DOCTEST_ANONYMOUS(CO_DOCTEST_ANON_FUNC_), __VA_ARGS__)
// end::test_case_macro[]

#endif //BOOST_ASYNC_TEST2_HPP
