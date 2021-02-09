#include "thread_pool.h"
#include "timer.h"

#include <iostream>

int main()
{
    auto pool = std::make_shared<thread::ThreadPool>(5);
    Timer t{pool};

    auto cap = 1;
    auto param = 2;

    // for (auto i = 1; i < 1000; i++) {
    //     t.setTimeout(5000, []() { return 1 + 1; });
    // }
    t.setTimeout(2000, []() { std::cout << "called after 2s\n"; });
    t.setTimeout(1000, []() { std::cout << "called after 1s\n"; });
    t.setInterval(1000, []() { std::cout << "interval called after 1s\n"; });
    t.setTimeout(3000, []() { std::cout << "called after 3s\n"; });
    t.setTimeout(2500, []() { std::cout << "called after 2.5s\n"; });

    auto start = std::chrono::system_clock::now();
    t.setTimeout(
        5000,
        [cap, start](int param) -> int {
            auto end = std::chrono::system_clock::now();
            auto last = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            std::cout << "hey\n";
            std::cout << "called after " << last << " \n";

            return cap + param;
        },
        param);

    // t.setInterval(
    //     2000,
    //     [cap](int param) {
    //         std::cout << "oops"
    //                   << "\n";
    //     },
    //     param);

    // std::cout << res.get() << "\n ";
    // t.tick();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    t.stop();
}