//
// detail/reactive_socket_recv_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_REACTIVE_SOCKET_RECV_OP_HPP
#define ASIO_DETAIL_REACTIVE_SOCKET_RECV_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include "bind_handler.hpp"
#include "buffer_sequence_adapter.hpp"
#include "fenced_block.hpp"
#include "handler_alloc_helpers.hpp"
#include "handler_work.hpp"
#include "memory.hpp"
#include "reactor_op.hpp"
#include "socket_ops.hpp"

#include "push_options.hpp"

namespace asio {
namespace detail {

template <typename MutableBufferSequence>
class reactive_socket_recv_op_base : public reactor_op
{
public:
  reactive_socket_recv_op_base(const asio::error_code& success_ec,
      socket_type socket, socket_ops::state_type state,
      const MutableBufferSequence& buffers,
      socket_base::message_flags flags, func_type complete_func)
    : reactor_op(success_ec,
        &reactive_socket_recv_op_base::do_perform, complete_func),
      socket_(socket),
      state_(state),
      buffers_(buffers),
      flags_(flags)
  {
  }

  static status do_perform(reactor_op* base)
  {
    ASIO_ASSUME(base != 0);
    reactive_socket_recv_op_base* o(
        static_cast<reactive_socket_recv_op_base*>(base));

    typedef buffer_sequence_adapter<asio::mutable_buffer,
        MutableBufferSequence> bufs_type;

    status result;
    if (bufs_type::is_single_buffer)
    {
      result = socket_ops::non_blocking_recv1(o->socket_,
          bufs_type::first(o->buffers_).data(),
          bufs_type::first(o->buffers_).size(), o->flags_,
          (o->state_ & socket_ops::stream_oriented) != 0,
          o->ec_, o->bytes_transferred_) ? done : not_done;
    }
    else
    {
      bufs_type bufs(o->buffers_);
      result = socket_ops::non_blocking_recv(o->socket_,
          bufs.buffers(), bufs.count(), o->flags_,
          (o->state_ & socket_ops::stream_oriented) != 0,
          o->ec_, o->bytes_transferred_) ? done : not_done;
    }

    if (result == done)
      if ((o->state_ & socket_ops::stream_oriented) != 0)
        if (o->bytes_transferred_ == 0)
          result = done_and_exhausted;

    ASIO_HANDLER_REACTOR_OPERATION((*o, "non_blocking_recv",
          o->ec_, o->bytes_transferred_));

    return result;
  }

private:
  socket_type socket_;
  socket_ops::state_type state_;
  MutableBufferSequence buffers_;
  socket_base::message_flags flags_;
};

template <typename MutableBufferSequence, typename Handler, typename IoExecutor>
class reactive_socket_recv_op :
  public reactive_socket_recv_op_base<MutableBufferSequence>
{
public:
  typedef Handler handler_type;
  typedef IoExecutor io_executor_type;

  ASIO_DEFINE_HANDLER_PTR(reactive_socket_recv_op);

  reactive_socket_recv_op(const asio::error_code& success_ec,
      socket_type socket, socket_ops::state_type state,
      const MutableBufferSequence& buffers, socket_base::message_flags flags,
      Handler& handler, const IoExecutor& io_ex)
    : reactive_socket_recv_op_base<MutableBufferSequence>(success_ec, socket,
        state, buffers, flags, &reactive_socket_recv_op::do_complete),
      handler_(static_cast<Handler&&>(handler)),
      work_(handler_, io_ex)
  {
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    ASIO_ASSUME(base != 0);
    reactive_socket_recv_op* o(static_cast<reactive_socket_recv_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(
          o->work_));

    ASIO_ERROR_LOCATION(o->ec_);

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, asio::error_code, std::size_t>
      handler(o->handler_, o->ec_, o->bytes_transferred_);
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

  static void do_immediate(operation* base, bool, const void* io_ex)
  {
    // Take ownership of the handler object.
    ASIO_ASSUME(base != 0);
    reactive_socket_recv_op* o(static_cast<reactive_socket_recv_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    ASIO_HANDLER_COMPLETION((*o));

    // Take ownership of the operation's outstanding work.
    immediate_handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(
          o->work_));

    ASIO_ERROR_LOCATION(o->ec_);

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, asio::error_code, std::size_t>
      handler(o->handler_, o->ec_, o->bytes_transferred_);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
    w.complete(handler, handler.handler_, io_ex);
    ASIO_HANDLER_INVOCATION_END;
  }

private:
  Handler handler_;
  handler_work<Handler, IoExecutor> work_;
};

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#endif // ASIO_DETAIL_REACTIVE_SOCKET_RECV_OP_HPP
