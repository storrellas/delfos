/*
 * Barrier.h
 *
 *  Created on: Feb 22, 2017
 *      Author: mjlopez
 */

#pragma once

#include <condition_variable>
#include <mutex>

#include <wcore/logging/Logger.h>

#include "utils/spy.h"



namespace delfos {
namespace enzo {

class Barrier {
// http://stackoverflow.com/questions/24465533/implementing-boostbarrier-in-c11
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    size_t _count;
public:
    explicit Barrier(std::size_t count=0)
    : _mutex(),
      _cv(),
      _count{count}
      {}
    void wait() {
        std::unique_lock<std::mutex> lk{_mutex};
        BLOG_TRACE(LogItem(delfos::core::void_event_type::undefined, "before wait"));
        _cv.wait(lk, [this](){ return _count == 0; });
        BLOG_TRACE(LogItem(delfos::core::void_event_type::undefined, "after wait"));
    }
    void increment() {
      std::unique_lock<std::mutex> lk{_mutex};
      BLOG_TRACE(
          LogItem(
              delfos::core::void_event_type::undefined,
              delfos::core::concat("_count: '", _count, "'")));
      if (--_count == 0) {
          _cv.notify_all();
          BLOG_TRACE(
              LogItem(
                  delfos::core::void_event_type::undefined,
                  "notify!"));
      }
    }
    void set(std::size_t count) {
      _count = count;
    }
};

} /* namespace enzo */
} /* namespace delfos */
