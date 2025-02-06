#pragma once
#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <atomic>
#include "queueT.h"

class ThreadPool
{
public:
    explicit ThreadPool(size_t num_threads);

    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread> workers;
    Queue<std::function<void()>, 100000> tasks;
    std::atomic<bool> stop;
};

#include "thread_pool.cpp"