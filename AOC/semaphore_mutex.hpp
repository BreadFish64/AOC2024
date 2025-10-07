#pragma once

#include <chrono>
#include <semaphore>

namespace AOC {

struct SemaphoreMutex {
    // https://cppreference.com/w/cpp/named_req/BasicLockable.html

    void lock() {
        semaphore_.acquire();
    }

    void unlock() {
        semaphore_.release();
    }

    // https://cppreference.com/w/cpp/named_req/Lockable.html

    bool try_lock() {
        return semaphore_.try_acquire();
    }

    // https://cppreference.com/w/cpp/named_req/TimedLockable.html

    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep, Period>& relTime) {
        return semaphore_.try_acquire_for(relTime);
    }

    template <class Clock, class Duration>
    bool try_lock_until(const std::chrono::time_point<Clock, Duration>& absTime) {
        return semaphore_.try_acquire_until(absTime);
    }

    private:
    std::binary_semaphore semaphore_{1};
};

}