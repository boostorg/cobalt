//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_CHANNEL_HPP
#define BOOST_ASYNC_CHANNEL_HPP

#include <boost/async/this_thread.hpp>
#include <boost/async/detail/util.hpp>

#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/config.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/variant2/variant.hpp>

#include <optional>

namespace boost::async
{

template<typename T>
struct channel_reader;

// tag::outline[]
template<typename T>
struct channel
{
  // create a channel with a buffer limit, executor & resource.
  channel(std::size_t limit = 0u,
          executor executor = this_thread::get_executor(),
          pmr::memory_resource * resource = this_thread::get_default_resource());

  // movable
  channel(channel && ) = default;

  using executor_type = executor;
  const executor_type & get_executor();

  // Closes the channel
  ~channel();
  bool is_open() const;
  // close the operation, will cancel all pending ops, too
  void close();

  // end::outline[]
 private:
  boost::circular_buffer<T, pmr::polymorphic_allocator<T>> buffer_;
  executor_type executor_;
  bool is_closed_{false};

  struct read_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false;
    std::optional<T> direct;
    asio::cancellation_slot cancel_slot;
    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};
    void (*begin_transaction)(void*);

    void transactional_unlink()
    {
      if (begin_transaction)
          begin_transaction(awaited_from.get());
      this->unlink();
    }

    struct cancel_impl;
    bool await_ready() { return !chn->buffer_.empty(); }
    template<typename Promise>
    BOOST_NOINLINE 
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);
    T await_resume();
    explicit operator bool() const {return chn && chn->is_open();}
  };

  struct write_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    variant2::variant<T*, const T*> ref;
    boost::source_location loc;
    bool cancelled = false, direct = false;
    asio::cancellation_slot cancel_slot;

    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};
    void (*begin_transaction)(void*);

    void transactional_unlink()
    {
      if (begin_transaction)
          begin_transaction(awaited_from.get());
      this->unlink();
    }

    struct cancel_impl;

    bool await_ready() { return !chn->buffer_.full(); }
    template<typename Promise>
    BOOST_NOINLINE 
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);
    void await_resume();
    explicit operator bool() const {return chn && chn->is_open();}
  };

  boost::intrusive::list<read_op,  intrusive::constant_time_size<false> > read_queue_;
  boost::intrusive::list<write_op, intrusive::constant_time_size<false> > write_queue_;
 public:
  read_op   read(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  read_op{{}, this, loc}; }
  write_op write(const T  && value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  write_op write(const T  &  value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  write_op write(      T &&  value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  write_op write(      T  &  value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  /*
  // tag::outline[]
  // an awaitable that yields T
  using __read_op__ = __unspecified__;

  // an awaitable that yields void
  using __write_op__ = __unspecified__;

  // read a value to a channel
  __read_op__  read();

  // write a value to the channel
  __write_op__ write(const T  && value);
  __write_op__ write(const T  &  value);
  __write_op__ write(      T &&  value);
  __write_op__ write(      T  &  value);

  // write a value to the channel if T is void
  __write_op__ write();  // end::outline[]
   */
  // tag::outline[]

};
// end::outline[]

template<>
struct channel<void>
{
  channel(std::size_t limit = 0u,
          executor executor = this_thread::get_executor())
        : limit_(limit), executor_(executor) {}
  channel(channel && ) = default;

  using executor_type = executor;
  const executor_type & get_executor() {return executor_;}

  BOOST_ASYNC_DECL ~channel();

  bool is_open() const {return !is_closed_;}
  BOOST_ASYNC_DECL void close();

 private:
  std::size_t limit_;
  std::size_t n_{0u};
  executor_type executor_;
  bool is_closed_{false};

  struct read_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false, direct = false;
    asio::cancellation_slot cancel_slot;
    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};
    void (*begin_transaction)(void*);

    void transactional_unlink()
    {
      if (begin_transaction)
          begin_transaction(awaited_from.get());
      this->unlink();
    }

    struct cancel_impl;
    bool await_ready() { return (chn->n_ > 0); }
    template<typename Promise>
    BOOST_NOINLINE 
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);
    BOOST_ASYNC_DECL void await_resume();
    explicit operator bool() const {return chn && chn->is_open();}
  };

  struct write_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false, direct = false;
    asio::cancellation_slot cancel_slot;
    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};
    void (*begin_transaction)(void*);

    void transactional_unlink()
    {
      if (begin_transaction)
          begin_transaction(awaited_from.get());
      this->unlink();
    }

    struct cancel_impl;
    bool await_ready()
    {
      return chn->n_ < chn->limit_;
    }

    template<typename Promise>
    BOOST_NOINLINE 
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);

    BOOST_ASYNC_DECL void await_resume();
    explicit operator bool() const {return chn && chn->is_open();}
  };

  boost::intrusive::list<read_op,  intrusive::constant_time_size<false> > read_queue_;
  boost::intrusive::list<write_op, intrusive::constant_time_size<false> > write_queue_;
 public:
  read_op   read(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  read_op{{}, this, loc}; }
  write_op write(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return write_op{{}, this, loc}; }
};

template<typename T>
struct channel_reader
{
  channel_reader(channel<T> & chan,
                 const boost::source_location & loc = BOOST_CURRENT_LOCATION) : chan_(chan), loc_(loc) {}

  auto operator co_await () -> typename channel<T>::read_op
  {
    return chan_->read(loc_);
  }

  explicit operator bool () const {return chan_ && chan_->is_open();}

 private:
  channel<T> * chan_;
  boost::source_location loc_;
};

}

#include <boost/async/impl/channel.hpp>

#endif //BOOST_ASYNC_CHANNEL_HPP
