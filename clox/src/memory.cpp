#include <cstdlib>
#include "common.h"
#include "memory.h"

void *reallocate(void *pointer, size_t old_size, size_t new_size)
{
  if (new_size == 0)
  {
    delete pointer;
    return nullptr;
  }

  void *result = std::realloc(pointer, new_size);

  if (!result)
  {
    return nullptr;
  }

  return result;
}
