#include "serial_executor.h"

#include <iostream>
#include <memory>
#include "thread_pool.h"

int main()
{

    auto pool_ptr = std::make_shared<thread::ThreadPool>();

    thread::SerialExecutor e{pool_ptr};
    auto a = e.execute([](int a) { return a; }, 1);
    std::cout << "test" << a.get() << "\n";

    auto b = e.execute([](int b) { return b; }, 2);
    std::cout << "test" << b.get() << "\n";

    auto c = e.execute([](int c) { return c; }, 3);
    std::cout << "test" << c.get() << "\n";

    // e.execute([](int b){
    //     std::cout << "test" << b << "\n";
    // },3);
}