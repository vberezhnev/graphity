#ifndef GRAPH_H
#define GRAPH_H

#include "edge.h"
#include "node.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>

b8 validate_node(i64 node_idx) {
  return (node_idx >= 0) && (node_idx < MAX_NUM_NODES) &&
         nodes[node_idx].enabled;
}

b8 validate_edge(i64 edge_idx) {
  return (edge_idx >= 0) && (edge_idx < MAX_NUM_EDGES) &&
         edges[edge_idx].enabled && validate_node(edges[edge_idx].src) &&
         validate_node(edges[edge_idx].dst);
}

void add_node(f64 x, f64 y) {
  for (i64 j = 0; j < MAX_NUM_NODES; ++j) {
    Node n2 = nodes[j];
    if (!n2.enabled) {
      continue;
    }

    if (fabs(x - n2.x) < 50 + n2.radius && fabs(y - n2.y) < 50 + n2.radius) {
      printf("Error: Cannot add node, overlapping nodes detected.\n");
      return;
    }
  }

  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    if (!nodes[i].enabled) {
      nodes[i] = (Node){
          .enabled = 1,
          .x = x,
          .y = y,
          .radius = 50,
          .weight = 2,
      };

      return;
    }
  }
}

void add_edge(i64 src, i64 dst) {
  assert(src >= 0 && src < MAX_NUM_NODES);
  assert(dst >= 0 && dst < MAX_NUM_NODES);

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    if (!edges[i].enabled) {
      edges[i] = (Edge){
          .enabled = 1,
          .src = src,
          .dst = dst,
          .width = 35,
      };

      return;
    }
  }
}

void remove_node() {
  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    Node n = nodes[i];

    if (n.enabled && n.hover) {
      nodes[i].enabled = 0;

      for (i64 j = 0; j < MAX_NUM_EDGES; ++j) {
        Edge e = edges[j];

        if (e.src == i || e.dst == i) {
          edges[j].enabled = 0;
        }
      }

      return;
    }
  }
}

void remove_edge() {
  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    Edge e = edges[i];

    if (e.enabled && e.hover) {
      edges[i].enabled = 0;

      return;
    }
  }
}

void update_edge(i64 edge_index) {
  assert(edge_index >= 0 && edge_index < MAX_NUM_EDGES);
  /* assert(validate_node(edges[edge_index].src)); */
  /* assert(validate_node(edges[edge_index].dst)); */

  f64 x = platform.cursor_x;
  f64 y = platform.cursor_y;

  Edge e = edges[edge_index];
  Node n0 = nodes[e.src];
  Node n1 = nodes[e.dst];

  edges[edge_index].hover =
      line_contains(n0.x, n0.y, n1.x, n1.y, e.width, x, y);
}

#endif
