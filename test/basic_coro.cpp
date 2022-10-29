// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "coro/basic_coro.hpp"
#include <asio/bind_cancellation_slot.hpp>
#include <asio/steady_timer.hpp>
#include <asio/thread_pool.hpp>
#include <asio/detached.hpp>

#include "doctest.h"
#include "test.hpp"

TEST_SUITE_BEGIN("coro");

using namespace coro;


auto coro_simple_cancel_impl(asio::io_context& ) noexcept -> basic_coro<void() noexcept, asio::error_code>
{
    CHECK(asio::cancellation_type::none == (co_await this_coro::cancellation_state).cancelled());
    asio::steady_timer timer{co_await this_coro::executor, std::chrono::seconds(1)};
    CHECK(asio::cancellation_type::none == (co_await this_coro::cancellation_state).cancelled());

    auto [ec] = co_await timer.async_wait(asio::as_tuple(asio::deferred));

    CHECK(asio::cancellation_type::none != (co_await this_coro::cancellation_state).cancelled());

    co_return ec;
}

TEST_CASE("coro_simple_cancel")
{
    asio::io_context ctx;
    asio::cancellation_signal sig;

    auto k = coro_simple_cancel_impl(ctx);

    asio::error_code res_ec;
    k.async_resume(
            asio::bind_cancellation_slot(sig.slot(),
                                         [&](asio::error_code ec) {res_ec = ec;}));
    asio::post(ctx, [&]{sig.emit(asio::cancellation_type::all);});

    CHECK(!res_ec);

    ctx.run();

    CHECK(res_ec == asio::error::operation_aborted);
}

auto coro_throw_cancel_impl(asio::io_context& ) -> basic_coro<void() , void>
{
    asio::steady_timer timer{ co_await this_coro::executor, std::chrono::seconds(1)};
    coro::interpret_result(co_await timer.async_wait(asio::as_tuple(asio::deferred)));
}

TEST_CASE("coro_throw_cancel")
{
    asio::io_context ctx;
    asio::cancellation_signal sig;

    auto k = coro_throw_cancel_impl(ctx);

    std::exception_ptr res_ex;
    k.async_resume(
            asio::bind_cancellation_slot(sig.slot(),
                                         [&](std::exception_ptr ex) {res_ex = ex;}));
    asio::post(ctx, [&]{sig.emit(asio::cancellation_type::all);});

    CHECK(!res_ex);

    ctx.run();

    CHECK(res_ex);
    try
    {
        if (res_ex)
            std::rethrow_exception(res_ex);
    }
    catch (asio::system_error& se)
    {
        CHECK(se.code() == asio::error::operation_aborted);
    }
}

auto coro_simple_cancel_nested_k(asio::io_context&, int& cnt) noexcept
    -> basic_coro<void() noexcept, asio::error_code>
{
    asio::steady_timer timer{
            co_await this_coro::executor,
            std::chrono::milliseconds(100)};

    CHECK(!(co_await this_coro::cancellation_state).cancelled());
    auto [ec] = co_await timer.async_wait(asio::as_tuple(asio::deferred));
    cnt++;
    CHECK((co_await this_coro::cancellation_state).cancelled());

    co_return ec;
}

auto coro_simple_cancel_nested_kouter(
        asio::io_context& ctx, int& cnt) noexcept
    -> basic_coro<asio::error_code() noexcept, asio::error_code>
{
    CHECK(cnt == 0);
    co_yield co_await coro_simple_cancel_nested_k(ctx, cnt);
    CHECK(cnt == 1);
    auto ec = co_await coro_simple_cancel_nested_k(ctx, cnt);
    CHECK(cnt == 2);
    co_return ec;
}

