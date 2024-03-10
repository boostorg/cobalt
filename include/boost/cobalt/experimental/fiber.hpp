// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_COBALT_FIBER_HPP
#define BOOST_COBALT_FIBER_HPP

#include <boost/context/fiber.hpp>

// this is all UB according to the standard. BUT it shouldn't be!

namespace boost::cobalt::experimental
{

namespace detail
{

struct fiber_promise
{

};

struct fiber_frame
{
  void (*resume_) (fiber_frame *) = +[](fiber_frame * ff) { ff->resume();};
  void (*destroy_)(fiber_frame *) = +[](fiber_frame * ff) { ff->destroy();};

  fiber_promise promise;

  void resume() {}
  void destroy() {}
};

}

}

#endif //BOOST_COBALT_FIBER_HPP
