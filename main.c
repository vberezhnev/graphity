#include "graph.h"
#include "lib/graphics.c"
#include <stdio.h>
#include "algorithms.h"

void draw_graph(void) {
  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    Edge e = graph.edges[i];
    Node n0 = graph.nodes[e.src];
    Node n1 = graph.nodes[e.dst];

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
    Node n = graph.nodes[i];
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
  f64 drag_x0 = 0;
  f64 drag_y0 = 0;

  f64 offset_x = 0;
  f64 offset_y = 0;

  {
    FILE *n = fopen("coords-write.txt", "rb");

    i32 num_nodes = readInt(n);

    for (i64 i = 0; i < num_nodes; ++i) {
      f64 x = readInt(n);
      f64 y = readInt(n);

      add_node(x, y);
    };

    i32 num_edges = readInt(n);

    for (i64 i = 0; i < num_edges; ++i) {
      i32 src = readInt(n);
      i32 dst = readInt(n);

      add_edge(src, dst);
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
        if (graph.nodes[i].enabled && graph.nodes[i].hover) {
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
        if (graph.nodes[i].enabled && graph.nodes[i].hover) {
          drag_node_index = i;
          dragging = 1;
          node_found = 1;

          drag_x0 = platform.cursor_x;
          drag_y0 = platform.cursor_y;

          for (i64 j = 0; j < MAX_NUM_NODES; ++j) {
            graph.nodes[j].drag_x = graph.nodes[j].x;
            graph.nodes[j].drag_y = graph.nodes[j].y;
          }

          /* offset_x = nodes[i].x - platform.cursor_x; */
          /* offset_y = nodes[i].y - platform.cursor_y; */
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
      f64 dx = platform.cursor_x - drag_x0;
      f64 dy = platform.cursor_y - drag_y0;

      graph.nodes[drag_node_index].x = graph.nodes[drag_node_index].drag_x + dx;
      graph.nodes[drag_node_index].y = graph.nodes[drag_node_index].drag_y + dy;

      for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
        b8 is_neighbor = 0;

        if (i == drag_node_index)
          continue;

        for (i64 j = 0; j < MAX_NUM_EDGES; ++j) {
          Edge e = graph.edges[j];
          if ((e.src == drag_node_index && e.dst == i) ||
              (e.src == i && e.dst == drag_node_index)) {
            is_neighbor = 1;
            break;
          }
        }

        if (is_neighbor) {
          graph.nodes[i].x = graph.nodes[i].drag_x + dx * .4;
          graph.nodes[i].y = graph.nodes[i].drag_y + dy * .4;
        }
      }

      /* if (!overlap) { */
      /* graph.nodes[drag_node_index].x = platform.cursor_x + offset_x; */
      /* graph.nodes[drag_node_index].y = platform.cursor_y + offset_y; */
      /* } */
    }

    if (platform.key_pressed[KEY_DELETE]) {
      remove_node();
      remove_edge();
    }

    for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
      if (graph.nodes[i].enabled) {
        update_node(i);
        if (graph.nodes[i].hover)
          hover_node = 1;
      }
    }

    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (hover_node) {
        graph.edges[i].hover = 0;
      } else {
        update_edge(i);
      }
    }

    if (platform.key_pressed[BUTTON_RIGHT])
      for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
        if (graph.nodes[i].enabled && graph.nodes[i].hover) {
          adding_edge = 1;
          adding_src = i;
          adding_dst = i;
          break;
        }
      }

    if (adding_edge) {
      f64 x0 = graph.nodes[adding_src].x;
      f64 y0 = graph.nodes[adding_src].y;
      f64 x1 = platform.cursor_x;
      f64 y1 = platform.cursor_y;

      fill_line(OP_SET, 0x7f007f, x0, y0, x1, y1, 30);
    }

    if (adding_edge)
      for (i64 i = 0; i < MAX_NUM_NODES; ++i)
        if (graph.nodes[i].enabled && graph.nodes[i].hover) {
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
        if (graph.nodes[i].enabled && graph.nodes[i].hover) {
          path_src = i;
          path_changed = 1;
          break;
        }
    }

    if (platform.key_pressed['2'])
      for (i64 i = 0; i < MAX_NUM_NODES; ++i)
        if (graph.nodes[i].enabled && graph.nodes[i].hover) {
          path_dst = i;
          path_changed = 1;
          break;
        }

    if (path_changed) {
      highlight_path(&graph, path_src, path_dst);
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
      if (graph.nodes[i].enabled) {
        writeInt(n, (i32)graph.nodes[i].x);
        writeInt(n, (i32)graph.nodes[i].y);
      }
    };

    writeInt(n, edges_count());

    for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
      if (graph.edges[i].enabled) {
        writeInt(n, graph.edges[i].src);
        writeInt(n, graph.edges[i].dst);
      }
    };

    fclose(n);
  }

  return 0;
}
