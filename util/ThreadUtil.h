#ifndef THREADUTIL_H
#define THREADUTIL_H

#include "Base.h"

namespace ThreadUtil {
    inline void execute_threads(std::function<void(int)> &func, const int num_of_thread) {
        vector<std::thread> exec_threads(num_of_thread);

        for (int thread_index = 0; thread_index < num_of_thread; thread_index++) {
            exec_threads[thread_index] = std::thread(func, thread_index);
        }

        for (int thread_index = 0; thread_index < num_of_thread; thread_index++) {
            exec_threads[thread_index].join();
        }
    }
}
#endif //THREADUTIL_H