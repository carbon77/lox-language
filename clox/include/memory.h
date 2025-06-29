#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include "common.h"

constexpr size_t MIN_CAPACITY = 8;

void *reallocate(void *pointer, size_t oldSize, size_t newSize);

inline size_t grow_capacity(size_t capacity)
{
  return capacity < 8 ? 8 : capacity * 2;
}

template <typename T>
T *grow_array(T *pointer, size_t old_count, size_t new_count)
{
  return static_cast<T *>(reallocate(pointer, sizeof(T) * old_count, sizeof(T) * new_count));
}

template <typename T>
T *free_array(T *pointer, size_t old_count)
{
  return static_cast<T *>(reallocate(pointer, sizeof(T) * old_count, 0));
}

#endif // !CLOX_MEMORY_H
