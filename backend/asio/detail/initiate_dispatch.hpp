//
// detail/initiate_dispatch.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_INITIATE_DISPATCH_HPP
#define ASIO_DETAIL_INITIATE_DISPATCH_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include "../associated_allocator.hpp"
#include "../associated_executor.hpp"
#include "work_dispatcher.hpp"
#include "../execution/allocator.hpp"
#include "../execution/blocking.hpp"
#include "../prefer.hpp"

#include "push_options.hpp"

namespace asio {
namespace detail {

class initiate_dispatch
{
public:
  template <typename CompletionHandler>
  void operator()(CompletionHandler&& handler,
      enable_if_t<
        execution::is_executor<
          associated_executor_t<decay_t<CompletionHandler>>
        >::value
      >* = 0) const
  {
    associated_executor_t<decay_t<CompletionHandler>> ex(
        (get_associated_executor)(handler));

    associated_allocator_t<decay_t<CompletionHandler>> alloc(
        (get_associated_allocator)(handler));

    asio::prefer(ex, execution::allocator(alloc)).execute(
        asio::detail::bind_handler(
          static_cast<CompletionHandler&&>(handler)));
  }

  template <typename CompletionHandler>
  void operator()(CompletionHandler&& handler,
      enable_if_t<
        !execution::is_executor<
          associated_executor_t<decay_t<CompletionHandler>>
        >::value
      >* = 0) const
  {
    associated_executor_t<decay_t<CompletionHandler>> ex(
        (get_associated_executor)(handler));

    associated_allocator_t<decay_t<CompletionHandler>> alloc(
        (get_associated_allocator)(handler));

    ex.dispatch(asio::detail::bind_handler(
          static_cast<CompletionHandler&&>(handler)), alloc);
  }
};

template <typename Executor>
class initiate_dispatch_with_executor
{
public:
  typedef Executor executor_type;

  explicit initiate_dispatch_with_executor(const Executor& ex)
    : ex_(ex)
  {
  }

  executor_type get_executor() const noexcept
  {
    return ex_;
  }

  template <typename CompletionHandler>
  void operator()(CompletionHandler&& handler,
      enable_if_t<
        execution::is_executor<
          conditional_t<true, executor_type, CompletionHandler>
        >::value
      >* = 0,
      enable_if_t<
        !detail::is_work_dispatcher_required<
          decay_t<CompletionHandler>,
          Executor
        >::value
      >* = 0) const
  {
    associated_allocator_t<decay_t<CompletionHandler>> alloc(
        (get_associated_allocator)(handler));

    asio::prefer(ex_, execution::allocator(alloc)).execute(
        asio::detail::bind_handler(
          static_cast<CompletionHandler&&>(handler)));
  }

  template <typename CompletionHandler>
  void operator()(CompletionHandler&& handler,
      enable_if_t<
        execution::is_executor<
          conditional_t<true, executor_type, CompletionHandler>
        >::value
      >* = 0,
      enable_if_t<
        detail::is_work_dispatcher_required<
          decay_t<CompletionHandler>,
          Executor
        >::value
      >* = 0) const
  {
    typedef decay_t<CompletionHandler> handler_t;

    typedef associated_executor_t<handler_t, Executor> handler_ex_t;
    handler_ex_t handler_ex((get_associated_executor)(handler, ex_));

    associated_allocator_t<handler_t> alloc(
        (get_associated_allocator)(handler));

    asio::prefer(ex_, execution::allocator(alloc)).execute(
        detail::work_dispatcher<handler_t, handler_ex_t>(
          static_cast<CompletionHandler&&>(handler), handler_ex));
  }

  template <typename CompletionHandler>
  void operator()(CompletionHandler&& handler,
      enable_if_t<
        !execution::is_executor<
          conditional_t<true, executor_type, CompletionHandler>
        >::value
      >* = 0,
      enable_if_t<
        !detail::is_work_dispatcher_required<
          decay_t<CompletionHandler>,
          Executor
        >::value
      >* = 0) const
  {
    associated_allocator_t<decay_t<CompletionHandler>> alloc(
        (get_associated_allocator)(handler));

    ex_.dispatch(asio::detail::bind_handler(
          static_cast<CompletionHandler&&>(handler)), alloc);
  }

  template <typename CompletionHandler>
  void operator()(CompletionHandler&& handler,
      enable_if_t<
        !execution::is_executor<
          conditional_t<true, executor_type, CompletionHandler>
        >::value
      >* = 0,
      enable_if_t<
        detail::is_work_dispatcher_required<
          decay_t<CompletionHandler>,
          Executor
        >::value
      >* = 0) const
  {
    typedef decay_t<CompletionHandler> handler_t;

    typedef associated_executor_t<handler_t, Executor> handler_ex_t;
    handler_ex_t handler_ex((get_associated_executor)(handler, ex_));

    associated_allocator_t<handler_t> alloc(
        (get_associated_allocator)(handler));

    ex_.dispatch(detail::work_dispatcher<handler_t, handler_ex_t>(
          static_cast<CompletionHandler&&>(handler), handler_ex), alloc);
  }

private:
  Executor ex_;
};

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#endif // ASIO_DETAIL_INITIATE_DISPATCH_HPP
