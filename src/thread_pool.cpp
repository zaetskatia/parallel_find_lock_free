#pragma once

#include "thread_pool.h"

ThreadPool::ThreadPool(size_t num_threads) : stop(false)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers.emplace_back([this]
                             {
            while (!stop)
            {
                std::function<void()> task;
                if(tasks.Pop(task))
                {
                    task();
                }
                else
                {
                    std::this_thread::yield();
                }
            } });
    }
}

ThreadPool::~ThreadPool()
{
    stop = true;

    for (std::thread &worker : workers)
        worker.join();
}

template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();

    while (!tasks.Push([task]()
                       { (*task)(); }))
    {
        std::this_thread::yield();
    }

    return res;
}
