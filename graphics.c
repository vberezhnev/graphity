//  ================================================================
//
//    graphics.c
//
//  ----------------------------------------------------------------
//
//  TODO:
//  - Blending.
//  - Anti-aliasing.
//
//  ================================================================

#ifndef GRAPHICS_HEADER_GUARD_
#define GRAPHICS_HEADER_GUARD_

#ifdef GRAPHICS_HEADER
#define REDUCED_SYSTEM_LAYER_HEADER
#endif

#include "reduced_system_layer.c"

#define EPSILON 1e-9

enum {
  OP_SET,
  OP_XOR,
};

u32  u32_from_rgb         (f32 red, f32 green, f32 blue);
void fill_rectangle       (u32 op, u32 color, f64 x0, f64 y0, f64 width, f64 height);
void fill_triangle        (u32 op, u32 color, f64 x0, f64 y0, f64 x1, f64 y1, f64 x2, f64 y2);
void fill_ellipse         (u32 op, u32 color, f64 x0, f64 y0, f64 width, f64 height);
void fill_line            (u32 op, u32 color, f64 x0, f64 y0, f64 x1, f64 y1, f64 width);
void draw_text_area       (u32 color, f64 x0, f64 y0, f64 width, f64 height, f64 max_scale_x, f64 max_scale_y, i64 num_chars, c32 *text);
void draw_selection_cursor(u32 color, f64 x0, f64 y0, f64 width, f64 height, f64 max_scale_x, f64 max_scale_y, i64 cursor, i64 selection, i64 num_chars, c32 *text);

#endif // GRAPHICS_HEADER_GUARD_

//  ================================================================

#ifndef GRAPHICS_HEADER
#ifndef GRAPHICS_IMPL_GUARD_
#define GRAPHICS_IMPL_GUARD_

#include <math.h>

f64 min3(f64 a, f64 b, f64 c) {
  if (a < b && a < c)
    return a;
  if (b < c)
    return b;
  return c;
}

f64 max3(f64 a, f64 b, f64 c) {
  if (a > b && a > c)
    return a;
  if (b > c)
    return b;
  return c;
}

b8 same_sign(f64 a, f64 b) {
  if (a >=  EPSILON && b <= -EPSILON) return 0;
  if (a <= -EPSILON && b >=  EPSILON) return 0;
  return 1;
}

u32 u32_from_rgb(f32 red, f32 green, f32 blue) {
  i32 r = (i32) floor(red   * 255.f);
  i32 g = (i32) floor(green * 255.f);
  i32 b = (i32) floor(blue  * 255.f);

  if (r <   0) r = 0;
  if (r > 255) r = 255;
  if (g <   0) g = 0;
  if (g > 255) g = 255;
  if (b <   0) b = 0;
  if (b > 255) b = 255;

  return (r << 16) | (g << 8) | b;
}

u64 bitfont[] = {
  0xbc0000000000, 0xc00300000, 0x5fd5040093f24fc9, 0xa00a2c2a1a280105, 0xc000415e6f, 0x400000020be0000, 0x1c38a8400000007d, 0x40002043e1020215, 0x408102000000010, 0x9800000000020002, 0xf913e00000033, 0x53200000207c8800, 0x3654880000099, 0x54b800000f840e00, 0xe953c000001a, 0x953e000000674080, 0x1e54b800000f, 0x490000000000240, 0x88a08000000, 0x20a220050a142850, 0x6520800000, 0x912f801eab260be, 0x800034952bf0001f, 0xc850bf0000921427, 0xf00010a54afc0003, 0xd29427800002142b, 0x840007e1023f0000, 0x7d09100000217e, 0x3f000188a08fc000, 0xc30c0cfc00000810, 0x27803f101013f00f, 0xc244bf0000f214, 0x4bf0002f21427800, 0xc254a480006c24, 0x407c00102fc08100, 0xf208080f0000fa0, 0x531007d81c607c0, 0xc208288c031141, 0x83fc00046954b10, 0x180e03000000, 0x41040000000ff04, 0x8102040810000404, 0x2a54600000000101, 0x309123e0000e, 0xc912180000a22447, 0x8000062a54700007, 0xe52a4300000029f0, 0xa0000602043e0001, 0x1d48000002074, 0x1f000003610f8000, 0x13e04f800000010, 0x470000780813e00f, 0x184893e0000e224, 0x23e0001f12243000, 0x82a54100000008, 0x40780000009f0200, 0xe208080e0001f20, 0xa22007981860780, 0x82082888022282, 0x16c200004ca95320, 0x7f000004, 0x408200000086d04, 0x8204,
};

#define CHAR_NUM_BITS_X 6
#define CHAR_NUM_BITS_Y 7
#define CHAR_NUM_BITS   (CHAR_NUM_BITS_X * CHAR_NUM_BITS_Y)

