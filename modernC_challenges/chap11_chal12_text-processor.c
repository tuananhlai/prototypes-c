#include "../arena.h"
#include <stdio.h>
#include <stdlib.h>

struct Node {
  const char *val;
  size_t len;
  struct Node *prev;
  struct Node *next;
};
typedef struct Node Node;

/** `val` is BORROWED. */
static Node *node_create(size_t len, const char val[len], Node *prev,
                         Node *next) {
  Node *retval = malloc(sizeof(Node));
  retval->val = val;
  retval->len = len;
  retval->prev = prev;
  retval->next = next;
  if (retval->prev) {
    retval->prev->next = retval;
  }
  if (retval->next) {
    retval->next->prev = retval;
  }
  return retval;
}

static void node_destroy(Node* node) {
  free(node);
}

typedef struct {
  Node *head;
  Arena *arena;
} Text;

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
int main(void) { return EXIT_SUCCESS; }