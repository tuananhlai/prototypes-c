#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STB_DS_IMPLEMENTATION
#include "../stb_ds.h"
#include "chap4_1_unionfind.c"

typedef struct {
  size_t node;
  size_t dist;
} Entry;

typedef struct {
  Entry *items;
  size_t len;
  size_t cap;
} Queue;

Queue *queue_create(void) {
  Queue *q = malloc(sizeof(Queue));
  *q = (Queue){.items = NULL, .len = 0, .cap = 0};
  return q;
}

static void realloc_if_needed(Queue *q, size_t len) {
  if (len <= q->cap) {
    return;
  }

  q->cap = (len + 1) * 2;
  q->items = realloc(q->items, q->cap * sizeof(Entry));
}

void queue_enqueue(Queue *q, Entry e) {
  realloc_if_needed(q, q->len + 1);
  q->items[q->len] = e;
  q->len++;
}

int queue_dequeue(Queue *q, Entry *out_entry) {
  if (q->len == 0) {
    return -1;
  }

  *out_entry = q->items[0];
  memmove(&q->items[0], &q->items[1], (q->len - 1) * sizeof(Entry));
  q->len--;
  return 0;
}

void queue_clear(Queue *q) { q->len = 0; }

size_t queue_size(Queue *q) { return q->len; }

void queue_destroy(Queue *q) {
  free(q->items);
  free(q);
}

/**
 * Return the distance of the shortest path between `start` and `end` node.
 */
ssize_t bfs(size_t num_nodes, bool adj_matrix[num_nodes][num_nodes],
            size_t start, size_t end) {
  ssize_t retval = -1;
  Queue *q = queue_create();
  queue_enqueue(q, (Entry){.node = start, .dist = 0});

  bool visited[num_nodes];
  memset(visited, 0, num_nodes * sizeof(bool));
  visited[start] = true;
  Entry cur;
  while (queue_size(q) > 0) {
    queue_dequeue(q, &cur);
    if (cur.node == end) {
      retval = cur.dist;
      goto cleanup;
    }

    for (size_t next_node = 0; next_node < num_nodes; next_node++) {
      if (visited[next_node] || !adj_matrix[cur.node][next_node]) {
        continue;
      }
      visited[next_node] = true;
      queue_enqueue(q, (Entry){.node = next_node, .dist = cur.dist + 1});
    }
  }

cleanup:
  queue_destroy(q);
  return retval;
}

/**
 * Returning a 1D array with length of `num_nodes`, where arr[i] == arr[j] if
 * and only if node i and node js are connected.
 */
void connected_components(size_t num_nodes,
                          const bool adj_matrix[num_nodes][num_nodes],
                          size_t *out_cc) {
  UnionFind *uf = uf_create(num_nodes);
  bool visited[num_nodes];
  memset(visited, 0, num_nodes * sizeof(bool));

  size_t *queue = NULL;
  size_t cur_node;
  for (size_t node = 0; node < num_nodes; node++) {
    if (visited[node]) {
      continue;
    }
    visited[node] = true;
    arrsetlen(queue, 0);
    arrput(queue, node);

    while (arrlenu(queue) > 0) {
      cur_node = queue[0];
      arrdel(queue, 0);
      for (size_t next_node = 0; next_node < num_nodes; next_node++) {
        if (visited[next_node] || !adj_matrix[cur_node][next_node]) {
          continue;
        }
        visited[next_node] = true;
        uf_union(uf, cur_node, next_node);
        arrput(queue, next_node);
      }
    }
  }

  for (size_t node = 0; node < num_nodes; node++) {
    out_cc[node] = uf_find(uf, node);
  }

  arrfree(queue);
  uf_destroy(uf);
}

/**
 * Return a spanning tree in adjacency matrix format of the given graph by
 * writing to `out_spanning_tree`. 0 if success. Non-zero otherwise.
 */
