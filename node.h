#ifndef NODE_H
#define NODE_H

#include "lib/graphics.c"
#include "edge.h"

typedef struct {
  b8 enabled;
  f64 radius;
  f64 x;
  f64 y;
  b8 hover;
  b8 highlight;
  f64 distance;
  f64 weight;
} Node;

Node nodes[MAX_NUM_EDGES] = {0};

void update_node(i64 node_index) {
  f64 x = platform.cursor_x;
  f64 y = platform.cursor_y;

  Node n = nodes[node_index];

  nodes[node_index].hover =
      ellipse_contains(n.x - n.radius, n.y - n.radius, n.radius * 2,
                       n.radius * 2, platform.cursor_x, platform.cursor_y

      );
}

i32 nodes_count() {
  i32 count = 0;

  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    if (nodes[i].enabled)
      ++count;
  }

  return count;
}

#endif
