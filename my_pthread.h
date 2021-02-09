#ifndef MY_PTHREAD_H
#define MY_PTHREAD_H

#include <pthread.h>

namespace thread {

class MyPthread {
public:
    using thread_func_type = void* (*)(void*);
    using thread_arg_type = void*;

    MyPthread(thread_func_type func, thread_arg_type args ){
        pthread_create(&thread_id_,nullptr, func, args);
    }

    void join(){
        pthread_join(thread_id_, nullptr);
    }

    void detach(){
        pthread_detach(thread_id_);
    }

    private:
    pthread_t thread_id_;
};

}  // namespace thread

#endif