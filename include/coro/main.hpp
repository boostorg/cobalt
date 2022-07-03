// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_MAIN_HPP
#define CORO_MAIN_HPP

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/signal_set.hpp>

#include <coroutine>
#include <optional>

#include <coro/concepts.hpp>
#include <coro/deferred.hpp>
#include <coro/ops.hpp>
#include <coro/this_coro.hpp>

namespace coro
{

namespace detail {struct main_promise;}
struct main { detail::main_promise * promise; };

}

coro::main co_main(int argc, char * argv[]);

namespace coro {

namespace detail
{
extern "C"
{
int main(int argc, char * argv[]);
}

struct signal_helper
{
    asio::cancellation_signal signal;
};


struct main_promise : signal_helper,
                      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                      promise_throw_if_cancelled_base,
                      enable_awaitables<main_promise>,
                      enable_deferred,
                      enable_operations<>

{
    main_promise(int, char **) : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>(
            signal_helper::signal.slot(), asio::enable_total_cancellation())
    {
        [[maybe_unused]] volatile auto p = &detail::main;
    }
    std::suspend_always initial_suspend() {return {};}
    std::suspend_never final_suspend() noexcept
    {
        asio::error_code ec;
        if (signal_set)
            signal_set->cancel(ec);
        return {};
    }


    void unhandled_exception() { throw ; }
    void return_value(int res = 0)
    {
        if (result)
            *result = res;
    }

    friend auto ::co_main(int argc, char * argv[]) -> struct ::coro::main;
    friend int main(int argc, char * argv[])
    {
        asio::io_context ctx;

        ::coro::main mn = co_main(argc, argv);
        int res ;
        mn.promise->result = &res;
        mn.promise->exec = asio::require(ctx.get_executor(), asio::execution::outstanding_work.tracked);
        auto p = std::coroutine_handle<detail::main_promise>::from_promise(*mn.promise);
        asio::signal_set ss{ctx, SIGINT, SIGTERM};
        mn.promise->signal_set = &ss;
        struct work
        {
            asio::signal_set & ss;
            asio::cancellation_signal & signal;
            void operator()(asio::error_code ec, int sig) const
            {
                if (sig == SIGINT)
                    signal.emit(asio::cancellation_type::total);
                if (sig == SIGTERM)
                    signal.emit(asio::cancellation_type::terminal);
                if (!ec)
                    ss.async_wait(*this);
            }
        };

        ss.async_wait(work{ss, mn.promise->signal});
        asio::post(ctx.get_executor(), [p]{p.resume();});

        ctx.run();
        return res;
    }

    using executor_type = asio::io_context::executor_type;
    executor_type get_executor() const {return exec->context().get_executor();}

    using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using enable_awaitables<main_promise>::await_transform;
    using enable_deferred::await_transform;
    using enable_operations<>::await_transform;

    auto await_transform(this_coro::executor_t) const
    {
        struct exec_helper
        {
            executor_type value;

            constexpr static bool await_ready() noexcept
            {
                return true;
            }

            constexpr static void await_suspend(std::coroutine_handle<>) noexcept
            {
            }

            executor_type await_resume() const noexcept
            {
                return value;
            }
        };

        return exec_helper{get_executor()};
    }

  private:
    int * result;
    std::optional<typename asio::require_result<executor_type, asio::execution::outstanding_work_t ::tracked_t>::type> exec;
    asio::signal_set * signal_set;
    ::coro::main get_return_object()
    {
        return ::coro::main{this};
    }

};
}

}

namespace std
{

template<>
struct coroutine_traits<coro::main, int, char**>
{
    using promise_type = coro::detail::main_promise;
};

}


#endif //CORO_MAIN_HPP
