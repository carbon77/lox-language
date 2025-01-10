#include "chuck.h"
#include "common.h"
#include "vm.h"
#ifdef DEBUG_SHOW_LINES
#include <stdio.h>
#endif

int main(int argc, char *argv[]) {
  initVM();

  Chunk chunk;
  initChunk(&chunk);

  int constant = addConstant(&chunk, 1.2);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  constant = addConstant(&chunk, 3.4);
  writeChunk(&chunk, OP_CONSTANT, 124);
  writeChunk(&chunk, constant, 124);

  writeChunk(&chunk, OP_ADD, 124);

  constant = addConstant(&chunk, 5.6);
  writeChunk(&chunk, OP_CONSTANT, 125);
  writeChunk(&chunk, constant, 126);

  writeChunk(&chunk, OP_DIVIDE, 126);
  writeChunk(&chunk, OP_NEGATE, 126);

  writeChunk(&chunk, OP_RETURN, 128);

  interpret(&chunk);

#ifdef DEBUG_SHOW_LINES
  for (int i = 0; i < chunk.linesCount; i++) {
    printf("%d ", chunk.lines[i]);
  }
#endif /* ifdef MACRO */
  freeVM();
  freeChunk(&chunk);
  return 0;
}