TEST_CASE("coro_simple_cancel_nested")
{
    asio::io_context ctx;
    asio::cancellation_signal sig;

    int cnt = 0;
    auto kouter = coro_simple_cancel_nested_kouter(ctx, cnt);

    asio::error_code res_ec;
    kouter.async_resume(
            asio::bind_cancellation_slot(sig.slot(),
                                         [&](asio::error_code ec) {res_ec = ec;}));
    asio::post(ctx, [&]{sig.emit(asio::cancellation_type::all);});
    CHECK(!res_ec);
    ctx.run();
    CHECK(res_ec == asio::error::operation_aborted);

    ctx.restart();
    res_ec = {};
    kouter.async_resume(
            asio::bind_cancellation_slot(sig.slot(),
                                         [&](asio::error_code ec) {res_ec = ec;}));
    asio::post(ctx, [&]{sig.emit(asio::cancellation_type::all);});
    CHECK(!res_ec);
    ctx.run();
    CHECK(res_ec == asio::error::operation_aborted);
    CHECK(cnt == 2);
}


auto coro_simple_co_spawn_impl(asio::io_context& , bool &done) noexcept -> basic_coro<void() noexcept, int>
{
    asio::steady_timer timer(
            co_await this_coro::executor,
            std::chrono::milliseconds(10));

    done = true;

    co_return 42;
}

TEST_CASE("coro_co_spawn")
{
    asio::io_context ctx;

    bool done1 = false;
    bool done2 = false;
    int res = 0;

    spawn(coro_simple_co_spawn_impl(ctx, done1), [&](int r){done2= true;  res = r;});

    ctx.run();

    CHECK(done1);
    CHECK(done2);
    CHECK(res == 42);
}


template <typename Func>
struct on_scope_exit
{
    Func func;

    static_assert(noexcept(func()));

    on_scope_exit(const Func &f)
            : func(static_cast< Func && >(f))
    {
    }

    on_scope_exit(Func &&f)
            : func(f)
    {
    }

    on_scope_exit(const on_scope_exit &) = delete;

    ~on_scope_exit()
    {
        func();
    }
};

basic_coro<int()> throwing_generator(asio::any_io_executor, int &last, bool &destroyed)
{
    on_scope_exit x = [&]() noexcept { destroyed = true; };
    (void)x;

    int i = 0;
    while (i < 3)
        co_yield last = ++i;

    throw std::runtime_error("throwing-generator");
}

CO_TEST_CASE("throwing_generator_test")
{
    int val = 0;
    bool destr = false;
    {
        auto gi = throwing_generator(
                co_await asio::this_coro::executor,
                val, destr);
        bool caught = false;
        try
        {
            for (int i = 0; i < 10; i++)
            {
                CHECK(val == i);
                const auto next = interpret_result(co_await gi.async_resume(asio::deferred));
                CHECK(next);
                CHECK(val == *next);
                CHECK(val == i + 1);
            }
        }
        catch (std::runtime_error &err)
        {
            caught = true;
            using std::operator ""sv;
            CHECK(err.what() == "throwing-generator"sv);
        }
        CHECK(val == 3);
        CHECK(caught);
    }
    CHECK(destr);
};



basic_coro<int(int)> throwing_stacked(
        asio::any_io_executor exec, int &val,
        bool &destroyed_inner, bool &destroyed)
{
    CHECK((co_await asio::this_coro::throw_if_cancelled()));

    on_scope_exit x = [&]() noexcept { destroyed = true; };
    (void)x;

    auto gen = throwing_generator(exec, val, destroyed_inner);
    while (auto next = co_await gen) // 1, 2, 4, 8, ...
        CHECK(42 ==(co_yield *next)); // offset is delayed by one cycle
}

CO_TEST_CASE("throwing_generator_stacked_test")
{
    int val = 0;
    bool destr = false, destr_inner = false;
    {
        auto gi = throwing_stacked(
                co_await asio::this_coro::executor,
                val, destr, destr_inner);
        bool caught = false;
        try
        {
            for (int i = 0; i < 10; i++)
            {
                CHECK(val == i);
                const auto next = interpret_result(co_await gi.async_resume(42, asio::deferred));
                CHECK(next);
                CHECK(val == *next);
                CHECK(val == i + 1);
            }
        }
        catch (std::runtime_error &err)
        {
            caught = true;
            using std::operator ""sv;
            CHECK(err.what() == "throwing-generator"sv);
        }
        CHECK(val == 3);
        CHECK(caught);
    }
    CHECK(destr);
    CHECK(destr_inner);
};


