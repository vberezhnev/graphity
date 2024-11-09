#include "lib/graphics.c"
#include "graph.h"
#include <stdio.h>

void draw_graph(void) {
  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    Edge e = edges[i];
    Node n0 = nodes[e.src];
    Node n1 = nodes[e.dst];

    u32 color = 0x7f7f7f; // grey color

    if (e.highlight)
      color = 0xff00ff; // pink color
    if (e.hover)
      color = 0x007f00; // green color

    // FIXME: color of line on node
    if (e.enabled)
      fill_line(OP_SET, color, n0.x, n0.y, n1.x, n1.y, e.width);
  }

  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    Node n = nodes[i];
    u32 color = 0x7f7f7f; // grey color

    if (n.highlight)
      color = 0xfff0ff; // no name color
    if (n.hover)
      color = 0x007f00; // green color

    if (n.enabled)
      fill_ellipse(OP_SET, color, n.x - n.radius, n.y - n.radius, n.radius * 2,
                   n.radius * 2);
  }
}

void highlight_path(i64 src, i64 dst) {
  if (!validate_node(src) || !validate_node(dst)) {
    printf("Invalid source or destination node index.\n");
    return;
  }

  clear_node_edge_highlight();

  // Set initial conditions
  nodes[src].highlight = 1;
  nodes[dst].highlight = 1;
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
    nodes[next_node_idx].highlight = 1;
    edges[edge_to_highlight].highlight = 1;

    // Move to the next node in the path
    curr_node_idx = next_node_idx;
  }
}

i32 readInt(FILE *f) {
  i32 x;
  fscanf(f, "%d", &x);

  return x;
}

void writeInt(FILE *f, i32 value) { fprintf(f, "%d ", value); }

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

  b8 dragging = 0;
  i64 drag_node_index = -1;

  f64 offset_x = 0;
  f64 offset_y = 0;

  {
    FILE *n = fopen("coords-write.txt", "rb");

    i32 num_nodes = readInt(n);

    for (i64 i = 0; i < num_nodes; ++i) {
      add_node(readInt(n), readInt(n));
    };

    i32 num_edges = readInt(n);

    for (i64 i = 0; i < num_edges; ++i) {
      add_edge(readInt(n), readInt(n));
    };

    fclose(n);
  }

  while (!platform.done) {
    p_wait_events();

    b8 hover_node = 0;

    fill_rectangle(OP_SET, 0xffffff, 0, 0, platform.frame_width,
                   platform.frame_height);

    if (platform.key_pressed[BUTTON_RIGHT])
      for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
        if (nodes[i].enabled && nodes[i].hover) {
          adding_edge = 1;
          adding_src = i;
          adding_dst = i;
          break;
        }
      }

    if (platform.key_pressed[BUTTON_LEFT]) {
      f64 x = platform.cursor_x;
      f64 y = platform.cursor_y;
      b8 node_found = 0;

      for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
        if (nodes[i].enabled && nodes[i].hover) {
          drag_node_index = i;
          dragging = 1;
          node_found = 1;

          offset_x = nodes[i].x - platform.cursor_x;
          offset_y = nodes[i].y - platform.cursor_y;
        }
      }

      if (!node_found)
        add_node(x, y);
      path_changed = 1;
    }

    if (!platform.key_down[BUTTON_LEFT]) {
      dragging = 0;
      path_changed = 1;
      drag_node_index = -1;
    }

    if (dragging) {
      b8 overlap = 0;

      for (i64 j = 0; j < MAX_NUM_NODES; ++j) {
        if (j == drag_node_index)
          continue;

        Node n2 = nodes[j];
        if (!n2.enabled) {
          continue;
        }

        if (fabs(platform.cursor_x + offset_x - n2.x) < 50 + n2.radius &&
            fabs(platform.cursor_y + offset_y - n2.y) < 50 + n2.radius) {
          printf("Error: Cannot add node, overlapping nodes detected.\n");

          overlap = 1;
        }
      }

      if (!overlap) {
        nodes[drag_node_index].x = platform.cursor_x + offset_x;
        nodes[drag_node_index].y = platform.cursor_y + offset_y;
      }
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
      path_changed = 0; // FIXME: if enabled, highlight isn't showing
    }

    draw_graph();

    p_render_frame();
  }

  p_cleanup();

  {
    FILE *n = fopen("coords-write.txt", "wb");

    writeInt(n, nodes_count());

    for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
      if (nodes[i].enabled) {
        writeInt(n, (i32)nodes[i].x);
        writeInt(n, (i32)nodes[i].y);
      }
    };

    writeInt(n, edges_count());

    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (edges[i].enabled) {
        writeInt(n, edges[i].src);
        writeInt(n, edges[i].dst);
      }
    };

    fclose(n);
  }

  return 0;
}
