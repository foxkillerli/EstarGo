//
// Created by Kaihua Li on 2022/7/4.
//

#ifndef ESTAR_GO_EXECUTOR_H
#define ESTAR_GO_EXECUTOR_H

#include <iostream>
#include <functional>
#include <thread>
#include <condition_variable>
#include <future>
#include <atomic>
#include <vector>
#include <queue>
#include <stdexcept>
#include <memory>

class Executor {
    // type define
    using Task = std::function<void()>;

private:
    //thread mPool
    std::vector<std::thread> mPool;
    //task queue
    std::queue<Task> mTaskQueue;
    //synchronization
    std::mutex mTaskQueueMutex;
    //notification
    std::condition_variable cv_task;
    //is closed
    std::atomic<bool> stop;
    //is exit
    volatile bool exit;

public:
    //constructor
    Executor(size_t size = 4)
            :stop {false}
    {
        size = size < 1 ? 1 : size;
        for(size_t i = 0; i < size; ++i) {
            mPool.emplace_back(&Executor::schedule, this);
        }
        exit = false;
    }

    ~Executor() {
        for (std::thread& thread : mPool) {
            // just let threads be
            thread.detach();
        }
        exit = true;
    }

    void shutdown() {
        stop.store(true);
    }

    void restart() {
        stop.store(false);
    }

    template<class F, class... Args>
    auto commit(F&& f, Args&&... args) -> std::future<decltype(std::ref(f)(args...))> {
        if(stop.load()) {
            // executor has been switched off
            throw std::runtime_error("task executor have closed commit.");
        }
        using ResType = decltype(std::ref(f)(args...));
        auto task = std::make_shared<std::packaged_task<ResType()> >(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        {
            std::lock_guard<std::mutex> lock {mTaskQueueMutex};
            mTaskQueue.emplace([task](){
                (*task)();
            });
        }
        cv_task.notify_all();

        std::future<ResType> future = task->get_future();
        return future;
    }

private:
    // get one task
    Task get_task() {
        std::unique_lock<std::mutex> lock {mTaskQueueMutex};
        cv_task.wait(lock, [this](){ return !mTaskQueue.empty();} );
        Task task { std::move(mTaskQueue.front()) };
        mTaskQueue.pop();
        return task;
    }

    void schedule() {
        while(true && !exit) {
            if (Task task = get_task()) {
                task();
            } else {
                // return;   // done
            }
        }
    }
};

typedef std::shared_ptr<Executor> ExecutorPtr;
#endif //ESTAR_GO_EXECUTOR_H