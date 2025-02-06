#pragma once
#include <atomic>
#include <cstddef>
#include <type_traits>

template <typename T, size_t size>
class Queue
{

public:
  Queue();

  bool Push(T &&element);
  bool Pop(T &element);

private:
  struct alignas(64) Slot
  {
    T val;
    std::atomic_size_t pop_count;
    std::atomic_size_t push_count;

    Slot() : pop_count(0U), push_count(0U) {}
  };

private:
  Slot _data[size];

  std::atomic_size_t _r_count;
  std::atomic_size_t _w_count;
};

#include "queueT.cpp"
