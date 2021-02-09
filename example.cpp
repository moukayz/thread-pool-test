#include "my_pthread.h"
#include "thread_pool.h"

#include <chrono>
#include <string>

int task_function(int a)
{
    return a;
}

void void_func(int a)
{
    std::cout << "call function returning void\n";
}

struct foo {
    int member_func(int a)
    {
        return a + b;
    }

    int b = 1;
};

int main()
{
    thread::ThreadPool pool{5};
    // thread::ThreadPool<> pool_std_thread{5};

    std::cout << "\nnormal task\n";
    auto param = 1;
    auto param2 = 2;
    auto result = pool.push(
        [param](int t) {
            // std::this_thread::sleep_for(std::chrono::seconds(3));
            return t + param;
        },
        param2);
    std::cout << result.get() << "\n";

    std::cout << "\ntask with raw function pointer:\n";
    result = pool.push(task_function, 5);
    std::cout << result.get() << "\n";

    std::cout << "\ntask with member function pointer:\n";
    foo ff;
    result = pool.push(&foo::member_func, &ff, 2);
    std::cout << result.get() << "\n";

    std::cout << "\ntask with no parameters:\n";
    result = pool.push([]() { return 1; });
    std::cout << result.get() << "\n";

    std::cout << "\ntask with capture\n";
    result = pool.push([param](){return param;});
    std::cout << result.get() << "\n";

    std::cout << "\ntask with std::function\n";
    std::function<int(int, std::string)> std_func{[](int a, std::string b) {
        return a + b[0];
    }};
    result = pool.push(std_func, 1, "2");
    std::cout << result.get() << "\n";

    std::cout << "\ntask with std::packaged_task\n";
    std::packaged_task<int(int, std::string)> package_func{[](int a, std::string b) {
        // std::this_thread::sleep_for(std::chrono::seconds(2));
        return a + b[0];
    }};
    auto r = package_func.get_future();
    pool.push(std::move(package_func), 1, "2");
    std::cout << r.get() << "\n";

    std::cout << "\ntask by passing reference:\n";
    auto data = 1;
    result = pool.push([](int& data) { return data++; }, std::ref(data));
    std::cout << "result is " << result.get() << "\n";
    std::cout << "data is " << data << "\n";

    std::cout << "\ntask with no return value\n";
    pool.push(void_func, 5);

    pool.resize(5);
    // std::cout << "\nafter resize\n\n";

    // std::cout << "\ntask with raw function pointer:\n";
    // result = pool.push(task_function, 5);
    // std::cout << result.get() << "\n";

    // std::cout << "\ntask with no parameters:\n";
    // result = pool.push([]() { return 1; });
    // std::cout << result.get() << "\n";

    // std::cout << "\ntask by passing reference:\n";
    // result = pool.push([](int& data) { return data++; }, std::ref(data));
    // std::cout << "result is " << result.get() << "\n";
    // std::cout << "data is " << data << "\n";

    // std::cout << "\ntask with no return value\n";
    // pool.push(void_func, 5);

    std::cout << "main end\n";
}