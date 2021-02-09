#include <iostream>
#include <future>
#include <functional>
#include <thread>

int main(){
    std::packaged_task<int()> task([](){
        std::cout << "in task id: " << std::this_thread::get_id() << "\n";
        return 1;
    });

    task();

    std::cout << "in main id: " << std::this_thread::get_id() << "\n";
}