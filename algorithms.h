#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "graph.h"
#include "lib/graphics.c"
#include <math.h>
#include <stdio.h>

f64 get_distance(f64 x0, f64 y0, f64 x1, f64 y1) {
  f64 dx = x1 - x0;
  f64 dy = y1 - y0;
  return sqrt(dx * dx + dy * dy);
}

void clear_node_edge_highlight(Graph *graph) {
  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    graph->nodes[i].highlight = 0;
    graph->nodes[i].distance = -1;
  }

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    graph->edges[i].highlight = 0;
  }
}

b8 validate_node(Graph *graph, i64 node_idx) {
  return (node_idx >= 0) && (node_idx < MAX_NUM_NODES) &&
         graph->nodes[node_idx].enabled;
}

b8 validate_edge(Graph *graph, i64 edge_idx) {
  return (edge_idx >= 0) && (edge_idx < MAX_NUM_EDGES) &&
         graph->edges[edge_idx].enabled &&
         validate_node(graph, graph->edges[edge_idx].src) &&
         validate_node(graph, graph->edges[edge_idx].dst);
}

void highlight_path(Graph *graph, i64 src, i64 dst) {
  clear_node_edge_highlight(graph);

  if (!validate_node(graph, src) || !validate_node(graph, dst)) {
    printf("Invalid source or destination node index.\n");
    return;
  }

  // Set initial conditions
  graph->nodes[src].highlight = 1;
  graph->nodes[dst].highlight = 1;
  graph->nodes[src].distance = 0;

  // Finding the shortest path using a distance-driven search
  for (;;) {
    i64 nearest_node_idx = -1;
    f64 nearest_dist = -1;

    // Evaluate all edges to find the nearest unvisited node
    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (!validate_edge(graph, i))
        continue;

      Edge edge = graph->edges[i];
      Node node0 = graph->nodes[edge.src];
      Node node1 = graph->nodes[edge.dst];
      f64 length = get_distance(node0.x, node0.y, node1.x, node1.y);

      // Look for unvisited nodes on both ends of the edge
      if (node0.distance >= 0 && node1.distance < 0) {
        if (nearest_node_idx < 0 || node0.distance + length < nearest_dist) {
          nearest_node_idx = edge.dst;
          nearest_dist = node0.distance + length;
        }
      }
      if (node1.distance >= 0 && node0.distance < 0) {
        if (nearest_node_idx < 0 || node1.distance + length < nearest_dist) {
          nearest_node_idx = edge.src;
          nearest_dist = node1.distance + length;
        }
      }
    }

    // If no further nodes can be found, exit the loop
    if (nearest_node_idx < 0)
      break;

    graph->nodes[nearest_node_idx].distance = nearest_dist;

    // Stop if we've reached the destination
    if (nearest_node_idx == dst)
      break;
  }

  // If no path was found, alert the user
  if (graph->nodes[dst].distance < 0) {
    printf("Path not found\n");
    return;
  }

  // Backtrack from destination to reconstruct the path
  i64 curr_node_idx = dst;

  while (curr_node_idx != src) {
    f64 dist = -1;
    i64 next_node_idx = -1;

    // Check all edges to find the next node in the path
    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (!validate_edge(graph, i))
        continue;

      Edge edge = graph->edges[i];
      Node node0 = graph->nodes[edge.src];
      Node node1 = graph->nodes[edge.dst];

      f64 length = get_distance(node0.x, node0.y, node1.x, node1.y);

      // Check for the nearest node along the current path
      if (edge.src == curr_node_idx && node1.distance >= 0 &&
          (dist < 0 || node1.distance + length < dist)) {
        next_node_idx = edge.dst;

        assert(graph->path_size < MAX_PATH_SIZE);

        graph->path[graph->path_size] = i;
        graph->path_size++;

        dist = node1.distance + length;
      }
      if (edge.dst == curr_node_idx && node0.distance >= 0 &&
          (dist < 0 || node0.distance + length < dist)) {
        next_node_idx = edge.src;

        assert(graph->path_size < MAX_PATH_SIZE);

        graph->path[graph->path_size] = i;
        graph->path_size++;

        dist = node0.distance + length;
      }
    }

    // If no next node is found, there's an error
    if (next_node_idx < 0) {
      printf("Internal error\n");
      break;
    }

    // Highlight the current node and edge as part of the path
    graph->nodes[next_node_idx].highlight = 1;
    /* graph->edges[edge_to_highlight].highlight = 1; */

    if (curr_node_idx == next_node_idx) {
      printf("Internal error\n");
      break;
    }

    // Move to the next node in the path
    curr_node_idx = next_node_idx;
    printf("%lld\n", curr_node_idx);
  }

  for (i64 i = 1; i < graph->path_size; ++i) {
    graph->edges[graph->path[i]].highlight = 1;
  }
}

#endif
