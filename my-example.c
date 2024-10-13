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
} Node;

typedef struct {
  b8 enabled;
  i64 src;
  i64 dst;
  f64 width;
  b8 hover;
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

      /* if (adding_src != adding_dst) { */
      /*   x1 = nodes[adding_dst].x; */
      /*   y1 = nodes[adding_dst].y; */
      /* } */

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

    draw_graph();

    p_render_frame();
  }

  p_cleanup();
  return 0;
}
