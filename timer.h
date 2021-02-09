#ifndef TIMER_H
#define TIMER_H

#include "thread_pool.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <utility>
#include <vector>

using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
using time_interval_t = std::chrono::milliseconds;

struct TimerTask {
    explicit TimerTask(std::function<void()> p_callback, time_point_t p_deadline, size_t p_interval, bool p_repeated)
        : callback(p_callback), deadline(p_deadline), interval(p_interval), repeated(p_repeated)
    {
    }

    bool operator<(const TimerTask& rhs)
    {
        return deadline < rhs.deadline;
    }

    std::function<void()> callback;
    time_point_t deadline;
    size_t interval;
    bool repeated;
};

class Timer {
public:
    explicit Timer(std::shared_ptr<thread::ThreadPool> pool) : internal_pool_{pool}
    {
        internal_pool_->push(&Timer::tick, this);
    }

    template <typename F, typename... Args>
    void setTimeout(size_t milliseconds, F&& f, Args&&... args)
    {
        add_task(milliseconds, false, std::forward<F>(f), std::forward<Args>(args)...);
    }

    template <typename F, typename... Args>
    void setInterval(size_t milliseconds, F&& f, Args&&... args)
    {
        add_task(milliseconds, true, std::forward<F>(f), std::forward<Args>(args)...);
    }

    void stop()
    {
        clear = true;
    }

    ~Timer()
    {
        stop();
    }

private:
    void tick()
    {
        while (!clear) {
            std::this_thread::sleep_for(std::chrono::microseconds(100));

            std::lock_guard<std::mutex> lock{task_mutex_};
            auto time_now = std::chrono::system_clock::now();

            for (auto it = tasks_.begin(); it != tasks_.end() && it->deadline <= time_now;) {
                internal_pool_->push(it->callback);

                if (it->repeated) {
                    it->deadline = time_now + std::chrono::milliseconds(it->interval);
                    auto reinsert_pos = std::lower_bound(it,
                        tasks_.end(),
                        it->deadline,
                        [](const TimerTask& task, time_point_t deadline) { return task.deadline < deadline; });
                    tasks_.insert(reinsert_pos, std::move(*it));
                }
                it = tasks_.erase(it);
            }
        }
    }

    template <typename F, typename... Args>
    void add_task(size_t interval, bool repeated, F&& f, Args&&... args)
    {
        using RetType = typename std::result_of<F(Args...)>::type;
        auto task_func =
            std::make_shared<std::function<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::lock_guard<std::mutex> lock{task_mutex_};

        auto deadline_time = std::chrono::system_clock::now() + std::chrono::milliseconds(interval);
        auto insert_pos =
            std::lower_bound(tasks_.begin(), tasks_.end(), deadline_time, [](const TimerTask& task, time_point_t time) {
                return task.deadline < time;
            });

        tasks_.emplace(
            insert_pos,
            [task_func]() { (*task_func)(); },
            deadline_time,
            interval,
            repeated);
    }

    std::shared_ptr<thread::ThreadPool> internal_pool_;
    std::list<TimerTask> tasks_{};
    std::mutex task_mutex_{};
    std::atomic_bool clear{false};
};

#endif