TEST_CASE_TEMPLATE("different_execs", T, int, void)
{
    asio::thread_pool th_ctx{1u};
    asio::io_context ctx;

    auto o = std::make_optional(
            asio::prefer(th_ctx.get_executor(),
                         asio::execution::outstanding_work.tracked));

    static bool ran_inner = false, ran_outer = false;

    struct c_inner_t
    {
        auto operator()(asio::any_io_executor e) -> basic_coro<T()>
        {
            auto p = e.target<asio::thread_pool::executor_type>();
            CHECK(p);
            CHECK(p->running_in_this_thread());
            ran_inner = true;
            co_return;
        };

    };

    c_inner_t c_inner;

    struct c_outer_t
    {

        auto operator()(asio::any_io_executor e, int, basic_coro<T()> tp) -> basic_coro<>
        {
            auto p = e.target<asio::io_context::executor_type>();

            CHECK(p);
            CHECK(p->running_in_this_thread());

            co_await tp;

            CHECK(p->running_in_this_thread());

            ran_outer = true;
        };
    };

    c_outer_t c_outer;

    bool ran = false;
    std::exception_ptr ex;

    auto c = c_outer(ctx.get_executor(), 10, c_inner(th_ctx.get_executor()));
    c.async_resume(
            [&](std::exception_ptr e)
            {
                CHECK(!e);
                ran = true;
            });

    CHECK(!ran);
    ctx.run();
    o.reset();
    CHECK(ran);
    CHECK(ran_inner);
    CHECK(ran_outer);
    CHECK(!ex);

    th_ctx.stop();
    th_ctx.join();
}


TEST_CASE("partial")
{
    asio::io_context ctx;
    bool ran = false;
    auto p = detail::post_coroutine(ctx, [&]{ran = true;});
    CHECK(!ran);
    p.resume();
    CHECK(!ran);
    ctx.run();
    CHECK(ran);
}

namespace coro
{
template struct basic_coro<void(), void, asio::any_io_executor>;
template struct basic_coro<int(), void, asio::any_io_executor>;
template struct basic_coro<void(), int, asio::any_io_executor>;
template struct basic_coro<int(int), void, asio::any_io_executor>;
template struct basic_coro<int(), int, asio::any_io_executor>;
template struct basic_coro<int(int), int, asio::any_io_executor>;

template struct basic_coro<void() noexcept, void, asio::any_io_executor>;
template struct basic_coro<int() noexcept, void, asio::any_io_executor>;
template struct basic_coro<void() noexcept, int, asio::any_io_executor>;
template struct basic_coro<int(int) noexcept, void, asio::any_io_executor>;
template struct basic_coro<int() noexcept, int, asio::any_io_executor>;
template struct basic_coro<int(int) noexcept, int, asio::any_io_executor>;
}


basic_coro<int()> generator_impl(
        asio::any_io_executor, int& last, bool& destroyed)
{
    on_scope_exit x = [&]() noexcept { destroyed = true; };
    (void)x;

    int i = 0;
    while (true)
        co_yield last = ++i;
}

CO_TEST_CASE("generator_test")
{
    int val = 0;
    bool destr = false;
    {
        auto gi = generator_impl(
                co_await asio::this_coro::executor, val, destr);

        for (int i = 0; i < 10; i++)
        {
            CHECK(val == i);
            const auto next = interpret_result(co_await gi.async_resume(asio::deferred));
            CHECK(next);
            CHECK(val == *next);
            CHECK(val == i + 1);
        }

        CHECK(!destr);
    }
    CHECK(destr);
};

basic_coro<void(), int> task_test(asio::io_context&)
{
    co_return 42;
}

basic_coro<void(), int> task_throw(asio::io_context&)
{
    throw std::logic_error(__func__);
    co_return 42;
}

TEST_CASE("run_task_test")
{
    asio::io_context ctx;

    bool tt1 = false;
    bool tt2 = false;
    bool tt3 = false;
    bool tt4 = false;
    auto tt = task_test(ctx);
    tt.async_resume(
            [&](std::exception_ptr pt, int i)
            {
                tt1 = true;
                CHECK(!pt);
                CHECK(i == 42);
                tt.async_resume(
                        [&](std::exception_ptr pt, int)
                        {
                            tt2 = true;
                            CHECK(pt);
                        });
            });

    auto tt_2 = task_throw(ctx);

    tt_2.async_resume(
            [&](std::exception_ptr pt, int)
            {
                tt3 = true;
                CHECK(pt);
                tt.async_resume(
                        [&](std::exception_ptr pt, int)
                        {
                            tt4 = true;
                            CHECK(pt);
                        });
            });

    ctx.run();

    CHECK(tt1);
    CHECK(tt2);
    CHECK(tt3);
    CHECK(tt4);
}

