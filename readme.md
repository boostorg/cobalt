# boost.promise

*first draft*.

This library provides a set of easy to use coroutine primitives & utilities.

The assumptions are:

 - `io_context` is the execution_context of choice.
 - Any `asio::io_context` is single threaded
 - Eager execution is the way to go
 - A thread created with promise is only using promise stuff.

## Entry points

```cpp
// a single threaded main running on an io_context
async::main co_main(int argc, char ** argv)
{
    // wrapper around asio::steady_timer
    async::io::steady_timer tim{co_await async::this_coro::executor};
    dt.expires_after(std::chrono::milliseconds(100));

    // forward to asio::steady_timer::async_wait
    co_await tim.wait();

    co_return 0;
}
```

That is, `main` runs on a single threaded `io_context`, you can however add more threads. Threads can be cancelled or stopped (as in calling io_context::stop) on destruction.

It also hooks up signals, so that things like `Ctrl+C` get forwarded as cancellations automatically

```cpp
async::thread example_thread(int ms)
{
    async::io::steady_timer tim{co_await async::this_coro::executor};
    dt.expires_after(std::chrono::milliseconds(100));
    co_await tim.wait();
}
```

This thread can be used from anywhere, not only from an async::main:

```cpp
int main(int argc, char ** argv)
{
    auto tt = example_thread(ms);
    tt.join();
    return 0;
}
```

## Async promise

The core primitive for creating your own functions is `async::promise<T>`.
It is eager, i.e. it starts execution immediately, before you `co_await`.

```cpp
async::promise<void> test()
{
    printf("test-1\n");
    asio::steady_timer tim{co_await async::this_coro::executor};
    dt.expires_after(std::chrono::milliseconds(100));
    co_await tim.async_wait(async::use_op);
    printf("test-2\n");
}

async::main co_main(int argc, char ** argv)
{
    printf("main-1\n");
    auto tt = test();
    printf("main-2\n");
    co_await tt;
    printf("main-3\n");
    return 0;
}
```

The output of the above will be:

```cpp
main-1
test-1
main-2
test-2
main-3
```

Unlike ops, returned by .wait, the promise can be disregarded; disregarding the promise does not cancel it, but rather detaches is. This makes it easy to 
spin up multiple tasks to run in parallel. In order to avoid accidental detaching the promise type uses `nodiscard` unless one uses `+` to detach it:

```cpp
async::promise<void> my_task();

async::main co_main()
{
    // warns & cancels the task
    my_task();
    // ok
    +my_task();
    co_return 0;
}
```

An async:promise can also be used with `spawn` to turn it into an asio operation.

## Generator

A generator is a coroutine that produces a series of values instead of one, but otherwise similar to `promise`.

```cpp
async::generator<int> test()
{
  printf("test-1\n");
  co_yield 1;
  printf("test-2\n");
  co_yield 2;
  printf("test-3\n");
  co_return 3;
}

async::main co_main(int argc, char ** argv)
{
    printf("main-1\n");
    auto tt = test();
    printf("main-2\n");
    i = co_await tt; // 1
    printf("main-3: %d\n", i);
    i = co_await tt; // 2
    printf("main-4: %d\n", i);
    i = co_await tt; // 3
    printf("main-5: %d\n", i);
    co_return 0;
}
```

```
main-1
test-1
main-2
main-3: 1
test-2
main-4: 2
test-3
main-5: 3
```

## Channels

Channels are modeled on golang; they are different from boost.asio channels in that they don't go through the executor.
Instead they directly context switch when possible.

```cpp
async::promise<void> test(async::channel<void> & chan)
{
  printf("Reader 1: %d\n", co_await chan.read());
  printf("Reader 2: %d\n", co_await chan.read());
  printf("Reader 3: %d\n", co_await chan.read());
}

async::main co_main(int argc, char ** argv)
{
  async::channel<int> chan{0u /* buffer size */};
  
  auto p = test(chan);
  
  printf("Writer 1\n");
  co_await chan.write(10);
  printf("Writer 2\n");
  co_await chan.write(11);
  printf("Writer 3\n");
  co_await chan.write(12);
  printf("Writer 4\n");
  
  co_await p;
  co_return 0u;
}
```

````
Writer-1
Reader-1: 10
Writer-2
Reader-1: 11
Writer-3
Reader-1: 12
Writer-4
````

## Ops

To make writing asio operations that have an early completion easier, async has an op-helper:

```cpp
template<typename Timer>
struct wait_op : async::enable_op<test_wait_op_2<Timer>> // enable_op is to use ADL
{
  Timer & tim;

  wait_op(Timer & tim) : tim(tim) {}
  
  // this gets used to determine if it needs to suspend for the op
  void ready(async::handler<system::error_code> h)
  {
    if (tim.expiry() < Timer::clock_type::now())
      h(system::error_code(asio::error::operation_aborted));
  }
  
  // this gets used to initiate the op if ti needs to suspend
  void initiate(async::completion_handler<system::error_code> complete)
  {
    tim.async_wait(std::move(complete));
  }
};

async::main co_main(int argc, char ** argv)
{
  async::steady_timer tim{co_await async::this_coro::executor}; // already expired
  co_await wait_op(tim); // will not suspend, since it's ready
}

```


## select

Select let's you await multiple awaitables at once. 

```cpp
async::promise<void> delay(int ms)
{
    asio::steady_timer tim{co_await async::this_coro::executor};
    dt.expires_after(std::chrono::milliseconds(ms));
    co_await tim.async_wait(async::use_op);
}

async::main co_main(int argc, char ** argv)
{
  auto res = co_await select(delay(100), delay(50));
  asert(res == 1); // delay(50) completes earlier, delay(100) is not cancelled  
  co_return 0u;
}
```

`async` has a special cancellation_type, `interrupt_await`, that can stop the select without cancelling. 
It is supported by `channel`, `promise` & `generator`. 

## 
