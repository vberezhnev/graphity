#ifndef GRAPH_H
#define GRAPH_H

#include "lib/graphics.c"
#include <math.h>
#include <stdio.h>

enum {
  MAX_NUM_NODES = 1024,
  MAX_NUM_EDGES = 1024,
  MAX_PATH_SIZE = 1024,
};

typedef struct {
  b8 enabled;
  i64 src;
  i64 dst;
  f64 width;
  b8 hover;
  b8 highlight;
} Edge;

typedef struct {
  b8 enabled;
  f64 radius;
  f64 x;
  f64 y;
  b8 hover;
  b8 highlight;
  f64 distance;
  f64 weight;

  f64 drag_x;
  f64 drag_y;
} Node;

typedef struct {
  Node nodes[MAX_NUM_NODES];
  Edge edges[MAX_NUM_EDGES];
  i64 path[MAX_PATH_SIZE];
  i64 path_size;
} Graph;

Graph graph = {0};

void add_node(f64 x, f64 y) {
  for (i64 j = 0; j < MAX_NUM_NODES; ++j) {
    Node n2 = graph.nodes[j];
    if (!n2.enabled) {
      continue;
    }

    if (fabs(x - n2.x) < 50 + n2.radius && fabs(y - n2.y) < 50 + n2.radius) {
      printf("Error: Cannot add node, overlapping nodes detected.\n");
      return;
    }
  }

  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    if (!graph.nodes[i].enabled) {
      graph.nodes[i] = (Node){
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

/*********/
/* EDGES */
/*********/

void add_edge(i64 src, i64 dst) {
  assert(src >= 0 && src < MAX_NUM_NODES);
  assert(dst >= 0 && dst < MAX_NUM_NODES);

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    if (!graph.edges[i].enabled) {
      graph.edges[i] = (Edge){
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
    Node n = graph.nodes[i];

    if (n.enabled && n.hover) {
      graph.nodes[i].enabled = 0;

      for (i64 j = 0; j < MAX_NUM_EDGES; ++j) {
        Edge e = graph.edges[j];

        if (e.src == i || e.dst == i) {
          graph.edges[j].enabled = 0;
        }
      }

      return;
    }
  }
}

void remove_edge() {
  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    Edge e = graph.edges[i];

    if (e.enabled && e.hover) {
      graph.edges[i].enabled = 0;

      return;
    }
  }
}

void update_edge(i64 edge_index) {
  assert(edge_index >= 0 && edge_index < MAX_NUM_EDGES);
  /* assert(validate_node(graph.edges[edge_index].src)); */
  /* assert(validate_node(graph.edges[edge_index].dst)); */

  f64 x = platform.cursor_x;
  f64 y = platform.cursor_y;

  Edge e = graph.edges[edge_index];
  Node n0 = graph.nodes[e.src];
  Node n1 = graph.nodes[e.dst];

  graph.edges[edge_index].hover =
      line_contains(n0.x, n0.y, n1.x, n1.y, e.width, x, y);
}

i32 edges_count() {
  i32 count = 0;

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    if (graph.edges[i].enabled)
      ++count;
  }

  return count;
}

/*********/
/* NODES */
/*********/

void update_node(i64 node_index) {
	assert(node_index >= 0 && node_index < MAX_NUM_NODES);
	
  f64 x = platform.cursor_x;
  f64 y = platform.cursor_y;

  Node n = graph.nodes[node_index];

  graph.nodes[node_index].hover =
      ellipse_contains(n.x - n.radius, n.y - n.radius, n.radius * 2,
                       n.radius * 2, platform.cursor_x, platform.cursor_y

      );
}

i32 nodes_count() {
  i32 count = 0;

  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    if (graph.nodes[i].enabled)
      ++count;
  }

  return count;
}


#endif
