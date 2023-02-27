// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_ASYNC_FOR_HPP
#define BOOST_ASYNC_ASYNC_FOR_HPP

#include <boost/preprocessor/cat.hpp>

#define BOOST_ASYNC_FOR_IMPL(Value, Expression, Id) \
for (auto Id = Expression; Id; )                    \
  if (Value = co_await Id; false) {} else

#define BOOST_ASYNC_FOR(Value, Expression) \
  BOOST_ASYNC_FOR_IMPL(Value, Expression, BOOST_PP_CAT(__boost_async_for_loop_value__, __LINE__))

#endif //BOOST_ASYNC_ASYNC_FOR_HPP
