#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"


#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObject(sizeof(type), objectType)

static Obj *allocateObject(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

static ObjString *allocateString(char *chars, int len, uint32_t hash) {
  ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->len = len;
  string->chars = chars;
  string->hash = hash;

  tableSet(&vm.strings, string, NIL_VAL);

  return string;
}

static uint32_t hashString(const char *key, int len) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < len; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}

ObjString *copyString(const char *chars, int len) {
  uint32_t hash = hashString(chars, len);
  ObjString *interned = tableFindString(&vm.strings, chars, len, hash);
  if (interned != NULL)
    return interned;

  char *heapChars = ALLOCATE(char, len + 1);
  memcpy(heapChars, chars, len);
  heapChars[len] = '\0';

  return allocateString(heapChars, len, hash);
}

ObjString *takeString(char *chars, int len) {
  uint32_t hash = hashString(chars, len);
  ObjString *interned = tableFindString(&vm.strings, chars, len, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, len + 1);
    return interned;
  }
  return allocateString(chars, len, hash);
}

void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
  case OBJ_STRING:
    printf("%s", AS_CSTRING(value));
    break;
  }
}
