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

    f64 cx = platform.cursor_x;
    f64 cy = platform.cursor_y;

    if (n.enabled)
      if (cx >= n.x - n.radius && cx <= n.x + n.radius &&
          cy >= n.y - n.radius && cy <= n.y + n.radius) {
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

void draw_edge() {}

void draw_graph(void) {
  for (i64 i = 0; i < MAX_NUM_NODES; ++i) {
    Node n = nodes[i];

    if (n.enabled)
      fill_ellipse(OP_SET, 0, n.x - n.radius, n.y - n.radius, n.radius * 2,
                   n.radius * 2);
  }

  for (i64 i = 0; i < MAX_NUM_EDGES; ++i) {
    Edge e = edges[i];
    Node n0 = nodes[e.src];
    Node n1 = nodes[e.dst];

    // FIXME: color of line on node
    if (e.enabled)
      fill_line(OP_SET, 0x003f00, n0.x, n0.y, n1.x, n1.y, e.width);
  }
}

i32 main() {
  p_init();

  add_node(50, 50);
  add_edge(0, 1);
  add_node(150, 150);

  while (!platform.done) {
    p_wait_events();

    fill_rectangle(OP_SET, 0xffffff, 0, 0, platform.frame_width,
                   platform.frame_height);
    if (platform.key_pressed[BUTTON_LEFT]) {
      f64 x = platform.cursor_x;
      f64 y = platform.cursor_y;

      add_node(x, y);
    }

    if (platform.key_pressed[KEY_DELETE]) {
      f64 x = platform.cursor_x;
      f64 y = platform.cursor_y;

      remove_node();
    }

    /* if (platform.key_pressed[BUTTON_RIGHT] && platform.key_pressed[MOD_CTRL])
     * { */
    /* } */

    draw_graph();

    p_render_frame();
  }

  p_cleanup();
  return 0;
}
