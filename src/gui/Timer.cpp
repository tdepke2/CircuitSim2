#include <gui/Timer.h>

#include <limits>




#include <iostream>



namespace gui {

std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, Timer::TimerComparator> Timer::timers_;

std::shared_ptr<Timer> Timer::create(const std::function<void(Timer*)>& callback, Clock::duration interval, unsigned int count) {
    auto timer = std::shared_ptr<Timer>(new Timer(callback, interval, count));
    timers_.push(timer);
    return timer;
}
std::shared_ptr<Timer> Timer::create(const std::function<void()>& callback, Clock::duration interval, unsigned int count) {
    auto timer = std::shared_ptr<Timer>(new Timer(callback, interval, count));
    timers_.push(timer);
    return timer;
}

void Timer::updateTimers() {
    if (timers_.empty()) {
        return;
    }
    const auto now = Clock::now();
    std::vector<std::shared_ptr<Timer>> stillRunning;

    while (!timers_.empty() && timers_.top()->nextTime_ <= now) {
        std::shared_ptr<Timer> timer = timers_.top();
        timers_.pop();
        if (timer->count_ > 0) {
            if (timer->count_ != std::numeric_limits<unsigned int>::max()) {
                --timer->count_;
            }

            using Tag = Callback<Timer*>::Tag;
            if (timer->callback_.getTag() == Tag::withArgs) {
                timer->callback_.value<Tag::withArgs>()(timer.get());
            } else {
                timer->callback_.value<Tag::noArgs>()();
            }

            if (timer->count_ > 0) {
                timer->nextTime_ = timer->nextTime_ + timer->interval_;
                stillRunning.push_back(timer);
            }
        }
    }

    for (const auto& timer : stillRunning) {
        timers_.push(timer);
    }
}

void Timer::setCallback(const std::function<void(Timer*)>& callback) {
    callback_ = callback;
}
void Timer::setCallback(const std::function<void()>& callback) {
    callback_ = callback;
}
void Timer::setInterval(Clock::duration interval) {
    interval_ = interval;
}
void Timer::setCount(unsigned int count) {
    count_ = count;
}
Timer::Clock::duration Timer::getInterval() const {
    return interval_;
}
unsigned int Timer::getCount() const {
    return count_;
}

Timer::Timer(const std::function<void(Timer*)>& callback, Clock::duration interval, unsigned int count) :
    callback_(callback),
    interval_(interval),
    count_(count),
    nextTime_(Clock::now() + interval) {

    std::cout << "Timer::Timer(callback Timer*), the time is " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count() << "\n";
}
Timer::Timer(const std::function<void()>& callback, Clock::duration interval, unsigned int count) :
    callback_(callback),
    interval_(interval),
    count_(count),
    nextTime_(Clock::now() + interval) {

    std::cout << "Timer::Timer(callback void), the time is " << std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now().time_since_epoch()).count() << "\n";
}

}
