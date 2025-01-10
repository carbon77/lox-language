#include <stdint.h>
#include <stdlib.h>

#include "chuck.h"
#include "memory.h"

void initChunk(Chunk *chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;

  chunk->lines = NULL;
  chunk->linesCount = 0;
  chunk->linesCapacity = 0;

  initValueArray(&chunk->constants);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->count++;

  // Add new line
  if (chunk->linesCapacity < chunk->linesCount + 2) {
    int oldLinesCapacity = chunk->linesCapacity;
    chunk->linesCapacity = GROW_CAPACITY(oldLinesCapacity);
    chunk->lines =
        GROW_ARRAY(int, chunk->lines, oldLinesCapacity, chunk->linesCapacity);
  }

  if (chunk->lines[chunk->linesCount - 2] == line) {
    chunk->lines[chunk->linesCount - 1]++;
  } else {
    chunk->lines[chunk->linesCount] = line;
    chunk->lines[chunk->linesCount + 1] = 1;
    chunk->linesCount += 2;
  }
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(int, chunk->lines, chunk->linesCapacity);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

int addConstant(Chunk *chunk, Value value) {
  writeValueArray(&chunk->constants, value);
  return chunk->constants.count - 1;
}

int getLine(Chunk *chunk, int instructionIdx) {
  int instIdx = instructionIdx;
  int lineIdx = -2;
  int line = chunk->lines[0];

  bool isLine = false;
  do {
    lineIdx += 2;
    if (lineIdx == 0) {
      isLine = instIdx < chunk->lines[lineIdx + 1];
    } else {
      int left = chunk->lines[lineIdx - 1];
      int right = chunk->lines[lineIdx + 1];
      isLine = left <= instIdx && instIdx < left + right;
    }
  } while (!isLine);
  return chunk->lines[lineIdx];
}
