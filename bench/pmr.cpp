// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/async.hpp>
#include <boost/asio.hpp>


using namespace boost;
constexpr std::size_t n = 50'000'000ull;

// below is the same as promise<void> just without the pmr stuff
namespace detail
{
struct async_no_pmr_promise;
}

struct [[nodiscard]] no_pmr_promise
{
  no_pmr_promise(no_pmr_promise &&lhs) noexcept;
  no_pmr_promise& operator=(no_pmr_promise && lhs) noexcept;

  // enable `co_await`. <1>
  auto operator co_await () {return receiver_.get_awaitable();}


  void cancel(asio::cancellation_type ct = asio::cancellation_type::all);

  using promise_type = ::detail::async_no_pmr_promise;
  no_pmr_promise(const no_pmr_promise &) = delete;
  no_pmr_promise& operator=(const no_pmr_promise &) = delete;

  ~no_pmr_promise()
  {
    if (attached_)
      cancel();
  }
 private:
  friend struct ::detail::async_no_pmr_promise;

  no_pmr_promise(::detail::async_no_pmr_promise * promise);
  bool attached_;

  async::detail::promise_receiver<void> receiver_;
};


namespace detail
{

struct async_no_pmr_promise
    : async::promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
      async::promise_throw_if_cancelled_base,
      async::enable_awaitables<async_no_pmr_promise>,
      async::enable_await_executor<async_no_pmr_promise>,
      async::detail::async_promise_result<void>
{
  using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
  using promise_throw_if_cancelled_base::await_transform;
  using enable_awaitables<async_no_pmr_promise>::await_transform;
  using enable_await_executor<async_no_pmr_promise>::await_transform;

  [[nodiscard]] no_pmr_promise get_return_object()
  {
    return no_pmr_promise{this};
  }

  mutable asio::cancellation_signal signal;

  using executor_type = async::executor;
  executor_type exec;
  const executor_type & get_executor() const {return exec;}

  template<typename ... Args>
  async_no_pmr_promise(Args & ...args)
      : exec{async::detail::get_executor_from_args(args...)}
  {
    this->reset_cancellation_source(signal.slot());
  }

  std::suspend_never initial_suspend()        {return {};}
  auto final_suspend() noexcept
  {
    struct final_awaitable
    {
      async_no_pmr_promise * promise;
      bool await_ready() const noexcept
      {
        return promise->receiver && promise->receiver->awaited_from.get() == nullptr;
      }

      std::coroutine_handle<void> await_suspend(std::coroutine_handle<async_no_pmr_promise> h) noexcept
      {
        std::coroutine_handle<void> res = std::noop_coroutine();
        if (promise->receiver && promise->receiver->awaited_from.get() != nullptr)
          res = std::coroutine_handle<void>::from_address(promise->receiver->awaited_from.release());


        if (auto &rec = h.promise().receiver; rec != nullptr)
        {
          if (!rec->done && !rec->exception)
            rec->exception = async::detail::completed_unexpected();
          rec->set_done();
          rec->awaited_from.reset(nullptr);
          rec = nullptr;
        }
        async::detail::self_destroy(h);
        return res;
      }

      void await_resume() noexcept
      {
      }
    };

    return final_awaitable{this};
  }

  void unhandled_exception()
  {
    if (this->receiver)
      this->receiver->unhandled_exception();
    else
      throw ;
  }

  ~async_no_pmr_promise()
  {
    if (this->receiver)
    {
      if (!this->receiver->done && !this->receiver->exception)
        this->receiver->exception = async::detail::completed_unexpected();
      this->receiver->set_done();
      this->receiver->awaited_from.reset(nullptr);
    }

  }

  friend struct async_initiate;
};

}

no_pmr_promise::no_pmr_promise(::detail::async_no_pmr_promise * promise)
    : receiver_(promise->receiver, promise->signal), attached_{true}
{
}

void no_pmr_promise::cancel(asio::cancellation_type ct)
{
  if (!receiver_.done && receiver_.reference == &receiver_)
    receiver_.cancel_signal.emit(ct);
}

volatile bool prevent_inlining = true;

struct prevent_inlining_t
{
  bool await_ready() volatile { return prevent_inlining;}
  void await_suspend(std::coroutine_handle<void>) {}
  void await_resume() {}
};

async::promise<void> atest()
{
  co_await prevent_inlining_t{};
  co_return ;
}

async::promise<void> atest(std::allocator_arg_t, async::pmr::polymorphic_allocator<void> )
{
  co_await prevent_inlining_t{};
  co_return ;
}

no_pmr_promise btest()
{
  co_await prevent_inlining_t{};
  co_return ;
}


async::main co_main(int argc, char * argv[])
{
  {
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0u; i < n; i++)
      co_await btest();
    auto end = std::chrono::steady_clock::now();
    printf("without pmr: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }
  {
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0u; i < n; i++)
       co_await atest();

    auto end = std::chrono::steady_clock::now();
    printf("with pmr   : %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
  }

  {
    auto res = async::this_thread::set_default_resource(async::pmr::get_default_resource());
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0u; i < n; i++)
      co_await atest();
    auto end = std::chrono::steady_clock::now();
    printf("with unset pmr: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    async::this_thread::set_default_resource(res);
  }

  {
    auto res = async::this_thread::set_default_resource(async::pmr::get_default_resource());
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0u; i < n; i++)
    {
      char buf[512];
      async::pmr::monotonic_buffer_resource res{&buf, 512};
      co_await atest(std::allocator_arg, async::pmr::polymorphic_allocator<void>(&res));
    }


    auto end = std::chrono::steady_clock::now();
    printf("with monotonic: %ld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    async::this_thread::set_default_resource(res);
  }

 co_return 0;
}