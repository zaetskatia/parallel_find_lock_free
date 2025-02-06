
#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <optional>
#include <vector>
#include <thread>
#include <vector>
// #include "thread_pool.h"
#include "thread_pool_mutex.h"

const int grain = 5000; // Grain size for chunking the data

// Example Record structure
struct Record
{
    int price;
    enum class Color
    {
        red,
        blue,
        green
    } color;

    friend std::ostream &operator<<(std::ostream &os, const Record &r)
    {
        os << "Price: " << r.price << ", Color: ";
        switch (r.color)
        {
        case Color::red:
            os << "red";
            break;
        case Color::blue:
            os << "blue";
            break;
        case Color::green:
            os << "green";
            break;
        }
        return os;
    }
};

template <typename Pred>
std::vector<Record *> find_all_rec(std::vector<Record> &vr, int first, int last, Pred pr)
{
    std::vector<Record *> results;
    for (int i = first; i < last; ++i)
    {
        if (pr(vr[i]))
        {
            results.push_back(&vr[i]);
        }
    }
    return results;
}

template <typename Pred>
std::vector<Record *> pfind_all(std::vector<Record> &vr, Pred pr, ThreadPool &pool)
{
    assert(vr.size() % grain == 0);

    std::vector<std::future<std::vector<Record *>>> futures;
    for (int i = 0; i < vr.size(); i += grain)
    {
        futures.push_back(pool.enqueue(find_all_rec<Pred>, std::ref(vr), i, i + grain, pr));
    }

    std::vector<Record *> results;
    for (auto &fut : futures)
    {
        auto partial_results = fut.get(); // Collect results from each future
        results.insert(results.end(), partial_results.begin(), partial_results.end());
    }

    return results; // Return all matches
}

std::vector<Record *> find_all_cheap_red(std::vector<Record> &goods, ThreadPool &pool)
{
    return pfind_all(goods, [](Record &r)
                     { return r.price < 200 && r.color == Record::Color::red; }, pool);
}

void benchmark_parallel_find(std::vector<Record> &goods, ThreadPool &pool, int iterations = 5)
{
    using namespace std::chrono;

    std::cout << "Running Parallel Find Benchmark...\n";

    double total_all_time = 0;
    std::vector<Record *> matches;

    for (int i = 0; i < iterations; ++i)
    {
        auto start = high_resolution_clock::now();
        matches = find_all_cheap_red(goods, pool);
        auto end = high_resolution_clock::now();

        double duration = duration_cast<milliseconds>(end - start).count();
        total_all_time += duration;

        std::cout << "find_all_cheap_red() run " << i + 1 << ": " << duration << " ms\n";
    }

    std::cout << "Average find_all_cheap_red() Time: " << (total_all_time / iterations) << " ms\n";
    std::cout << "Total Matches Found: " << matches.size() << "\n";

    double all_throughput = goods.size() / (total_all_time / iterations / 1000.0);

    std::cout << "\nThroughput:\n";
    std::cout << "find_all_cheap_red() Throughput: " << all_throughput << " records/sec\n";
    std::cout << "=====================================\n";
}

int main()
{
    std::vector<Record> goods;
    for (int i = 0; i < 1000000; ++i)
    {
        goods.push_back({i % 300,
                         static_cast<Record::Color>(i % 3)});
    }

    ThreadPool pool(std::thread::hardware_concurrency());

    benchmark_parallel_find(goods, pool);

    return 0;
}