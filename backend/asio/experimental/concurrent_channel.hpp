//
// experimental/concurrent_channel.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP
#define ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../detail/config.hpp"
#include "../any_io_executor.hpp"
#include "../detail/type_traits.hpp"
#include "../execution/executor.hpp"
#include "../is_executor.hpp"
#include "basic_concurrent_channel.hpp"
#include "channel_traits.hpp"

#include "../detail/push_options.hpp"

namespace asio {
namespace experimental {
namespace detail {

template <typename ExecutorOrSignature, typename = void>
struct concurrent_channel_type
{
  template <typename... Signatures>
  struct inner
  {
    typedef basic_concurrent_channel<any_io_executor, channel_traits<>,
        ExecutorOrSignature, Signatures...> type;
  };
};

template <typename ExecutorOrSignature>
struct concurrent_channel_type<ExecutorOrSignature,
    enable_if_t<
      is_executor<ExecutorOrSignature>::value
        || execution::is_executor<ExecutorOrSignature>::value
    >>
{
  template <typename... Signatures>
  struct inner
  {
    typedef basic_concurrent_channel<ExecutorOrSignature,
        channel_traits<>, Signatures...> type;
  };
};

} // namespace detail

/// Template type alias for common use of channel.
#if defined(GENERATING_DOCUMENTATION)
template <typename ExecutorOrSignature, typename... Signatures>
using concurrent_channel = basic_concurrent_channel<
    specified_executor_or_any_io_executor, channel_traits<>, signatures...>;
#else // defined(GENERATING_DOCUMENTATION)
template <typename ExecutorOrSignature, typename... Signatures>
using concurrent_channel = typename detail::concurrent_channel_type<
    ExecutorOrSignature>::template inner<Signatures...>::type;
#endif // defined(GENERATING_DOCUMENTATION)

} // namespace experimental
} // namespace asio

#include "../detail/pop_options.hpp"

#endif // ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP
