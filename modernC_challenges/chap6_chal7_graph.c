#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  size_t node;
  size_t dist;
} Entry;

typedef struct {
  Entry* items;
  size_t len;
  size_t cap;
} Queue;

Queue queue_create(void) { return (Queue){.items = NULL, .len = 0, .cap = 0}; }

static void realloc_if_needed(Queue* q, size_t len) {
  if (len <= q->cap) {
    return;
  }

  q->cap = (len + 1) * 2;
  q->items = realloc(q->items, q->cap * sizeof(Entry));
}

void enqueue(Queue* q, Entry e) {
  realloc_if_needed(q, q->len + 1);
  q->items[q->len] = e;
  q->len++;
}

int dequeue(Queue* q, Entry* out_entry) {
  if (q->len == 0) {
    return -1;
  }

  *out_entry = q->items[0];
  memmove(&q->items[0], &q->items[1], (q->len - 1) * sizeof(Entry));
  q->len--;
  return 0;
}


void queue_destroy(Queue* q) {
  free(q->items);
  q->items = NULL;
  q->len = 0;
  q->cap = 0;
}

size_t bfs(size_t num_nodes, bool adj_matrix[num_nodes][num_nodes],
           size_t start, size_t end) {
  
  return 0;
}
