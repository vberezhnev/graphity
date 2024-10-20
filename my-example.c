#include "graphics.c"

enum {
  MAX_NUM_EDGES = 1024,
  MAX_NUM_NODES = 1024,
};

typedef struct {
  b8 enabled;
  f64 radius;
  f64 x;
  f64 y;
  b8 hover;
  /* b8 highlight; */
  f64 distance;
  f64 weight;
} Node;

typedef struct {
  b8 enabled;
  i64 src;
  i64 dst;
  f64 width;
  b8 hover;
  /* b8 hightlight; */
} Edge;

Node nodes[MAX_NUM_EDGES] = {0};
Edge edges[MAX_NUM_EDGES] = {0};

void add_node(f64 x, f64 y) {
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

void draw_graph(void) {
  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    Edge e = edges[i];
    Node n0 = nodes[e.src];
    Node n1 = nodes[e.dst];

    // FIXME: color of line on node
    if (e.enabled)
      fill_line(OP_SET, e.hover ? 0x005f00 : 0, n0.x, n0.y, n1.x, n1.y,
                e.width);
  }

  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    Node n = nodes[i];

    if (n.enabled)
      fill_ellipse(OP_SET, n.hover ? 0x002f00 : 0, n.x - n.radius,
                   n.y - n.radius, n.radius * 2, n.radius * 2);
  }
}

void update_edge(i64 edge_index) {
  f64 x = platform.cursor_x;
  f64 y = platform.cursor_y;

  Edge e = edges[edge_index];
  Node n0 = nodes[e.src];
  Node n1 = nodes[e.dst];

  edges[edge_index].hover =
      line_contains(n0.x, n0.y, n1.x, n1.y, e.width, x, y);
}

void update_node(i64 node_index) {
  f64 x = platform.cursor_x;
  f64 y = platform.cursor_y;

  Node n = nodes[node_index];

  nodes[node_index].hover =
      ellipse_contains(n.x - n.radius, n.y - n.radius, n.radius * 2,
                       n.radius * 2, platform.cursor_x, platform.cursor_y

      );
}

f64 get_distance(f64 x0, f64 y0, f64 x1, f64 y1) {
  f64 dx = x1 - x0;
  f64 dy = y1 - y0;
  return sqrt(dx * dx + dy * dy);
}

b8 validate_node(i64 node_idx) {
  return (node_idx >= 0) && (node_idx < MAX_NUM_NODES) &&
         nodes[node_idx].enabled;
}

b8 validate_edge(i64 edge_idx) {
  return (edge_idx >= 0) && (edge_idx < MAX_NUM_EDGES) &&
         edges[edge_idx].enabled && validate_node(edges[edge_idx].src) &&
         validate_node(edges[edge_idx].dst);
}

void clear_node_edge_highlight() {
  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    nodes[i].hover = 0;
    nodes[i].distance = -1;
  }

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    edges[i].hover = 0;
  }
}

void highlight_path(i64 src, i64 dst) {
  clear_node_edge_highlight();

  // Set initial conditions
  nodes[src].hover = 1;
  nodes[dst].hover = 1;
  nodes[src].distance = 0;

  // Finding the shortest path using a distance-driven search
  for (;;) {
    i64 nearest_node_idx = -1;
    f64 nearest_dist = -1;

    // Evaluate all edges to find the nearest unvisited node
    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (!validate_edge(i))
        continue;

      Edge edge = edges[i];
      Node node0 = nodes[edge.src];
      Node node1 = nodes[edge.dst];
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

    nodes[nearest_node_idx].distance = nearest_dist;

    // Stop if we've reached the destination
    if (nearest_node_idx == dst)
      break;
  }

  // If no path was found, alert the user
  if (nodes[dst].distance < 0) {
    printf("Path not found\n");
    return;
  }

  // Backtrack from destination to reconstruct the path
  f64 dist = nodes[dst].distance;
  i64 curr_node_idx = dst;

  while (curr_node_idx != src) {
    i64 next_node_idx = -1;
    i64 edge_to_highlight = -1;

    // Check all edges to find the next node in the path
    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (!validate_edge(i))
        continue;

      Edge edge = edges[i];
      Node node0 = nodes[edge.src];
      Node node1 = nodes[edge.dst];

      // Check for the nearest node along the current path
      if (edge.src == curr_node_idx && node1.distance >= 0 &&
          node1.distance < dist) {
        next_node_idx = edge.dst;
        edge_to_highlight = i;
        dist = node1.distance;
      }
      if (edge.dst == curr_node_idx && node0.distance >= 0 &&
          node0.distance < dist) {
        next_node_idx = edge.src;
        edge_to_highlight = i;
        dist = node0.distance;
      }
    }

    // If no next node is found, there's an error
    if (next_node_idx < 0) {
      printf("Internal error\n");
      break;
    }

    // Highlight the current node and edge as part of the path
    nodes[next_node_idx].hover = 1;
    edges[edge_to_highlight].hover = 1;

    // Move to the next node in the path
    curr_node_idx = next_node_idx;
  }
}

