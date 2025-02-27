#include "queueT.h"

template <typename T, size_t size>
Queue<T, size>::Queue() : _r_count(0U), _w_count(0U) {}

template <typename T, size_t size>
bool Queue<T, size>::Push(T &&element)
{
    size_t w_count = _w_count.load(std::memory_order_relaxed);

    while (true)
    {
        const size_t index = w_count % size;

        const size_t push_count =
            _data[index].push_count.load(std::memory_order_acquire);
        const size_t pop_count =
            _data[index].pop_count.load(std::memory_order_relaxed);

        if (push_count > pop_count)
        {
            return false;
        }

        const size_t revolution_count = w_count / size;
        const bool our_turn = revolution_count == push_count;

        if (our_turn)
        {
            /* Try to acquire the slot by bumping the monotonic write counter */
            if (_w_count.compare_exchange_weak(w_count, w_count + 1U,
                                               std::memory_order_relaxed))
            {
                _data[index].val = std::move(element);
                _data[index].push_count.store(push_count + 1U,
                                              std::memory_order_release);
                return true;
            }
        }
        else
        {
            w_count = _w_count.load(std::memory_order_relaxed);
        }
    }
}

template <typename T, size_t size>
bool Queue<T, size>::Pop(T &element)
{
    size_t r_count = _r_count.load(std::memory_order_relaxed);

    while (true)
    {
        const size_t index = r_count % size;

        const size_t pop_count =
            _data[index].pop_count.load(std::memory_order_acquire);
        const size_t push_count =
            _data[index].push_count.load(std::memory_order_relaxed);

        if (pop_count == push_count)
        {
            return false;
        }

        const size_t revolution_count = r_count / size;
        const bool our_turn = revolution_count == pop_count;

        if (our_turn)
        {
            /* Try to acquire the slot by bumping the monotonic read counter. */
            if (_r_count.compare_exchange_weak(r_count, r_count + 1U,
                                               std::memory_order_relaxed))
            {
                element = std::move(_data[index].val);
                _data[index].pop_count.store(pop_count + 1U,
                                             std::memory_order_release);
                return true;
            }
        }
        else
        {
            r_count = _r_count.load(std::memory_order_relaxed);
        }
    }
}
