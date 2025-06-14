//
// detail/resolver_service_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP
#define ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include "../error.hpp"
#include "../execution_context.hpp"
#include "mutex.hpp"
#include "noncopyable.hpp"
#include "resolve_op.hpp"
#include "socket_ops.hpp"
#include "socket_types.hpp"
#include "scoped_ptr.hpp"
#include "thread.hpp"

#if defined(ASIO_HAS_IOCP)
# include "win_iocp_io_context.hpp"
#else // defined(ASIO_HAS_IOCP)
# include "scheduler.hpp"
#endif // defined(ASIO_HAS_IOCP)

#include "push_options.hpp"

namespace asio {
namespace detail {

class resolver_service_base
{
public:
  // The implementation type of the resolver. A cancellation token is used to
  // indicate to the background thread that the operation has been cancelled.
  typedef socket_ops::shared_cancel_token_type implementation_type;

  // Constructor.
  ASIO_DECL resolver_service_base(execution_context& context);

  // Destructor.
  ASIO_DECL ~resolver_service_base();

  // Destroy all user-defined handler objects owned by the service.
  ASIO_DECL void base_shutdown();

  // Perform any fork-related housekeeping.
  ASIO_DECL void base_notify_fork(
      execution_context::fork_event fork_ev);

  // Construct a new resolver implementation.
  ASIO_DECL void construct(implementation_type& impl);

  // Destroy a resolver implementation.
  ASIO_DECL void destroy(implementation_type&);

  // Move-construct a new resolver implementation.
  ASIO_DECL void move_construct(implementation_type& impl,
      implementation_type& other_impl);

  // Move-assign from another resolver implementation.
  ASIO_DECL void move_assign(implementation_type& impl,
      resolver_service_base& other_service,
      implementation_type& other_impl);

  // Move-construct a new timer implementation.
  void converting_move_construct(implementation_type& impl,
      resolver_service_base&, implementation_type& other_impl)
  {
    move_construct(impl, other_impl);
  }

  // Move-assign from another timer implementation.
  void converting_move_assign(implementation_type& impl,
      resolver_service_base& other_service,
      implementation_type& other_impl)
  {
    move_assign(impl, other_service, other_impl);
  }

  // Cancel pending asynchronous operations.
  ASIO_DECL void cancel(implementation_type& impl);

protected:
  // Helper function to start an asynchronous resolve operation.
  ASIO_DECL void start_resolve_op(resolve_op* op);

#if !defined(ASIO_WINDOWS_RUNTIME)
  // Helper class to perform exception-safe cleanup of addrinfo objects.
  class auto_addrinfo
    : private asio::detail::noncopyable
  {
  public:
    explicit auto_addrinfo(asio::detail::addrinfo_type* ai)
      : ai_(ai)
    {
    }

    ~auto_addrinfo()
    {
      if (ai_)
        socket_ops::freeaddrinfo(ai_);
    }

    operator asio::detail::addrinfo_type*()
    {
      return ai_;
    }

  private:
    asio::detail::addrinfo_type* ai_;
  };
#endif // !defined(ASIO_WINDOWS_RUNTIME)

  // Helper class to run the work scheduler in a thread.
  class work_scheduler_runner;

  // Start the work scheduler if it's not already running.
  ASIO_DECL void start_work_thread();

  // The scheduler implementation used to post completions.
#if defined(ASIO_HAS_IOCP)
  typedef class win_iocp_io_context scheduler_impl;
#else
  typedef class scheduler scheduler_impl;
#endif
  scheduler_impl& scheduler_;

private:
  // Mutex to protect access to internal data.
  asio::detail::mutex mutex_;

  // Private scheduler used for performing asynchronous host resolution.
  asio::detail::scoped_ptr<scheduler_impl> work_scheduler_;

  // Thread used for running the work io_context's run loop.
  asio::detail::scoped_ptr<asio::detail::thread> work_thread_;

  // Whether the scheduler locking is enabled.
  bool scheduler_locking_;
};

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "impl/resolver_service_base.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP
