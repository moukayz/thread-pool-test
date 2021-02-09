#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <functional>
#include <memory>
#include <thread>
#include <utility>

namespace thread {

using thread_func_t = void* (*)(void*);
using thread_arg_t = void*;

class MyThread {
public:
    explicit MyThread(thread_func_t func, thread_arg_t arg);

    void join();

    void detach();

    ~MyThread();

    MyThread(MyThread&&);
    MyThread(const MyThread&) = delete;

    MyThread& operator=(MyThread&&);
    MyThread& operator=(const MyThread&) = delete;

private:
    class thread_impl;
    std::unique_ptr<thread_impl> impl;
};

}  // namespace thread

#endif