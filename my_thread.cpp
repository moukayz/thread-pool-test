#include "my_thread.h"

#include <thread>

namespace thread {

class MyThread::thread_impl {
public:
    explicit thread_impl(thread_func_t func, thread_arg_t arg) : thread_(func, arg) {}

    void join()
    {
        thread_.join();
    }

    void detach()
    {
        thread_.detach();
    }

private:
    std::thread thread_;
};

MyThread::MyThread(thread_func_t func, thread_arg_t arg) : impl(new thread_impl(func, arg)) {}
MyThread::MyThread(MyThread&&) = default;
MyThread& MyThread::operator=(MyThread&&) = default;
MyThread::~MyThread() = default;

void MyThread::join()
{
    impl->join();
}

void MyThread::detach()
{
    impl->detach();
}
}  // namespace thread