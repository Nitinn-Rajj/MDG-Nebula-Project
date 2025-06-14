//
// detail/timer_scheduler.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_TIMER_SCHEDULER_HPP
#define ASIO_DETAIL_TIMER_SCHEDULER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include "timer_scheduler_fwd.hpp"

#if defined(ASIO_WINDOWS_RUNTIME)
# include "winrt_timer_scheduler.hpp"
#elif defined(ASIO_HAS_IOCP)
# include "win_iocp_io_context.hpp"
#elif defined(ASIO_HAS_IO_URING_AS_DEFAULT)
# include "io_uring_service.hpp"
#elif defined(ASIO_HAS_EPOLL)
# include "epoll_reactor.hpp"
#elif defined(ASIO_HAS_KQUEUE)
# include "kqueue_reactor.hpp"
#elif defined(ASIO_HAS_DEV_POLL)
# include "dev_poll_reactor.hpp"
#else
# include "select_reactor.hpp"
#endif

#endif // ASIO_DETAIL_TIMER_SCHEDULER_HPP