int spanning_tree(size_t num_nodes, const bool adj_matrix[num_nodes][num_nodes],
                  bool out_spanning_tree[num_nodes][num_nodes]) {
  int retval = -1;
  size_t *queue = NULL;
  bool visited[num_nodes];
  memset(visited, 0, num_nodes * sizeof(bool));
  arrput(queue, 0);
  visited[0] = true;

  size_t cur;
  while (arrlenu(queue) > 0) {
    cur = queue[0];
    arrdel(queue, 0);

    for (size_t next_node = 0; next_node < num_nodes; next_node++) {
      if (visited[next_node] || !adj_matrix[cur][next_node]) {
        continue;
      }
      visited[next_node] = true;
      out_spanning_tree[cur][next_node] = true;
      out_spanning_tree[next_node][cur] = true;
      arrput(queue, next_node);
    }
  }

  for (size_t node = 0; node < num_nodes; node++) {
    if (!visited[node]) {
      goto cleanup;
    }
  }

  retval = 0;
cleanup:
  arrfree(queue);
  return retval;
}

typedef struct {
  size_t key;
  size_t val;
} HeapEntry;

typedef struct {
  HeapEntry *arr;
} Heap;

Heap *heap_create() {
  Heap *h = malloc(sizeof(Heap));
  h->arr = NULL;
  arrsetlen(h->arr, 1);
  return h;
}

void heap_destroy(Heap *h) {
  arrfree(h->arr);
  free(h);
}

static void heap_swap(Heap *h, size_t i, size_t j) {
  HeapEntry tmp = h->arr[i];
  h->arr[i] = h->arr[j];
  h->arr[j] = tmp;
}

static void heap_swim(Heap *h, size_t idx) {
  size_t parent_idx;
  while (idx != 1) {
    parent_idx = idx / 2;
    if (h->arr[parent_idx].key <= h->arr[idx].key) {
      break;
    }
    heap_swap(h, parent_idx, idx);
    idx = parent_idx;
  }
}

static void heap_sink(Heap *h, size_t idx) {
  size_t min_child_idx;
  while (idx * 2 < arrlenu(h->arr)) {
    min_child_idx = idx * 2;
    if (min_child_idx + 1 < arrlenu(h->arr) &&
        h->arr[min_child_idx].key > h->arr[min_child_idx + 1].key) {
      min_child_idx++;
    }

    if (h->arr[idx].key <= h->arr[min_child_idx].key) {
      break;
    }

    heap_swap(h, idx, min_child_idx);
    idx = min_child_idx;
  }
}

size_t heap_len(Heap *h) { return arrlenu(h->arr) - 1; }

void heap_push(Heap *h, HeapEntry e) {
  arrput(h->arr, e);
  heap_swim(h, arrlenu(h->arr) - 1);
}

HeapEntry heap_pop(Heap *h) {
  HeapEntry he = h->arr[1];
  heap_swap(h, 1, arrlenu(h->arr) - 1);
  arrpop(h->arr);
  heap_sink(h, 1);
  return he;
}

typedef enum {
  WHITE = 0,
  GRAY = 1,
  BLACK = 2,
} NodeState;

/**
 * Return the total weight of the shortest path from start to end.
 */
int shortest_path(size_t num_nodes, size_t adj_matrix[num_nodes][num_nodes],
                  size_t start, size_t end, size_t *out_path_length) {
  int retval = -1;
  Heap *h = heap_create();
  heap_push(h, (HeapEntry){
                   .key = 0,
                   .val = start,
               });
  HeapEntry cur;
  NodeState state[num_nodes];
  memset(state, 0, num_nodes * sizeof(int));

  while (heap_len(h) > 0) {
    cur = heap_pop(h);
    if (state[cur.val] == BLACK) {
      continue;
    }
    state[cur.val] = BLACK;
    if (cur.val == end) {
      retval = 0;
      *out_path_length = cur.key;
      goto cleanup;
    }

    for (size_t next_node = 0; next_node < num_nodes; next_node++) {
      if (state[next_node] == BLACK || adj_matrix[cur.val][next_node] == 0) {
        continue;
      }
      state[next_node] = GRAY;
      heap_push(h, (HeapEntry){.key = cur.key + adj_matrix[cur.val][next_node],
                               .val = next_node});
    }
  }

cleanup:
  heap_destroy(h);
  return retval;
}