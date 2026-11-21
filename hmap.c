#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  ST_EMPTY = 0,
  ST_OCCUPIED = 1,
  ST_TOMBSTONE = 2,
} State;

typedef struct {
  const char *key;
  int value;
  State state;
} Entry;

typedef struct {
  Entry *entries;
  size_t cap;
} HashMap;

static size_t hash(const char *key) { return strlen(key); }
static size_t sh_entry_idx(HashMap *hm, const char *key) {
  return hash(key) % hm->cap;
}

HashMap *sh_create() { 
  HashMap* hm = malloc(sizeof(HashMap));
  hm->cap = 16;
  Entry* entries = calloc(hm->cap, sizeof(Entry));
  hm->entries = entries;
  return hm; 
}

void sh_set(HashMap *hm, const char *key, int val) {
  size_t entry_idx = sh_entry_idx(hm, key);
  size_t start_idx = entry_idx;

  while (hm->entries[entry_idx].state == ST_OCCUPIED &&
         strcmp(hm->entries[entry_idx].key, key) != 0) {
    entry_idx = (entry_idx + 1) % hm->cap;
    // TODO: panic instead?
    if (entry_idx == start_idx)
      return;
  }

  hm->entries[entry_idx] = (Entry){
      .key = key,
      .value = val,
      .state = ST_OCCUPIED,
  };
}

int *sh_get(HashMap *hm, const char *key) {
  size_t entry_idx = sh_entry_idx(hm, key);
  size_t start_idx = entry_idx;

  while (hm->entries[entry_idx].state == ST_EMPTY ||
         strcmp(hm->entries[entry_idx].key, key) != 0) {
    entry_idx = (entry_idx + 1) % hm->cap;
    if (entry_idx == start_idx)
      return nullptr;
  }
  if (hm->entries[entry_idx].state == ST_TOMBSTONE)
    return nullptr;
  return &hm->entries[entry_idx].value;
}

void sh_delete(HashMap *hm, const char *key) {
  size_t entry_idx = sh_entry_idx(hm, key);
  size_t start_idx = entry_idx;

  while (hm->entries[entry_idx].state != ST_EMPTY && strcmp(hm->entries[entry_idx].key, key) != 0) {
    entry_idx = (entry_idx + 1) % hm->cap;
    if (entry_idx == start_idx)
      return;
  }

  if (hm->entries[entry_idx].state == ST_EMPTY) return;
  hm->entries[entry_idx].state = ST_TOMBSTONE;
}
void sh_destroy(HashMap *hm) {
  free(hm->entries);
  free(hm);
}

int main(void) {
  HashMap *mp = sh_create();
  assert(mp != nullptr);

  sh_set(mp, "bar", 10);
  assert(sh_get(mp, "nonexistent") == nullptr);
  assert(*sh_get(mp, "bar") == 10);

  sh_delete(mp, "bar");
  assert(sh_get(mp, "bar") == nullptr);

  return EXIT_SUCCESS;
}