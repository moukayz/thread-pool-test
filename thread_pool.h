#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "my_thread.h"

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

// test
#include <iostream>

namespace thread {

using TaskFuncType = std::function<void()>;
constexpr size_t DEFAULT_THREAD_NUM = 2;

// template <typename ThreadType = std::thread,
//     typename MutexType = std::mutex,
//     typename CondType = std::condition_variable>
class ThreadPool {
public:
    enum class ThreadStatus { WORKING, FINISHED };

    using pool_type = ThreadPool;

    static void* worker_routine(void* pool_ptr)
    {
        auto this_ptr = static_cast<pool_type*>(pool_ptr);

        thread_local size_t thread_id = this_ptr->thread_count++;

        // test
        {
            std::lock_guard<std::mutex> lock(this_ptr->task_mutex_);
            std::cout << "thread start " << thread_id << "\n";
        }

        while (true) {
            std::unique_lock<std::mutex> lock(this_ptr->task_mutex_);
            this_ptr->task_condition_.wait(lock, [this_ptr] { return this_ptr->stop || !this_ptr->tasks_.empty(); });

            if (this_ptr->stop && this_ptr->tasks_.empty()) {
                // test
                std::cout << "thread stop" << thread_id << "\n";

                return nullptr;
            }

            TaskFuncType task{std::move(this_ptr->tasks_.front())};
            this_ptr->tasks_.pop();

            lock.unlock();

            std::cout << "thread " << thread_id << " active\n";

            task();

            std::cout << "thread " << thread_id << " disactive\n";
        }

        return nullptr;
    }

    ThreadPool(size_t thread_nums = DEFAULT_THREAD_NUM)
        : worker_num_{thread_nums}, worker_status_(thread_nums, ThreadStatus::WORKING)
    {
        for (auto count = 0; count < thread_nums; ++count) {
            workers_.emplace_back(worker_routine, this);
        }
    }

    template <typename Func,
        typename... Args,
        typename = typename std::enable_if<std::is_void<typename std::result_of<Func(Args...)>::type>::value>::type>
    void push(Func&& func, Args&&... args)
    {
        auto task = std::make_shared<std::packaged_task<void()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        {
            std::lock_guard<std::mutex> lock(task_mutex_);
            tasks_.emplace([task] { (*task)(); });
        }
        task_condition_.notify_one();
    }

    template <typename Func,
        typename... Args,
        typename = typename std::enable_if<!std::is_void<typename std::result_of<Func(Args...)>::type>::value>::type>
    auto push(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type>
    {
        using RetType = typename std::result_of<Func(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        auto res_future = task->get_future();

        {
            std::lock_guard<std::mutex> lock(task_mutex_);
            tasks_.emplace([task] { (*task)(); });
        }
        task_condition_.notify_one();

        return res_future;
    }

    bool resize(size_t new_num)
    {
        if (new_num > std::thread::hardware_concurrency()) {
            return false;
        }

        if (new_num > worker_num_) {
            for (auto i = worker_num_; i < new_num; ++i) {
                workers_.emplace_back(worker_routine, this);
            }
            worker_status_.resize(new_num, ThreadStatus::WORKING);
        }
        else if (new_num < worker_num_) {
            return false;
        }

        worker_num_ = new_num;
        return true;
    }

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock{task_mutex_};
            stop = true;
        }
        task_condition_.notify_all();

        for (auto& worker : workers_) {
            worker.join();
        }
    }

private:
    // std::vector<std::thread> workers_;
    std::vector<MyThread> workers_;
    std::vector<ThreadStatus> worker_status_;
    std::queue<TaskFuncType> tasks_{};

    std::mutex task_mutex_{};
    std::condition_variable task_condition_{};
    bool stop{false};
    size_t worker_num_;

    // test
    std::atomic<size_t> thread_count{0};
};

}  // namespace thread

#endif