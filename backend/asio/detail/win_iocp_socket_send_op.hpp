//
// detail/win_iocp_socket_send_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_IOCP_SOCKET_SEND_OP_HPP
#define ASIO_DETAIL_WIN_IOCP_SOCKET_SEND_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"

#if defined(ASIO_HAS_IOCP)

#include "bind_handler.hpp"
#include "buffer_sequence_adapter.hpp"
#include "fenced_block.hpp"
#include "handler_alloc_helpers.hpp"
#include "handler_work.hpp"
#include "memory.hpp"
#include "operation.hpp"
#include "socket_ops.hpp"
#include "../error.hpp"

#include "push_options.hpp"

namespace asio {
namespace detail {

template <typename ConstBufferSequence, typename Handler, typename IoExecutor>
class win_iocp_socket_send_op : public operation
{
public:
  ASIO_DEFINE_HANDLER_PTR(win_iocp_socket_send_op);

  win_iocp_socket_send_op(socket_ops::weak_cancel_token_type cancel_token,
      const ConstBufferSequence& buffers, Handler& handler,
      const IoExecutor& io_ex)
    : operation(&win_iocp_socket_send_op::do_complete),
      cancel_token_(cancel_token),
      buffers_(buffers),
      handler_(static_cast<Handler&&>(handler)),
      work_(handler_, io_ex)
  {
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& result_ec,
      std::size_t bytes_transferred)
  {
    asio::error_code ec(result_ec);

    // Take ownership of the operation object.
    ASIO_ASSUME(base != 0);
    win_iocp_socket_send_op* o(static_cast<win_iocp_socket_send_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(
          o->work_));

#if defined(ASIO_ENABLE_BUFFER_DEBUGGING)
    // Check whether buffers are still valid.
    if (owner)
    {
      buffer_sequence_adapter<asio::const_buffer,
          ConstBufferSequence>::validate(o->buffers_);
    }
#endif // defined(ASIO_ENABLE_BUFFER_DEBUGGING)

    socket_ops::complete_iocp_send(o->cancel_token_, ec);

    ASIO_ERROR_LOCATION(ec);

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, asio::error_code, std::size_t>
      handler(o->handler_, ec, bytes_transferred);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      fenced_block b(fenced_block::half);
      ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
      w.complete(handler, handler.handler_);
      ASIO_HANDLER_INVOCATION_END;
    }
  }

private:
  socket_ops::weak_cancel_token_type cancel_token_;
  ConstBufferSequence buffers_;
  Handler handler_;
  handler_work<Handler, IoExecutor> work_;
};

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#endif // defined(ASIO_HAS_IOCP)

#endif // ASIO_DETAIL_WIN_IOCP_SOCKET_SEND_OP_HPP
