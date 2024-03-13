//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_FIBER_HPP
#define BOOST_COBALT_EXPERIMENTAL_FIBER_HPP

#include <boost/context/continuation.hpp>
#include <coroutine>

// more UB and not useful.

namespace boost::cobalt::experimental
{

class continuation_frame
{
  void (*resume_) (continuation_frame *) = +[](continuation_frame * ff) { ff->resume();};
  void (*destroy_)(continuation_frame *) = +[](continuation_frame * ff) { ff->destroy();};

  boost::context::continuation continuation_;
 public:

        boost::context::continuation& continuation()       { return continuation_;}
  const boost::context::continuation& continuation() const { return std::move(continuation_);}

  continuation_frame(boost::context::continuation && continuation_ = {}) : continuation_(std::move(continuation_)) {}

  void resume()
  {
    resume_ = nullptr;
    continuation_ = std::move(continuation_).resume();
  }

  void destroy()
  {
    continuation_ = {};
  }
};

static_assert(std::is_standard_layout_v<continuation_frame>);

}

template<>
struct std::coroutine_handle<boost::context::continuation>
{
  constexpr operator coroutine_handle<>() const noexcept { return coroutine_handle<>::from_address(address()); }

  constexpr explicit operator bool() const noexcept { return true; }

  constexpr bool done() const noexcept { return false; }
  void operator()() const noexcept {}

  void resume() const noexcept {frame_->resume();}
  void destroy() const noexcept {frame_->destroy();}

  boost::context::continuation& promise() const noexcept { return frame_->continuation(); }

  constexpr void* address() const noexcept { return frame_; }

  coroutine_handle(boost::cobalt::experimental::continuation_frame & frame) : frame_(&frame) {}
 private:
  boost::cobalt::experimental::continuation_frame * frame_;
};

#endif //BOOST_COBALT_EXPERIMENTAL_FUTURE_HPP
