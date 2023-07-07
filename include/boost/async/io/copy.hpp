//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_COPY_HPP
#define BOOST_ASYNC_IO_COPY_HPP

#include <boost/async/io/stream.hpp>
#include <boost/async/promise.hpp>
#include <boost/async/io/buffers/dynamic_buffer_view.hpp>

namespace boost::async::io
{

BOOST_ASYNC_DECL promise<std::pair<transfer_result, transfer_result>>
copy(stream & source, stream & sink);

BOOST_ASYNC_DECL promise<std::pair<transfer_result, transfer_result>>
copy(stream & source, stream & sink, buffers::dynamic_buffer_view buffer, std::size_t chunk_size = 4096);



}

#endif //BOOST_ASYNC_IO_COPY_HPP
