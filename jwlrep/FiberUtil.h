// SPDX-License-Identifier: MIT

// Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

#pragma once

#include <boost/fiber/all.hpp>
#include <cassert>

namespace jwlrep {

namespace detail {

inline void waitAllImpl(std::shared_ptr<boost::fibers::barrier> /*unused*/) {}

template <typename Fn, typename... Fns>
void waitAllImpl(std::shared_ptr<boost::fibers::barrier> barrier, Fn&& function,
                 Fns&&... functions) {
  boost::fibers::fiber(std::bind(
                           [](std::shared_ptr<boost::fibers::barrier>& barrier,
                              typename std::decay<Fn>::type& function) mutable {
                             function();
                             barrier->wait();
                           },
                           barrier, std::forward<Fn>(function)))
      .detach();
  waitAllImpl(barrier, std::forward<Fns>(functions)...);
}

}  // namespace detail

template <typename... Fns>
void waitAll(Fns&&... functions) {
  std::size_t const count(sizeof...(functions));
  auto barrier = std::make_shared<boost::fibers::barrier>(count + 1);
  waitAllImpl(&barrier, std::forward<Fns>(functions)...);
  barrier->wait();
}

template <typename T>
class nchannel {
 public:
  nchannel(boost::fibers::buffered_channel<T>& channel, std::size_t limit)
      : channel_(channel), limit_(limit) {
    assert(channel_);
    if (limit_ == 0) {
      channel_.close();
    }
  }

  auto push(T&& value) -> boost::fibers::channel_op_status {
    auto const ok = channel_->push(std::forward<T>(value));
    if (ok == boost::fibers::channel_op_status::success && --limit_ == 0) {
      channel_.close();
    }
    return ok;
  }

 private:
  boost::fibers::buffered_channel<T>& channel_;
  std::size_t limit_;
};

}  // namespace jwlrep