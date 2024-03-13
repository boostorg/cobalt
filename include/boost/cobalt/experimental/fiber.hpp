//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_FIBER_HPP
#define BOOST_COBALT_EXPERIMENTAL_FIBER_HPP

#include <boost/context/fiber.hpp>
#include <coroutine>

// more UB and not useful.

namespace boost::cobalt::experimental
{

class fiber_frame
{
  void (*resume_) (fiber_frame *) = +[](fiber_frame * ff) { ff->resume();};
  void (*destroy_)(fiber_frame *) = +[](fiber_frame * ff) { ff->destroy();};

  boost::context::fiber fiber_;
 public:

        boost::context::fiber& fiber()       { return fiber_;}
  const boost::context::fiber& fiber() const { return std::move(fiber_);}

  fiber_frame(boost::context::fiber && fiber_ = {}) : fiber_(std::move(fiber_)) {}

  void resume()
  {
    resume_ = nullptr;
    fiber_ = std::move(fiber_).resume();
  }

  void destroy()
  {
    fiber_ = {};
  }
};

static_assert(std::is_standard_layout_v<fiber_frame>);

}

template<>
struct std::coroutine_handle<boost::context::fiber>
{
  constexpr operator coroutine_handle<>() const noexcept { return coroutine_handle<>::from_address(address()); }

  constexpr explicit operator bool() const noexcept { return true; }

  constexpr bool done() const noexcept { return false; }
  void operator()() const noexcept {}

  void resume() const noexcept {frame_->resume();}
  void destroy() const noexcept {frame_->destroy();}

  boost::context::fiber& promise() const noexcept { return frame_->fiber(); }

  constexpr void* address() const noexcept { return frame_; }

  coroutine_handle(boost::cobalt::experimental::fiber_frame & frame) : frame_(&frame) {}
 private:
  boost::cobalt::experimental::fiber_frame * frame_;
};

#endif //BOOST_COBALT_EXPERIMENTAL_FUTURE_HPP
