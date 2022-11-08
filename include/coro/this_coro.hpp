// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_THIS_CORO_HPP
#define CORO_THIS_CORO_HPP

#include <asio/associated_allocator.hpp>
#include <asio/associated_cancellation_slot.hpp>
#include <asio/associated_executor.hpp>
#include <asio/dispatch.hpp>
#include <asio/this_coro.hpp>
#include <asio/cancellation_state.hpp>

#include <coroutine>
#include <optional>
#include <tuple>

namespace coro
{

template<typename ... Args>
struct completion_handler;

namespace this_coro
{

using namespace asio::this_coro;

}

template<typename Executor>
struct promise_executor_base
{
    using executor_type = std::remove_volatile_t<Executor>;
    executor_type get_executor() const {return executor_;}

    promise_executor_base(executor_type exec) : executor_{exec} {}

    auto await_transform(this_coro::executor_t) const
    {
        struct exec_helper
        {
            const executor_type& value;

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

        return exec_helper{executor_};
    }
    executor_type exchange_executor(executor_type exec)
    {
        return std::exchange(executor_, exec);
    }

  private:
    executor_type executor_;
};

template<typename CancellationSlot = asio::cancellation_slot,
         typename DefaultFilter = asio::enable_terminal_cancellation>
struct promise_cancellation_base
{
    using cancellation_slot_type = asio::cancellation_slot;
    cancellation_slot_type get_cancellation_slot() const {return state_.slot();}

    template<typename InitialFilter = asio::enable_terminal_cancellation>
    promise_cancellation_base(CancellationSlot slot = {}, InitialFilter filter = {})
        : source_(slot), state_{source_, filter} {}


    // This await transformation resets the associated cancellation state.
    auto await_transform(asio::this_coro::cancellation_state_t) noexcept
    {
        struct result
        {
            asio::cancellation_state &state;

            bool await_ready() const noexcept
            {
                return true;
            }

            void await_suspend(std::coroutine_handle<void>) noexcept
            {
            }

            auto await_resume() const
            {
                return state;
            }
        };
        return result{state_};
    }

    // This await transformation resets the associated cancellation state.
    auto await_transform(asio::this_coro::reset_cancellation_state_0_t) noexcept
    {
        struct result
        {
            asio::cancellation_state &state;
            CancellationSlot &source;

            bool await_ready() const noexcept
            {
                return true;
            }

            void await_suspend(std::coroutine_handle<void>) noexcept
            {
            }

            auto await_resume() const
            {
                state = asio::cancellation_state(source, DefaultFilter());
            }
        };
        return result{state_, source_};
    }

    // This await transformation resets the associated cancellation state.
    template <typename Filter>
    auto await_transform(
            asio::this_coro::reset_cancellation_state_1_t<Filter> reset) noexcept
    {
        struct result
        {
            asio::cancellation_state & state;
            Filter filter_;
            CancellationSlot &source;

            bool await_ready() const noexcept
            {
                return true;
            }

            void await_suspend(std::coroutine_handle<void>) noexcept
            {
            }

            auto await_resume()
            {
                state = asio::cancellation_state(
                        source,
                        ASIO_MOVE_CAST(Filter)(filter_));
            }
        };

        return result{state_, ASIO_MOVE_CAST(Filter)(reset.filter), source_};
    }

    // This await transformation resets the associated cancellation state.
    template <typename InFilter, typename OutFilter>
    auto await_transform(
            asio::this_coro::reset_cancellation_state_2_t<InFilter, OutFilter> reset)
    noexcept
    {
        struct result
        {
            asio::cancellation_state & state;
            InFilter in_filter_;
            OutFilter out_filter_;
            CancellationSlot &source;


            bool await_ready() const noexcept
            {
                return true;
            }

            void await_suspend(std::coroutine_handle<void>) noexcept
            {
            }

            auto await_resume()
            {
                state = asio::cancellation_state(
                        source,
                        ASIO_MOVE_CAST(InFilter)(in_filter_),
                        ASIO_MOVE_CAST(OutFilter)(out_filter_));
            }
        };

        return result{state_,
                      ASIO_MOVE_CAST(InFilter)(reset.in_filter),
                      ASIO_MOVE_CAST(OutFilter)(reset.out_filter),
                      source_};
    }
    const asio::cancellation_state & cancellation_state() const {return state_;}
    asio::cancellation_state & cancellation_state() {return state_;}
    asio::cancellation_type cancelled() const {return state_.cancelled();}

    cancellation_slot_type get_cancellation_slot() {return state_.slot();}

    void reset_cancellation_source(CancellationSlot source = CancellationSlot())
    {
        state_ = {source_ = source, DefaultFilter() };
    }

          CancellationSlot & source() {return source_;}
    const CancellationSlot & source() const {return source_;}
  private:
    CancellationSlot source_;
    asio::cancellation_state state_{source_, DefaultFilter() };

};

struct promise_throw_if_cancelled_base
{
    promise_throw_if_cancelled_base(bool throw_if_cancelled = false) {}

    // This await transformation determines whether cancellation is propagated as
    // an exception.
    auto await_transform(this_coro::throw_if_cancelled_0_t)
    noexcept
    {
        struct result
        {
            bool value_;

            bool await_ready() const noexcept
            {
                return true;
            }

            void await_suspend(std::coroutine_handle<void>) noexcept
            {
            }

            auto await_resume()
            {
                return value_;
            }
        };

        return result{throw_if_cancelled_};
    }

    // This await transformation sets whether cancellation is propagated as an
    // exception.
    auto await_transform(this_coro::throw_if_cancelled_1_t throw_if_cancelled)
    noexcept
    {
        struct result
        {
            promise_throw_if_cancelled_base* this_;
            bool value_;

            bool await_ready() const noexcept
            {
                return true;
            }

            void await_suspend(std::coroutine_handle<void>) noexcept
            {
            }

            auto await_resume()
            {
                this_->throw_if_cancelled_ = value_;
            }
        };

        return result{this, throw_if_cancelled.value};
    }
    bool throw_if_cancelled() const {return throw_if_cancelled_;}
  protected:
    bool throw_if_cancelled_{true};
};

}

#endif //CORO_THIS_CORO_HPP
