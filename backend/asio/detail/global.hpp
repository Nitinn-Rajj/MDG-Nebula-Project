//
// detail/global.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_GLOBAL_HPP
#define ASIO_DETAIL_GLOBAL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"

#if !defined(ASIO_HAS_THREADS)
# include "null_global.hpp"
#elif defined(ASIO_WINDOWS)
# include "win_global.hpp"
#elif defined(ASIO_HAS_PTHREADS)
# include "posix_global.hpp"
#else
# include "std_global.hpp"
#endif

namespace asio {
namespace detail {

template <typename T>
inline T& global()
{
#if !defined(ASIO_HAS_THREADS)
  return null_global<T>();
#elif defined(ASIO_WINDOWS)
  return win_global<T>();
#elif defined(ASIO_HAS_PTHREADS)
  return posix_global<T>();
#else
  return std_global<T>();
#endif
}

} // namespace detail
} // namespace asio

#endif // ASIO_DETAIL_GLOBAL_HPP
