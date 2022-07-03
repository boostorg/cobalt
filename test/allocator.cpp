// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <coro/allocator.hpp>

#include "doctest.h"
#include "test.hpp"

struct allocate_test {};

template<typename Allocator>
struct allocate_test_promise
        : coro::promise_allocator_arg_base<Allocator>,
          coro::enable_await_allocator<allocate_test_promise<Allocator>>
{
    using coro::promise_allocator_arg_base<Allocator>::promise_allocator_arg_base;

    std::suspend_never initial_suspend() {return {};}
    std::suspend_never final_suspend() noexcept {return {};}

    allocate_test get_return_object() {return {};}
    void unhandled_exception() {throw;}
};

static_assert(
        std::is_same_v<
                typename coro::promise_allocator_arg_type<int, std::allocator_arg_t, double>::type,
                double>);

namespace std
{

template<typename ... Args>
struct coroutine_traits<allocate_test, Args...>
{
    using allocator_type = typename coro::promise_allocator_arg_type<Args...>::type;
    using promise_type = allocate_test_promise<allocator_type>;
};

}

template<typename Value = void>
struct tracked_allocator
{
    using value_type = Value;
    std::vector<std::pair<void*, std::size_t>>   & allocs, &deallocs;

    tracked_allocator(std::vector<std::pair<void*, std::size_t>>   & allocs,
                      std::vector<std::pair<void*, std::size_t>>   & deallocs) : allocs(allocs), deallocs(deallocs) {}


    template<typename T>
    tracked_allocator(const tracked_allocator<T> & a) : allocs(a.allocs), deallocs(a.deallocs) {}

    Value* allocate(std::size_t n)
    {
        auto p = new Value[n];
        allocs.emplace_back(p, n);
        return p;
    }

    void deallocate(Value* p, std::size_t n)
    {
        deallocs.emplace_back(p, n);
        delete[] p;
        CHECK(allocs.back() == deallocs.back());
    }

    bool operator==(const tracked_allocator & rhs) const
    {
        return &allocs == &rhs.allocs
            && &deallocs == &rhs.deallocs;
    }
};


TEST_SUITE_BEGIN("allocator");


template<typename ... Args>
allocate_test test_stdalloc(Args && ... args)
{
    CHECK(std::allocator<void>() == co_await coro::this_coro::allocator);
}

template<typename Alloc, typename ... Args>
allocate_test test_alloc(Alloc & alloc, Args && ... args)
{
    CHECK(alloc == co_await coro::this_coro::allocator);
}


CO_TEST_CASE("allocator")
{
    SUBCASE("alloc_void")      test_stdalloc();
    SUBCASE("alloc_void_args") test_stdalloc(2,1,2);
    SUBCASE("alloc_void_arg")  test_stdalloc(std::allocator_arg, std::allocator<void>());
    SUBCASE("alloc_void_arg_args")  test_stdalloc(1,2,std::allocator_arg, std::allocator<void>(), 3);

    std::vector<std::pair<void*, std::size_t>> allocs, deallocs;
    tracked_allocator<void> ta{allocs, deallocs};
    
    SUBCASE("tracked_alloc")
    {
        test_alloc(ta, std::allocator_arg, ta);
        CHECK(allocs.size() == 1u);
        CHECK(deallocs.size() == 1u);
    }

    SUBCASE("tracked_alloc_2")
    {
        test_alloc(ta, 1,2,3, std::allocator_arg, ta, 4);
        CHECK(allocs.size() == 1u);
        CHECK(deallocs.size() == 1u);
    }
    co_return ;
}


TEST_SUITE_END();