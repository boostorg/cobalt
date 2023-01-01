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
    // warns
    my_task();
    // ok
    +my_task();
    co_return 0;
}

```

An async:promise can also be used with `spawn` to turn it into an asio operation.


## Wrappers

The wrappers around the io-objects follow this pattern:

no sync-io, the async_ prefix is dropped. All those function return ops that can be awaited. Furthmore, that is an overload taking a reference to an `error_code`, that avoids throwing exceptions.

They additionally implement abstract interfaces such as `read_stream`:

```cpp
struct execution_context
{
  /// The type of the executor associated with the object.
  using executor_type = asio::io_context::executor_type;
  /// Get the executor associated with the object.
  virtual executor_type get_executor() = 0;
  virtual ~execution_context() = default;
};


struct read_stream : virtual execution_context
{

// the three following functions are for async_read_until
  virtual void async_read_some(asio::mutable_buffer buffer,                     read_handler h) = 0;
  virtual void async_read_some(static_buffer_base::mutable_buffers_type buffer, read_handler h) = 0;
  virtual void async_read_some(multi_buffer::mutable_buffers_type buffer,       read_handler h) = 0;

  [[nodiscard]] auto read_some(asio::mutable_buffer buffer);
  [[nodiscard]] auto read_some(asio::mutable_buffer buffer, system::error_code & ec);
};
```

This allows the following usage:

```cpp
// can be any readable stream, a pipe, a tcp socket, a ssl<tcp-socket> etc.
async::promise<void> read_my_thingy(async_read_stream & rs);

async::main co_main(int argc, char ** argv)
{
    // redable & writable pipes!
    auto [rp, wp] = async::connect_pipe(co_await async::this_coro::executor);

    read_my_thingy(rp);

    co_return 0;
}
```

Additionally, the algorithms of asio are supported for a fixed set of buffers:


```cpp
auto read_until(concepts::read_stream & stream, mutable_buffer              buffer);
auto read_until(concepts::read_stream & stream, flat_static_buffer_base    &buffer);
auto read_until(concepts::read_stream & stream, static_buffer_base         &buffer);
auto read_until(concepts::read_stream & stream, flat_buffer                &buffer);
auto read_until(concepts::read_stream & stream, multi_buffer               &buffer);
auto read_until(concepts::read_stream & stream, std::string                &buffer);
auto read_until(concepts::read_stream & stream, std::vector<unsigned char> &buffer);
auto read_until(concepts::read_stream & stream, streambuf                  &buffer);
```

With equivalent overloads matching all overloads in asio.

```cpp
async::main co_main(int argc, char ** argv)
{
    // redable & writable pipes!
    auto [rp, wp] = async::io::connect_pipe(co_await async::this_coro::executor);

    std::string line;
    async::io::read_until(rp, line, '\n');

    co_return 0;
}
```
