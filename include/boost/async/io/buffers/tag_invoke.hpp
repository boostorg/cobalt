//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_TAG_INVOKE_HPP
#define BOOST_BUFFERS_TAG_INVOKE_HPP

#include <boost/async/config.hpp>

namespace boost::async::io::buffers {

/** size tag for tag_invoke.
*/
struct size_tag {};

/** prefix tag for tag_invoke.
*/
struct prefix_tag {};

/** suffix tag for tag-invoke.
*/
struct suffix_tag {};

} // boost::buffers

#endif
