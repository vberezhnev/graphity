#ifndef EDGE_H
#define EDGE_H

#include "lib/graphics.c"

enum {
  MAX_NUM_NODES = 1024,
  MAX_NUM_EDGES = 1024,
};

typedef struct {
  b8 enabled;
  i64 src;
  i64 dst;
  f64 width;
  b8 hover;
  b8 highlight;
} Edge;

Edge edges[MAX_NUM_EDGES] = {0};

i32 edges_count() {
  i32 count = 0;

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    if (edges[i].enabled)
      ++count;
  }

  return count;
}

#endif
