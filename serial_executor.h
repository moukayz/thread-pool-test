#ifndef SERIAL_EXECUTOR_H
#define SERIAL_EXECUTOR_H

#include "thread_pool.h"

#include <memory>
#include <queue>
#include <utility>
#include <vector>

namespace thread {

class SerialExecutor {
public:
    using pool_type = ThreadPool;

    explicit SerialExecutor(std::shared_ptr<pool_type> pool) : internal_pool_{pool} {}

    template <typename F, typename... Args>
    auto execute(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using RetType = typename std::result_of<F(Args...)>::type;

        auto task =
            std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto res = task->get_future();

        {
            std::lock_guard<std::mutex> lock{queue_mutex_};
            serial_tasks_.emplace([task, this] {
                (*task)();
                this->scheduleNext();
            });
        }

        if (!active) {
            scheduleNext();
        }
        return res;
    }

private:
    void scheduleNext()
    {
        TaskFuncType task{};

        std::unique_lock<std::mutex> lock{queue_mutex_};

        if (!serial_tasks_.empty()) {
            task = std::move(serial_tasks_.front());
            serial_tasks_.pop();
            active = true;
        }
        else {
            active = false;
        }

        lock.unlock();

        if (task) {
            internal_pool_->push(std::move(task));
        }
    }

    std::shared_ptr<pool_type> internal_pool_{};
    std::queue<TaskFuncType> serial_tasks_{};
    std::mutex queue_mutex_{};
    std::atomic<bool> active{false};
};
}  // namespace thread

#endif