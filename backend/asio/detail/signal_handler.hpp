//
// detail/signal_handler.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SIGNAL_HANDLER_HPP
#define ASIO_DETAIL_SIGNAL_HANDLER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include "bind_handler.hpp"
#include "fenced_block.hpp"
#include "handler_alloc_helpers.hpp"
#include "handler_work.hpp"
#include "memory.hpp"
#include "signal_op.hpp"

#include "push_options.hpp"

namespace asio {
namespace detail {

template <typename Handler, typename IoExecutor>
class signal_handler : public signal_op
{
public:
  ASIO_DEFINE_HANDLER_PTR(signal_handler);

  signal_handler(Handler& h, const IoExecutor& io_ex)
    : signal_op(&signal_handler::do_complete),
      handler_(static_cast<Handler&&>(h)),
      work_(handler_, io_ex)
  {
  }

  static void do_complete(void* owner, operation* base,
      const asio::error_code& /*ec*/,
      std::size_t /*bytes_transferred*/)
  {
    // Take ownership of the handler object.
    signal_handler* h(static_cast<signal_handler*>(base));
    ptr p = { asio::detail::addressof(h->handler_), h, h };

    ASIO_HANDLER_COMPLETION((*h));

    // Take ownership of the operation's outstanding work.
    handler_work<Handler, IoExecutor> w(
        static_cast<handler_work<Handler, IoExecutor>&&>(
          h->work_));

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    detail::binder2<Handler, asio::error_code, int>
      handler(h->handler_, h->ec_, h->signal_number_);
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
  Handler handler_;
  handler_work<Handler, IoExecutor> work_;
};

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#endif // ASIO_DETAIL_SIGNAL_HANDLER_HPP
