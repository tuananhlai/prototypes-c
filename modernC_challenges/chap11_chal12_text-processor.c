#include <assert.h>
#include <stddef.h>
#define ARENA_IMPLEMENTATION
#include "../arena.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Blob {
  const char *val;
  size_t len;
  struct Blob *prev;
  struct Blob *next;
};
typedef struct Blob Blob;

static void blob_init(Blob *blob, size_t len, const char val[len], Blob *prev,
                      Blob *next) {
  blob->val = val;
  blob->len = len;
  blob->prev = prev;
  blob->next = next;
  if (blob->prev) {
    blob->prev->next = blob;
  }
  if (blob->next) {
    blob->next->prev = blob;
  }
}

/** Split the given blob into two at `split_idx`. Write the right part to `out`.
 */
static void blob_split(Blob *blob, size_t split_idx, Blob *out) {
  if (split_idx >= blob->len)
    return;

  auto original_next = blob->next;
  blob_init(out, blob->len - split_idx, blob->val + split_idx, blob,
            original_next);
  blob->len = split_idx;
}

typedef struct {
  Blob *head;
  Blob *tail;
  Arena arena;
} Text;

Text *text_create() {
  Arena arena = {0};
  Text *retval = malloc(sizeof(Text));
  retval->head = nullptr;
  retval->tail = nullptr;
  retval->arena = arena;
  return retval;
}

void text_append(Text *text, const char *str) {
  size_t val_len = strlen(str);
  char *val = arena_alloc(&text->arena, sizeof(*val) * val_len);
  memcpy(val, str, sizeof(*val) * val_len);
  Blob *new_blob = arena_alloc(&text->arena, sizeof(*new_blob));
  blob_init(new_blob, val_len, val, text->tail, nullptr);

  text->tail = new_blob;
  if (text->head == nullptr) {
    text->head = new_blob;
  }
}

static Blob *text_blob_join(Text *text, const Blob *prev, const Blob *next) {
  if (prev->next != next) {
    return nullptr;
  }

  size_t merged_val_len = prev->len + next->len;
  char *merged_val = arena_alloc(&text->arena, merged_val_len);
  memcpy(merged_val, prev->val, prev->len);
  memcpy(merged_val + prev->len, next->val, next->len);

  Blob *merged_blob = arena_alloc(&text->arena, sizeof(*merged_blob));
  blob_init(merged_blob, merged_val_len, merged_val, prev->prev, next->next);
  return merged_blob;
}

static void text_split_lines(Text *text) {
  for (auto x = text->head; x; x = x->next) {
    const char *newline;
    while (true) {
      newline = memchr(x->val, '\n', x->len);
      if (newline != nullptr || x->next == nullptr)
        break;

      x = text_blob_join(text, x, x->next);
    }

    auto idx = newline - x->val;
    assert(idx >= 0);
    if ((size_t)idx == x->len - 1)
      continue;

    Blob *new_blob = arena_alloc(&text->arena, sizeof(*new_blob));
    blob_split(x, idx + 1, new_blob);
  }
}

void text_print(Text *text) {
  size_t i = 0;
  for (auto x = text->head; x; x = x->next) {
    printf("#%zu ", i++);
    fwrite(x->val, sizeof(char), x->len, stdout);
  }
}

void text_destroy(Text *text) {
  arena_free(&text->arena);
  free(text);
}

/**
Challenge 12 (text processor). For a text processor, can you use a doubly linked
list to store text? The idea is to represent a “blob” of text through a struct
that contains a string (for the text) and pointers to preceding and following
blobs.

Can you build a function that splits a text blob in two at a given point?

One that joins two consecutive text blobs?

One that runs through the entire text and puts it in the form of one blob per
line?

Can you create a function that prints the entire text or prints until the
text is cut off due to the screen size?
*/
int main(void) {
  Text *text = text_create();

  text_append(text, "hel\nlo");
  text_append(text, ",");
  text_append(text, "world!\n");

  text_split_lines(text);

  text_print(text);

  text_destroy(text);
  return EXIT_SUCCESS;
}