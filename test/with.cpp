//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/promise.hpp>
#include <boost/async/op.hpp>
#include <boost/async/with.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/this_coro.hpp>

#include "doctest.h"
#include "test.hpp"

TEST_SUITE_BEGIN("with");

namespace asio = boost::asio;

struct finalizer_test
{
    using executor_type = boost::async::executor ;
    executor_type exec;
    executor_type get_executor()
    {
        return exec;
    }

    bool exit_called = false;
    std::exception_ptr e;

    auto exit(std::exception_ptr ee)
    {
        exit_called = true;
        e = ee;
        return asio::post(exec, boost::async::use_op);
    }

    ~finalizer_test()
    {
        CHECK(e != nullptr);
    }
};

auto tag_invoke(const boost::async::with_exit_tag & wet , finalizer_test * ft,
                std::exception_ptr e)
{

    return ft->exit(e);
}

CO_TEST_CASE("sync")
{
    finalizer_test ft{co_await boost::async::this_coro::executor};

    CHECK_THROWS(
        co_await boost::async::with (
            &ft,
            [](finalizer_test * ft)
            {
                throw std::runtime_error("42");
            }));


    CHECK(ft.e != nullptr);
}

CO_TEST_CASE("async")
{
    finalizer_test ft{co_await boost::async::this_coro::executor};

    CHECK_THROWS(
            co_await boost::async::with (
                    &ft,
                    [](finalizer_test * ft) -> boost::async::promise<void>
                    {
                      throw std::runtime_error("42");
                      co_return;
                    }));


    CHECK(ft.e != nullptr);
}


TEST_SUITE_END();