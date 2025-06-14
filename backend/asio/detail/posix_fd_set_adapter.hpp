//
// detail/posix_fd_set_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_POSIX_FD_SET_ADAPTER_HPP
#define ASIO_DETAIL_POSIX_FD_SET_ADAPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"

#if !defined(ASIO_WINDOWS) \
  && !defined(__CYGWIN__) \
  && !defined(ASIO_WINDOWS_RUNTIME)

#include <cstring>
#include "noncopyable.hpp"
#include "reactor_op_queue.hpp"
#include "socket_types.hpp"

#include "push_options.hpp"

namespace asio {
namespace detail {

// Adapts the FD_SET type to meet the Descriptor_Set concept's requirements.
class posix_fd_set_adapter : noncopyable
{
public:
  posix_fd_set_adapter()
    : max_descriptor_(invalid_socket)
  {
    using namespace std; // Needed for memset on Solaris.
    FD_ZERO(&fd_set_);
  }

  void reset()
  {
    using namespace std; // Needed for memset on Solaris.
    FD_ZERO(&fd_set_);
  }

  bool set(socket_type descriptor)
  {
    if (descriptor < (socket_type)FD_SETSIZE)
    {
      if (max_descriptor_ == invalid_socket || descriptor > max_descriptor_)
        max_descriptor_ = descriptor;
      FD_SET(descriptor, &fd_set_);
      return true;
    }
    return false;
  }

  void set(reactor_op_queue<socket_type>& operations, op_queue<operation>& ops)
  {
    reactor_op_queue<socket_type>::iterator i = operations.begin();
    while (i != operations.end())
    {
      reactor_op_queue<socket_type>::iterator op_iter = i++;
      if (!set(op_iter->first))
      {
        asio::error_code ec(error::fd_set_failure);
        operations.cancel_operations(op_iter, ops, ec);
      }
    }
  }

  bool is_set(socket_type descriptor) const
  {
    return FD_ISSET(descriptor, &fd_set_) != 0;
  }

  operator fd_set*()
  {
    return &fd_set_;
  }

  socket_type max_descriptor() const
  {
    return max_descriptor_;
  }

  void perform(reactor_op_queue<socket_type>& operations,
      op_queue<operation>& ops) const
  {
    reactor_op_queue<socket_type>::iterator i = operations.begin();
    while (i != operations.end())
    {
      reactor_op_queue<socket_type>::iterator op_iter = i++;
      if (is_set(op_iter->first))
        operations.perform_operations(op_iter, ops);
    }
  }

private:
  mutable fd_set fd_set_;
  socket_type max_descriptor_;
};

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#endif // !defined(ASIO_WINDOWS)
       // && !defined(__CYGWIN__)
       // && !defined(ASIO_WINDOWS_RUNTIME)

#endif // ASIO_DETAIL_POSIX_FD_SET_ADAPTER_HPP
