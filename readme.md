# boost.promise

*proof of concept*.

This library provides a set of easy to use, efficient & fast to compile wrappers around asio & beast (and possible future boost libraries).

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
    async::io::steady_timer tim{co_await async::this_coro::executor};
    dt.expires_after(std::chrono::milliseconds(100));
    co_await tim.wait();
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