#define BITFONT_LEN ((i64) (sizeof bitfont / sizeof *bitfont))

i64 char_column_offset(c32 c, i64 column_index) {
  if (column_index < 0 || column_index >= CHAR_NUM_BITS_X)
    return -1;
  return (c - 32) * CHAR_NUM_BITS + column_index * CHAR_NUM_BITS_Y;
}

b8 char_bit(i64 column_offset, i64 row_index) {
  if (column_offset < 0 || column_offset / 64 >= BITFONT_LEN || row_index < 0 || row_index >= CHAR_NUM_BITS_Y)
    return 0;

  i64 bit_index   = column_offset + row_index;
  i64 qword_index = bit_index / 64;
  if (qword_index < 0 || qword_index >= BITFONT_LEN)
    return 0;
  u64 mask        = 1ull << (bit_index % 64);
  return !!(bitfont[qword_index] & mask);
}

u64 char_column_convolved(c32 c, i64 column_index) {
  if (column_index < 0 || column_index >= CHAR_NUM_BITS_X)
    return 0;

  u64 column = 0;
  i64 offset = char_column_offset(c, column_index);

  for (i64 y = 0; y < CHAR_NUM_BITS_Y; ++y)
    if (char_bit(offset, y))
      column |= 3ull << y;

  return column;
}

b8 char_column_empty(c32 c, i64 column_index) {
  if (column_index < 0 || column_index >= CHAR_NUM_BITS_X)
    return 1;

  i64 offset = char_column_offset(c, column_index);

  for (i64 y = 0; y < CHAR_NUM_BITS_Y; ++y)
    if (char_bit(offset, y))
      return 0;

  return 1;
}

i64 char_width(c32 c) {
  if (c < 32)
    return 0;
  if (c == ' ' || c > 127)
    return 4;

  i64 width = 0;

  for (; width < CHAR_NUM_BITS_X; ++width)
    if (char_column_empty(c, width) && char_column_empty(c, width + 1))
      break;

  return width;
}

i64 char_spacing(i64 num_chars, c32 *text, i64 index) {
  assert(text != NULL);

  if (index < 0 || index + 1 >= num_chars)
    return 0;

  u64 a = char_column_convolved(text[index], char_width(text[index]) - 1);
  u64 b = char_column_convolved(text[index + 1], 0);

  if (!!(a & b))
    return 1;

  return 0;
}

i64 text_cursor(i64 num_chars, c32 *text) {
  assert(text != NULL);

  i64 cursor = 0;

  for (i64 i = 0; i < num_chars; ++i) {
    if (text[i] <= ' ') {
      if (text[i] == '\n')
        cursor = 0;
      else if (text[i] == '\b' && i > 0)
        cursor -= char_width(text[i - 1]) + char_spacing(num_chars, text, i - 1);
      else if (text[i] == '\r')
        cursor = 0;
      else
        cursor += char_width(' ') + char_spacing(num_chars, text, i);
      continue;
    }
    cursor += char_width(text[i]) + char_spacing(num_chars, text, i);
  }

  return cursor;
}

i64 enum_text_columns(i64 num_chars, c32 *text) {
  assert(text != NULL);

  i64 cols = 0;
  i64 n    = 0;

  for (i64 i = 0; i < num_chars; ++i) {
    if (text[i] <= ' ') {
      if (text[i] == '\n') {
        if (cols < n)
          cols = n;
        n = 0;
      } else if (text[i] == '\b' && i > 0) {
        if (cols < n)
          cols = n;
        n -= char_width(text[i - 1]) + char_spacing(num_chars, text, i - 1);
      } else if (text[i] == '\r') {
        if (cols < n)
          cols = n;
        n = 0;
      } else
        n += char_width(' ') + char_spacing(num_chars, text, i);
      continue;
    }
    n += char_width(text[i]) + char_spacing(num_chars, text, i);
  }

  if (cols < n)
    cols = n;

  return cols;
}

i64 enum_text_rows(i64 num_chars, c32 *text) {
  assert(text != NULL);

  i64 rows = 0;

  for (i64 i = 0; i <= num_chars; ++i)
    if (i == num_chars || text[i] == '\n') {
      if (rows > 0)
        ++rows;
      rows += CHAR_NUM_BITS_Y;
    }

  return rows;
}