basic_coro<char()> completion_generator_test_impl(
        asio::any_io_executor, int limit)
{
    for (int i = 0; i < limit; i++)
        co_yield i;
}

CO_TEST_CASE("completion_generator_test")
{
    std::vector<int> res;
    auto g = completion_generator_test_impl(
            co_await asio::this_coro::executor, 10);

    CHECK(g.is_open());
    while (auto val = interpret_result(co_await g.async_resume(asio::deferred)))
        res.push_back(*val);

    CHECK(!g.is_open());
    CHECK((res == std::vector{0,1,2,3,4,5,6,7,8,9}));
};



basic_coro<int(int)>
symmetrical_test_impl(asio::any_io_executor)
{
    CHECK(123 == co_await this_coro::initial);
    int i = 0;
    while (true)
        i = (co_yield i) + i;
}

CO_TEST_CASE("symmetrical_test")
{
    auto g = symmetrical_test_impl(co_await asio::this_coro::executor);

    CHECK(g.is_open());

    CHECK(0  == interpret_result(co_await g.async_resume(123, asio::deferred)).value_or(-1));
    CHECK(1  == interpret_result(co_await g.async_resume(1, asio::deferred)).value_or(-1));
    CHECK(3  == interpret_result(co_await g.async_resume(2, asio::deferred)).value_or(-1));
    CHECK(6  == interpret_result(co_await g.async_resume(3, asio::deferred)).value_or(-1));
    CHECK(10 == interpret_result(co_await g.async_resume(4, asio::deferred)).value_or(-1));
    CHECK(15 == interpret_result(co_await g.async_resume(5, asio::deferred)).value_or(-1));
    CHECK(21 == interpret_result(co_await g.async_resume(6, asio::deferred)).value_or(-1));
    CHECK(28 == interpret_result(co_await g.async_resume(7, asio::deferred)).value_or(-1));
    CHECK(36 == interpret_result(co_await g.async_resume(8, asio::deferred)).value_or(-1));
    CHECK(45 == interpret_result(co_await g.async_resume(9, asio::deferred)).value_or(-1));
};


basic_coro<int()>
stack_generator(asio::any_io_executor, int i = 1)
{
    for (;;)
    {
        co_yield i;
        i *= 2;
    }
}

basic_coro<int(int)>
stack_accumulate(asio::any_io_executor exec)
{
    auto gen  = stack_generator(exec);
    int offset = 0;
    while (auto next = co_await gen) // 1, 2, 4, 8, ...
        offset  = co_yield *next + offset; // offset is delayed by one cycle
}

basic_coro<int()>
main_stack_coro(asio::io_context&, bool & done)
{
    auto g = stack_accumulate(co_await asio::this_coro::executor);

    CHECK(g.is_open());
    CHECK(1    == (co_await g(1000)).value_or(-1));
    CHECK(2002 == (co_await g(2000)).value_or(-1));
    CHECK(3004 == (co_await g(3000)).value_or(-1));
    CHECK(4008 == (co_await g(4000)).value_or(-1));
    CHECK(5016 == (co_await g(5000)).value_or(-1));
    CHECK(6032 == (co_await g(6000)).value_or(-1));
    CHECK(7064 == (co_await g(7000)).value_or(-1));
    CHECK(8128 == (co_await g(8000)).value_or(-1));
    CHECK(9256 == (co_await g(9000)).value_or(-1));
    CHECK(511 ==  (co_await g(-1)).value_or(-1));
    done = true;
}

TEST_CASE("stack_test")
{
    bool done = false;
    asio::io_context ctx;
    auto k = main_stack_coro(ctx, done);
    k.async_resume(asio::detached);
    ctx.run();
    CHECK(done);
}


TEST_SUITE_END();