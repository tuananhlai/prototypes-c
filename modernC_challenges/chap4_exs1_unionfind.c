#include <stdio.h>
#include <stdlib.h>

typedef struct {
  size_t* parent;
  size_t parent_len;
} UnionFind;

UnionFind* uf_create(size_t n) {
  UnionFind* uf = malloc(sizeof(UnionFind));
  uf->parent = malloc(n * sizeof(size_t));
  uf->parent_len = n;

  for (size_t i = 0; i < uf->parent_len; i++) {
    uf->parent[i] = i;
  }
  return uf;
}

size_t uf_find(UnionFind* uf, size_t v) {
  while (v != uf->parent[v]) {
    v = uf->parent[v];
  }
  return v;
}

void uf_find_replace(UnionFind* uf, size_t v, size_t new_root) {
  size_t next_parent;
  while (true) {
    next_parent = uf->parent[v];
    uf->parent[v] = new_root;

    if (v == next_parent) {
      break;
    }
    v = next_parent;
  }
}

size_t uf_find_compress(UnionFind* uf, size_t v) {
  if (v == uf->parent[v]) {
    return v;
  }
  size_t root = uf_find_compress(uf, uf->parent[v]);
  uf->parent[v] = root;
  return root;
}

void uf_union(UnionFind* uf, size_t a, size_t b) {
  size_t root_a = uf_find_compress(uf, a);
  uf_find_replace(uf, b, root_a);
}

bool uf_connected(UnionFind* uf, size_t a, size_t b) {
  return uf_find_compress(uf, a) == uf_find_compress(uf, b);
}

void uf_destroy(UnionFind* uf) {
  free(uf->parent);
  free(uf);
}
