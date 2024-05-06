#pragma once

#include <gui/Signal.h>

#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace gui {

/**
 * The `Timer` runs a function callback at a specified interval of time, this
 * can be used to schedule events. Timers start ticking when constructed, and
 * stop when the count is reached. In order to keep the timers running, the
 * `updateTimers()` function must be called periodically.
 */
class Timer {
public:
    using Clock = std::chrono::steady_clock;

    // When the interval is zero the timer runs every time `updateTimers()` is
    // called, when the count is `std::numeric_limits<unsigned int>::max()` the
    // timer will run forever.
    static std::shared_ptr<Timer> create(const std::function<void(Timer*)>& callback, Clock::duration interval = std::chrono::milliseconds(0), unsigned int count = 1);
    static std::shared_ptr<Timer> create(const std::function<void()>& callback, Clock::duration interval = std::chrono::milliseconds(0), unsigned int count = 1);
    // Measures the current time and updates any timers that are ready.
    static void updateTimers();

    Timer(const Timer& rhs) = delete;
    Timer& operator=(const Timer& rhs) = delete;

    void setCallback(const std::function<void(Timer*)>& callback);
    void setCallback(const std::function<void()>& callback);
    void setInterval(Clock::duration interval);
    // Setting the count to zero will stop the timer.
    void setCount(unsigned int count);
    Clock::duration getInterval() const;
    unsigned int getCount() const;

protected:
    Timer(const std::function<void(Timer*)>& callback, Clock::duration interval, unsigned int count);
    Timer(const std::function<void()>& callback, Clock::duration interval, unsigned int count);

private:
    struct TimerComparator {
        inline bool operator()(const std::shared_ptr<Timer>& lhs, const std::shared_ptr<Timer>& rhs) {
            return lhs->nextTime_ > rhs->nextTime_;
        }
    };

    static std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, TimerComparator> timers_;

    Callback<Timer*> callback_;
    Clock::duration interval_;
    unsigned int count_;
    Clock::time_point nextTime_;
};

} // namespace gui
