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

#include <boost/intrusive/list.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/variant2/variant.hpp>

#include <optional>

namespace boost::async
{

template<typename T>
struct channel_reader;

template<typename T>
struct channel
{
  channel(std::size_t limit = 0u,
          asio::io_context::executor_type executor = this_thread::get_executor(),
          container::pmr::memory_resource * resource = this_thread::get_default_resource())
            : buffer_(limit, resource), executor_(executor) {}
  channel(channel && ) = default;

  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() {return executor_;}

  ~channel();
  bool is_open() const {return !is_closed_;}
  void close();

 private:
  boost::circular_buffer<T, container::pmr::polymorphic_allocator<T>> buffer_;
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
    void (*reserve_completion)(void*);

    void unlink()
    {
      intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >::unlink();
      if (reserve_completion)
          reserve_completion(awaited_from.get());
    }

    struct cancel_impl;
    bool await_ready() { return !chn->buffer_.empty(); }
    template<typename Promise>
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
    void (*reserve_completion)(void*);

    void unlink()
    {
      intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >::unlink();
      if (reserve_completion)
        reserve_completion(awaited_from.get());
    }

    struct cancel_impl;

    bool await_ready() { return !chn->buffer_.full(); }
    template<typename Promise>
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
};

template<>
struct channel<void>
{
  channel(std::size_t limit = 0u,
          asio::io_context::executor_type executor = this_thread::get_executor())
        : limit_(limit), executor_(executor) {}
  channel(channel && ) = default;

  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() {return executor_;}

  ~channel();

  bool is_open() const {return !is_closed_;}
  void close();

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
    void (*reserve_completion)(void*);

    void unlink()
    {
      intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >::unlink();
      if (reserve_completion)
        reserve_completion(awaited_from.get());
    }

    struct cancel_impl;
    bool await_ready() { return (chn->n_ > 0); }
    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);
    void await_resume();
    explicit operator bool() const {return chn && chn->is_open();}
  };

  struct write_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false, direct = false;
    asio::cancellation_slot cancel_slot;
    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};
    void (*reserve_completion)(void*);

    void unlink()
    {
      intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >::unlink();
      if (reserve_completion)
        reserve_completion(awaited_from.get());
    }

    struct cancel_impl;
    bool await_ready() { return chn->n_ < chn->limit_; }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);

    void await_resume();
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
