
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>     
#include <iostream>
#include <queue>                
#include <thread>               
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>

class ThreadPool {

private:
    std::mutex taskMutex;
    std::condition_variable conditionVariable;
    std::vector<std::jthread> workers;
    std::queue<std::shared_ptr<std::packaged_task<void()>>> tasks;
    bool endOfLife = false;
public:
    
    explicit ThreadPool(size_t threads);
    
    ~ThreadPool();

    template<class F>
    void AddTask(F&& task) {
        {
            std::unique_lock taskLock(taskMutex);
            tasks.push(std::make_shared<std::packaged_task<void()>>(
                std::forward<decltype(task)>(task)));
        }
        conditionVariable.notify_one();
    }

    template<class F, class... Args, class Return = std::invoke_result_t<F, Args...>>
    std::future<Return> enqueue(F&& f, Args&&... args){
        using packageType = std::packaged_task<Return()>;

        packageType task(std::bind_front(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<Return> futureTask = task.get_future();
        if constexpr (std::is_void_v<Return>)
            AddTask(std::move(task));
        else
            AddTask([&task]{task();});

        futureTask.wait();
        return futureTask;
    }

    void ExecuteTask();
};

#endif


