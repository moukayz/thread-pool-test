#include "pthread.h"
#include <functional>

#include <chrono>
#include <iostream>
#include <thread>

#define handle_error_en(en, msg) \
    do {                         \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

class Worker {
public:
    int data{1};
    pthread_t threadId;

    int start_lambda()
    {
        return pthread_create(
            &threadId,
            nullptr,
            [](void* self) -> void* {
                std::cout << static_cast<Worker*>(self)->data << "\n";
                return nullptr;
            },
            this);
    }

    int start_functor()
    {
        struct func {
            void* operator()(void* self)
            {
                std::cout << static_cast<Worker*>(self)->data << "\n";
                return nullptr;
            }
        };



        // return pthread_create(&threadId, nullptr, , this);
    }

    ~Worker()
    {
        pthread_join(threadId, nullptr);
    }
};

int main()
{
    Worker a;
    auto ret = a.start_lambda();
    if (ret != 0) {
        handle_error_en(ret, "pthread_create");
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
}