void draw_text(u32 color, f64 x0, f64 y0, f64 scale_x, f64 scale_y, i64 num_chars, c32 *text) {
  assert(text != NULL);

  f64 x = x0;
  f64 y = y0;

  f64 kx = scale_x;
  f64 h  = scale_y * CHAR_NUM_BITS_Y;

  for (i64 n = 0; n < num_chars; ++n) {
    if (text[n] <= ' ') {
      if (text[n] == '\n') {
        x  = x0;
        y += scale_y * (CHAR_NUM_BITS_Y + 1);
      }
      else if (text[n] == '\b' && n > 0)
        x -= kx * (char_width(text[n - 1]) + char_spacing(num_chars, text, n - 1));
      else if (text[n] == '\r')
        x  = x0;
      else
        x += kx * (char_width(' ') + char_spacing(num_chars, text, n));
      continue;
    }

    i64 num_cols = char_width(text[n]);
    f64 w        = num_cols * kx;

    i64 i0 = (i64) floor(x + .5);
    i64 i1 = (i64) floor(x + w + .5);
    i64 j0 = (i64) floor(y + .5);
    i64 j1 = (i64) floor(y + h + .5);

    for (i64 i = i0; i < i1; ++i) {
      if (i < 0) continue;
      if (i >= platform.frame_width) break;

      i64 column = ((i - i0) * num_cols) / (i1 - i0);
      i64 offset = char_column_offset(text[n], column);

      for (i64 j = j0; j < j1; ++j) {
        if (j < 0) continue;
        if (j >= platform.frame_height) break;

        i64 row = ((j - j0) * CHAR_NUM_BITS_Y) / (j1 - j0);

        if (char_bit(offset, row))
          platform.pixels[j * platform.frame_width + i] = color;
      }
    }

    x += kx * (num_cols + char_spacing(num_chars, text, n));
  }
}

void put_pixel(i64 i, i64 j, u32 op, u32 color) {
  if (i < 0 || i >= platform.frame_width || j < 0 || j >= platform.frame_height)
    return;
  if (op == OP_XOR)
    platform.pixels[j * platform.frame_width + i] ^= color;
  else
    platform.pixels[j * platform.frame_width + i]  = color;
}

void fill_rectangle(u32 op, u32 color, f64 x0, f64 y0, f64 width, f64 height) {
  i64 i0 = (i64) floor(x0 + .5);
  i64 j0 = (i64) floor(y0 + .5);
  i64 i1 = (i64) floor(x0 + width + .5);
  i64 j1 = (i64) floor(y0 + height + .5);

  if (i0 < 0) i0 = 0;
  if (j0 < 0) j0 = 0;
  if (i1 >= platform.frame_width)  i1 = platform.frame_width  - 1;
  if (j1 >= platform.frame_height) j1 = platform.frame_height - 1;

  for (i64 j = j0; j < j1; ++j)
    for (i64 i = i0; i < i1; ++i)
      put_pixel(i, j, op, color);
}

void fill_triangle(u32 op, u32 color, f64 x0, f64 y0, f64 x1, f64 y1, f64 x2, f64 y2) {
  i64 min_x = (i64) floor(min3(x0, x1, x2));
  i64 min_y = (i64) floor(min3(y0, y1, y2));
  i64 max_x = (i64) ceil (max3(x0, x1, x2));
  i64 max_y = (i64) ceil (max3(y0, y1, y2));

  //  Z-components of cross-products
  //
  f64 z0 = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
  f64 z1 = (x2 - x1) * (y0 - y1) - (x0 - x1) * (y2 - y1);
  f64 z2 = (x0 - x2) * (y1 - y2) - (x1 - x2) * (y0 - y2);

  for (i64 j = min_y; j <= max_y; ++j)
    for (i64 i = min_x; i <= max_x; ++i) {
      f64 x = (f64) i;
      f64 y = (f64) j;

      //  Z-components of cross-products
      //
      f64 pz0 = (x - x0) * (y2 - y0) - (x2 - x0) * (y - y0);
      f64 pz1 = (x - x1) * (y0 - y1) - (x0 - x1) * (y - y1);
      f64 pz2 = (x - x2) * (y1 - y2) - (x1 - x2) * (y - y2);

      //  Check signs
      //
      if (!same_sign(z0, pz0)) continue;
      if (!same_sign(z1, pz1)) continue;
      if (!same_sign(z2, pz2)) continue;

      put_pixel(i, j, op, color);
    }  
}

void fill_ellipse(u32 op, u32 color, f64 x0, f64 y0, f64 width, f64 height) {
  //  FIXME PERF:
  //  Implement better algorithm.

  i64 i0 = (i64) floor(x0 + .5);
  i64 j0 = (i64) floor(y0 + .5);
  i64 i1 = (i64) floor(x0 + width + .5);
  i64 j1 = (i64) floor(y0 + height + .5);

  f64 dw = width  / 2;
  f64 dh = height / 2;

  if (dw < EPSILON || dh < EPSILON)
    return;

  f64 cx = x0 + dw;
  f64 cy = y0 + dh;
  f64 kx = 1. / dw;
  f64 ky = 1. / dh;

  for (i64 j = j0; j < j1; ++j) {
    if (j < 0 || j >= platform.frame_height)
      continue;
    f64 dy   = (((f64) j) - cy) * ky;
    f64 dydy = dy * dy;
    for (i64 i = i0; i < i1; ++i) {
      if (i < 0 || i >= platform.frame_width)
        continue;
      f64 dx = (((f64) i) - cx) * kx;
      if (dx * dx + dydy <= 1.0)
        put_pixel(i, j, op, color);
    }
  }
}

