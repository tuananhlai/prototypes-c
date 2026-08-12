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

int main() {
  UnionFind* uf = uf_create(10);
  printf("--- Initial State ---\n");
  printf("Are 0 and 1 connected? %s\n", uf_connected(uf, 0, 1) ? "Yes" : "No");

  printf("\n--- Performing Unions ---\n");
  printf("Union(0, 1)\n");
  uf_union(uf, 0, 1);
  printf("Union(1, 2)\n");
  uf_union(uf, 1, 2);
  
  printf("Are 0 and 2 connected? %s (Expected: Yes)\n", uf_connected(uf, 0, 2) ? "Yes" : "No");
  printf("Are 0 and 3 connected? %s (Expected: No)\n", uf_connected(uf, 0, 3) ? "Yes" : "No");

  printf("\n--- Merging Disjoint Sets ---\n");
  printf("Union(3, 4)\n");
  uf_union(uf, 3, 4);
  printf("Union(2, 4) - Merging the two groups\n");
  uf_union(uf, 2, 4);
  
  printf("Are 0 and 3 connected now? %s (Expected: Yes)\n", uf_connected(uf, 0, 3) ? "Yes" : "No");
  printf("Are 0 and 9 connected? %s (Expected: No)\n", uf_connected(uf, 0, 9) ? "Yes" : "No");


  uf_destroy(uf);
  return 0;
}