i32 main() {
  platform = (Platform){
      .title = "Graph",
      .frame_width = 960,
      .frame_height = 720,
  };

  p_init();

  b8 adding_edge = 0;
  i64 adding_src = 0;
  i64 adding_dst = 0;

  b8 path_changed = 0;
  i64 path_src = -1;
  i64 path_dst = -1;

  add_node(100, 100);
  add_node(300, 100);
  add_node(120, 300);

  add_edge(0, 1);
  add_edge(0, 2);
  add_edge(1, 2);

  while (!platform.done) {
    p_wait_events();

    b8 hover_node = 0;

    fill_rectangle(OP_SET, 0xffffff, 0, 0, platform.frame_width,
                   platform.frame_height);
    if (platform.key_pressed[BUTTON_LEFT]) {
      f64 x = platform.cursor_x;
      f64 y = platform.cursor_y;

      add_node(x, y);
    }

    if (platform.key_pressed[KEY_DELETE]) {
      remove_node();
      remove_edge();
    }

    for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
      if (nodes[i].enabled) {
        update_node(i);
        if (nodes[i].hover)
          hover_node = 1;
      }
    }

    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (hover_node) {
        edges[i].hover = 0;
      } else {
        update_edge(i);
      }
    }

    if (platform.key_pressed[BUTTON_RIGHT])
      for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
        if (nodes[i].enabled && nodes[i].hover) {
          adding_edge = 1;
          adding_src = i;
          adding_dst = i;
          break;
        }
      }

    if (adding_edge) {
      f64 x0 = nodes[adding_src].x;
      f64 y0 = nodes[adding_src].y;
      f64 x1 = platform.cursor_x;
      f64 y1 = platform.cursor_y;

      fill_line(OP_SET, 0x7f007f, x0, y0, x1, y1, 30);
    }

    if (adding_edge)
      for (i64 i = 0; i < MAX_NUM_NODES; ++i)
        if (nodes[i].enabled && nodes[i].hover) {
          adding_dst = i;
          break;
        }

    if (adding_edge && !platform.key_down[BUTTON_RIGHT]) {
      adding_edge = 0;
      add_edge(adding_src, adding_dst);
    }

    // Finding shortest path //
    if (platform.key_pressed['1']) {
      for (i64 i = 0; i < MAX_NUM_NODES; ++i)
        if (nodes[i].enabled && nodes[i].hover) {
          path_src = i;
          path_changed = 1;
          break;
        }
    }

    if (platform.key_pressed['2'])
      for (i64 i = 0; i < MAX_NUM_NODES; ++i)
        if (nodes[i].enabled && nodes[i].hover) {
          path_dst = i;
          path_changed = 1;
          break;
        }

    if (path_changed) {
      highlight_path(path_src, path_dst);
      // path_changed = 0; // FIXME: if enabled, highlight isn't showing
    }

    draw_graph();

    p_render_frame();
  }

  p_cleanup();
  return 0;
}
