#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char *buffer;
  size_t offset;
  size_t len;
} Arena;

Arena *arena_create(size_t len) {
  Arena *a = malloc(sizeof(Arena));
  a->buffer = malloc(len);
  a->len = len;
  a->offset = 0;
  return a;
}

void *arena_alloc(Arena *a, size_t len) {
  size_t new_offset = a->offset + len;
  if (new_offset > a->len) {
    return NULL;
  }
  void* buf = a->buffer + a->offset;
  a->offset = new_offset;
  return buf;
}

void arena_destroy(Arena *a) {
  free(a->buffer);
  free(a);
}

int main(void) {
  Arena *arena = arena_create(8);
  uint32_t *a = arena_alloc(arena, sizeof(uint32_t));
  *a = 6000;
  uint16_t *b = arena_alloc(arena, sizeof(uint16_t));
  *b = 3000;
  uint8_t *c = arena_alloc(arena, sizeof(uint8_t));
  *c = 150;
  char *d = arena_alloc(arena, sizeof(char));
  *d = 'A';
  char *empty = arena_alloc(arena, sizeof(char));
  assert(empty == NULL);
  printf("a=%u, b=%u, c=%u, d=%u", *a, *b, *c, *d);
  arena_destroy(arena);
  return EXIT_SUCCESS;
}