//
// detail/win_global.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_GLOBAL_HPP
#define ASIO_DETAIL_WIN_GLOBAL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include "static_mutex.hpp"
#include "tss_ptr.hpp"

#include "push_options.hpp"

namespace asio {
namespace detail {

template <typename T>
struct win_global_impl
{
  // Destructor automatically cleans up the global.
  ~win_global_impl()
  {
    delete ptr_;
  }

  static win_global_impl instance_;
  static static_mutex mutex_;
  T* ptr_;
  static tss_ptr<T> tss_ptr_;
};

template <typename T>
win_global_impl<T> win_global_impl<T>::instance_ = { 0 };

template <typename T>
static_mutex win_global_impl<T>::mutex_ = ASIO_STATIC_MUTEX_INIT;

template <typename T>
tss_ptr<T> win_global_impl<T>::tss_ptr_;

template <typename T>
T& win_global()
{
  if (static_cast<T*>(win_global_impl<T>::tss_ptr_) == 0)
  {
    win_global_impl<T>::mutex_.init();
    static_mutex::scoped_lock lock(win_global_impl<T>::mutex_);
    if (win_global_impl<T>::instance_.ptr_ == 0)
      win_global_impl<T>::instance_.ptr_ = new T;
    win_global_impl<T>::tss_ptr_ = win_global_impl<T>::instance_.ptr_;
  }

  return *win_global_impl<T>::tss_ptr_;
}

} // namespace detail
} // namespace asio

#include "pop_options.hpp"

#endif // ASIO_DETAIL_WIN_GLOBAL_HPP