void fill_line(u32 op, u32 color, f64 x0, f64 y0, f64 x1, f64 y1, f64 width) {
  f64 dx = x1 - x0;
  f64 dy = y1 - y0;

  //  Tangent
  //
  f64 tx = -dy;
  f64 ty = dx;
  f64 tl = sqrt(tx * tx + ty * ty);
  if (tl >= EPSILON) {
    tx /= tl;
    ty /= tl;
  }
  tx *= width * .5;
  ty *= width * .5;

  fill_triangle(op, color, x0 - tx, y0 - ty, x0 + tx, y0 + ty, x1 + tx, y1 + ty);
  fill_triangle(op, color, x0 - tx, y0 - ty, x1 + tx, y1 + ty, x1 - tx, y1 - ty);
}

void draw_text_area(u32 color, f64 x0, f64 y0, f64 width, f64 height, f64 max_scale_x, f64 max_scale_y, i64 num_chars, c32 *text) {
  assert(max_scale_x > 1e-6);
  assert(max_scale_y > 1e-6);

  i64 num_columns = enum_text_columns(num_chars, text);
  i64 num_rows    = enum_text_rows(num_chars, text);

  f64 scale_x = width  / num_columns;
  f64 scale_y = height / num_rows;

  f64 kx = scale_x / max_scale_x;
  f64 ky = scale_y / max_scale_y;

  f64 k = kx < ky ? kx : ky;

  kx = k * max_scale_x;
  ky = k * max_scale_y;

  draw_text(color, x0, y0, kx, ky, num_chars, text);
}

void draw_selection_cursor(u32 color, f64 x0, f64 y0, f64 width, f64 height, f64 max_scale_x, f64 max_scale_y, i64 cursor, i64 selection, i64 num_chars, c32 *text) {
  assert(max_scale_x > 1e-6);
  assert(max_scale_y > 1e-6);

  i64 num_columns = enum_text_columns(num_chars, text);
  i64 num_rows    = enum_text_rows(num_chars, text);
  i64 cursor_x    = text_cursor(cursor, text);
  i64 cursor_y    = enum_text_rows(cursor, text);
  f64 scale_x = width  / num_columns;
  f64 scale_y = height / num_rows;

  f64 kx = scale_x / max_scale_x;
  f64 ky = scale_y / max_scale_y;

  f64 k = kx < ky ? kx : ky;

  kx = k * max_scale_x;
  ky = k * max_scale_y;

  if (selection != 0) {
    i64 selection_x, selection_y;

    if (selection > 0) {
      selection_x = text_cursor(cursor + selection, text);
      selection_y = enum_text_rows(cursor + selection, text);
    } else {
      selection_x = cursor_x;
      selection_y = cursor_y;
      cursor_x    = text_cursor(cursor + selection, text);
      cursor_y    = enum_text_rows(cursor + selection, text);
    }

    if (cursor_y == selection_y)
      fill_rectangle(OP_XOR, color,
        x0 + kx * cursor_x,
        y0 + ky * cursor_y - ky * (CHAR_NUM_BITS_Y + 1),
        kx * (selection_x - cursor_x),
        ky * (CHAR_NUM_BITS_Y + 1)
      );
    else {
      fill_rectangle(OP_XOR, color,
        x0 + kx * cursor_x,
        y0 + ky * cursor_y - ky * (CHAR_NUM_BITS_Y + 1),
        kx * (num_columns - cursor_x),
        ky * (CHAR_NUM_BITS_Y + 1)
      );
      for (i64 j = cursor_y + CHAR_NUM_BITS_Y + 1; j < selection_y; j += CHAR_NUM_BITS_Y + 1)
        fill_rectangle(OP_XOR, color,
          x0,
          y0 + ky * j - ky * (CHAR_NUM_BITS_Y + 1),
          kx * num_columns,
          ky * (CHAR_NUM_BITS_Y + 1)
        );
      fill_rectangle(OP_XOR, color,
        x0,
        y0 + ky * selection_y - ky * (CHAR_NUM_BITS_Y + 1),
        kx * selection_x,
        ky * (CHAR_NUM_BITS_Y + 1)
      );
    }
  } else
    fill_rectangle(OP_XOR, color, x0 + kx * cursor_x, y0 + ky * cursor_y - ky * CHAR_NUM_BITS_Y, kx * .5, ky * (CHAR_NUM_BITS_Y - 1));
}

#endif // GRAPHICS_IMPL_GUARD_
#endif // GRAPHICS_HEADER 
