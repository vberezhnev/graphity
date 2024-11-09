#ifndef UTILS_H
#define UTILS_H

#include "node.h"
#include "edge.h"

f64 get_distance(f64 x0, f64 y0, f64 x1, f64 y1) {
  f64 dx = x1 - x0;
  f64 dy = y1 - y0;
  return sqrt(dx * dx + dy * dy);
}

void clear_node_edge_highlight() {
  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    nodes[i].highlight = 0;
    nodes[i].distance = -1;
  }

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    edges[i].highlight = 0;
  }
}

#endif
