#if 0 /*
#/  ================================================================
#/
#/    reduced_system_layer.c
#/
#/  This is a reduced system layer.
#/  It allows you to create a window, draw graphics in it, handle
#/  input events, write samples to audio output, send and receive
#/  UDP packets.
#/
#/  ----------------------------------------------------------------
#/
#/  DESIGN PRINCIPLES
#/
#/  - Minimalistic feature set. For graphics, you have access to the
#/    pixel buffer, and that's it.
#/
#/  - No implicit control flow. No callbacks. You write your own
#/    main and call everything explicitly. But the number of things
#/    you have to call to do something is as little as possible.
#/
#/  - Optimized to use in a single source file.
#/    Installation process? Ctrl+C, Ctrl+V, done.
#/
#/  If you have an idea how to reduce the feature set further,
#/  let me know!
#/
#/  ----------------------------------------------------------------
#/
#/  To-Do list
#/
#/  - Examples
#/    - Conway's Game if Life
#/    - Julia Set
#/    - Labyrinth
#/    - Landscape
#/  - Features
#/    - Sound
#/    - Clipboard daemon
#/
#/  ALSA
#/  https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html
#/
#/  ----------------------------------------------------------------
#/
#/    (C) 2024 Mitya Selivanov <guattari.tech>, MIT License
#/
#/  ================================================================
#/
#/    Self-compilation shell script
#/
SRC=${0##*./}
BIN=${SRC%.*}
gcc                                         \
  -Wall -Wextra -Werror -pedantic           \
  -Wno-old-style-declaration                \
  -Wno-missing-braces                       \
  -Wno-unused-variable                      \
  -Wno-unused-but-set-variable              \
  -Wno-unused-parameter                     \
  -Wno-overlength-strings                   \
  -O3                                       \
  -fsanitize=undefined,address,leak -mshstk \
  -D REDUCED_SYSTEM_LAYER_EXAMPLE           \
  -lX11 -lm                                 \
  -o $BIN $SRC &&                           \
  ./$BIN $@ && rm $BIN
exit $? # */
#endif

//  ================================================================
//
//    Types
//
//  ================================================================

#ifndef TYPES_HEADER_GUARD_
#define TYPES_HEADER_GUARD_

typedef signed char        i8;
typedef signed short       i16;
typedef signed             i32;
typedef signed long long   i64;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned           u32;
typedef unsigned long long u64;
typedef char               c8;
typedef int                c32;
typedef signed char        b8;
typedef float              f32;
typedef double             f64;

#endif // TYPES_HEADER_GUARD_

//  ================================================================
//
//    Basic declarations
//
//  ================================================================

#ifndef REDUCED_SYSTEM_LAYER_HEADER_GUARD_
#define REDUCED_SYSTEM_LAYER_HEADER_GUARD_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//  ================================================================
//
//    PLATFORM API
//
//  ================================================================

enum {
  MAX_NUM_PIXELS        = 10 * 1024 * 1024,
  MAX_INPUT_SIZE        = 256,
  MAX_CLIPBOARD_SIZE    = 10 * 1024 * 1024,
  MAX_NUM_AUDIO_SAMPLES = 0,
  MAX_NUM_SOCKETS       = 64,

  AUDIO_NUM_CHANNELS = 2,
  AUDIO_SAMPLE_RATE  = 44100,

  IPv4_UDP = 1,
  IPv6_UDP = 2,

  KEY_LEFT = 128,
  KEY_RIGHT,
  KEY_UP,
  KEY_DOWN,
  KEY_LCTRL,
  KEY_RCTRL,
  KEY_LSHIFT,
  KEY_RSHIFT,
  KEY_LALT,
  KEY_RALT,
  KEY_ESCAPE,
  KEY_PRINTSCREEN,
  KEY_DELETE,
  KEY_PAUSE,
  KEY_INSERT,
  KEY_HOME,
  KEY_END,
  KEY_PAGEUP,
  KEY_PAGEDOWN,
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  MOD_CTRL,
  MOD_SHIFT,
  MOD_ALT,
  MOD_CAPS,
  MOD_NUM,
  MOD_SCROLL,
  KEY_F_,
  KEY_KP_ = KEY_F_ + 64,
};

typedef struct {
  b8  ctrl   : 1;
  b8  shift  : 1;
  b8  alt    : 1;
  b8  caps   : 1;
  b8  num    : 1;
  b8  scroll : 1;
  u16 key;
  c32 c;
} Input_Key;

typedef struct {
  c8  *      title;
  i32        frame_width;
  i32        frame_height;
  u32 *      pixels;
  i64        input_size;
  Input_Key *input;
  i64        clipboard_size;
  c8  *      clipboard;
  b8         done;
  b8         graceful_exit;
  b8         has_focus;
  b8         has_cursor;
  i32        cursor_x;
  i32        cursor_y;
  i32        cursor_dx;
  i32        cursor_dy;
  i64        wheel_dy;
  b8         key_down[512];
  b8         key_pressed[512];
} Platform;

typedef struct {
  u16 protocol;
  u16 port;
  union {
    u32 v4_address_as_u32;
    u8  v4_address[4];
    u8  v6_address[16];
  };
} IP_Address;

//  UTF-8
//  NOTE We need UTF-8 because we use Xutf8LookupString on Xlib.
i32 utf8_size(c32 c);
c32 utf8_read(i64 len, c8 *s);
i32 utf8_write(c32 c, c8 *buffer);

//  Time and sleep
i64  p_time(void);
void p_yield(void);
void p_sleep_for(i64 duration);

//  Window
void p_init(void);
void p_cleanup(void);
i32  p_handle_events(void);
i32  p_wait_events(void);
void p_render_frame(void);

//  Clipboard
void p_clipboard_write(i64 size, c8 *data);

//  Sound
void p_handle_audio(i64 time_elapsed);
void p_queue_sound(i64 delay, i64 num_samples, f32 *samples);

//  UDP sockets
i64 p_recv(u16 slot, IP_Address address, i64 size, u8 *data, u16 *local_port, IP_Address *remote_address);
i64 p_send(u16 slot, IP_Address address, i64 size, u8 *data, u16 *local_port);

extern Platform platform;

//  ================================================================

#endif // REDUCED_SYSTEM_LAYER_HEADER_GUARD_

//  ================================================================
//
//    WRITE YOUR CODE HERE
//
//  ================================================================

#ifdef REDUCED_SYSTEM_LAYER_EXAMPLE

i32 main(i32 argc, c8 **argv) {
  (void) argc;
  (void) argv;

  platform = (Platform) {
    .title        = "Reduced System Layer",
    .frame_width  = 1280,
    .frame_height =  720,
  };

  p_init();

  while (!platform.done) {
    p_handle_events();
    p_render_frame();
    p_sleep_for(0);
  }

  p_cleanup();
  return 0;
}

#endif

//  ================================================================
//
//    PLATFORM IMPLEMENTATION
//
//  ================================================================

#ifndef REDUCED_SYSTEM_LAYER_HEADER
#ifndef REDUCED_SYSTEM_LAYER_IMPL_GUARD_
#define REDUCED_SYSTEM_LAYER_IMPL_GUARD_

Platform platform = {0};

//  ================================================================
//
//  Utilities
//
//  ================================================================

c32 utf8_read(i64 len, c8 *s) {
  if (len >= 1 &&
      (s[0] & 0x80) == 0)
    return s[0];
  if (len >= 2 &&
      (s[0] & 0xe0) == 0xc0 &&
      (s[1] & 0xc0) == 0x80)
    return (s[1] & 0x3f)
        | ((s[0] & 0x1f) << 6);
  if (len >= 3 &&
      (s[0] & 0xf0) == 0xe0 &&
      (s[1] & 0xc0) == 0x80 &&
      (s[2] & 0xc0) == 0x80)
    return (s[2] & 0x3f)
        | ((s[1] & 0x3f) <<  6)
        | ((s[0] & 0x0f) << 12);
  if (len >= 4 &&
      (s[0] & 0xf8) == 0xf0 &&
      (s[1] & 0xc0) == 0x80 &&
      (s[2] & 0xc0) == 0x80 &&
      (s[3] & 0xc0) == 0x80)
    return (s[3] & 0x3f)
        | ((s[2] & 0x3f) <<  6)
        | ((s[1] & 0x3f) << 12)
        | ((s[0] & 0x07) << 18);
  return 0;
}

i32 utf8_size(c32 c) {
  if ((c & 0x00007f) == c) return 1;
  if ((c & 0x0007ff) == c) return 2;
  if ((c & 0x00ffff) == c) return 3;
  if ((c & 0x1fffff) == c) return 4;
  return 0;
}

i32 utf8_write(c32 c, c8 *buffer) {
  if ((c & 0x7f) == c) {
    buffer[0] = (c8) c;
    return 1;
  }

  if ((c & 0x7ff) == c) {
    buffer[0] = 0xc0 | ((c >> 6) & 0x1f);
    buffer[1] = 0x80 | ( c       & 0x3f);
    return 2;
  }

  if ((c & 0xffff) == c) {
    buffer[0] = 0xc0 | ((c >> 12) & 0x0f);
    buffer[1] = 0x80 | ((c >>  6) & 0x3f);
    buffer[2] = 0x80 | ( c        & 0x3f);
    return 3;
  }

  if ((c & 0x1fffff) == c) {
    buffer[0] = 0xc0 | ((c >> 18) & 0x07);
    buffer[1] = 0x80 | ((c >> 12) & 0x3f);
    buffer[2] = 0x80 | ((c >>  6) & 0x3f);
    buffer[3] = 0x80 | ( c        & 0x3f);
    return 4;
  }

  return 0;
}

//  ================================================================
//
//  Unix
//
//  ================================================================

#ifdef __unix__

#include <sched.h>
#include <unistd.h>

i64 p_time(void) {
  struct timespec t;
  timespec_get(&t, TIME_UTC);
  return 1000 * t.tv_sec + t.tv_nsec / 1000000;
}

void p_yield(void) {
  sched_yield();
}

void p_sleep_for(i64 duration) {
  if (duration <= 0) {
    usleep(0);
    return;
  }

  if (duration >= 1000)
    // seconds
    sleep(duration / 1000);

  if ((duration % 1000) > 0)
    // microseconds
    usleep((duration % 1000) * 1000);
}

#endif

//  ================================================================
//
//  UDP sockets
//
//  ================================================================

#ifdef __unix__

#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct {
  b8         ready;
  i32        socket;
  u16        local_port;
  IP_Address address;
} Socket_Slot;

b8          _sockets_init             = 0;
Socket_Slot _sockets[MAX_NUM_SOCKETS] = {0};

void sockets_initialize(void) {
  if (_sockets_init)
    return;

  signal(SIGPIPE, SIG_IGN);
  _sockets_init = 1;
}

void sockets_cleanup(void) {
  for (i64 i = 0; i < MAX_NUM_SOCKETS; ++i)
    if (_sockets[i].ready) {
      close(_sockets[i].socket);
      _sockets[i].ready = 0;
    }
}

b8 sockets_open(u16 slot, IP_Address address, u16 *local_port) {
  sockets_initialize();

  b8 change_address =
       !_sockets[slot].ready
    ||  _sockets[slot].address.protocol != address.protocol
    || (address.port != 0 && _sockets[slot].local_port != address.port)
    || (memcmp(_sockets[slot].address.v6_address, &(u8[sizeof address.v6_address]) {0}, sizeof address.v6_address) != 0 &&
        memcmp(_sockets[slot].address.v6_address, address.v6_address,                   sizeof address.v6_address) != 0);

  if (change_address && _sockets[slot].ready) {
    close(_sockets[slot].socket);
    _sockets[slot].ready = 0;
  }

  struct sockaddr *p;
  i32              p_len;

  struct sockaddr_in  a4 = {0};
  struct sockaddr_in6 a6 = {0};

  if (address.protocol == IPv4_UDP) {
    p     = (struct sockaddr *) &a4;
    p_len = sizeof a4;
  } else {
    p     = (struct sockaddr *) &a6;
    p_len = sizeof a6;
  }

  if (!_sockets[slot].ready) {
    _sockets[slot].socket = socket(address.protocol == IPv4_UDP ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (_sockets[slot].socket == -1) {
      fprintf(stderr, "ERROR: socket failed (errno %d)\n", errno);
      return 0;
    }

    if (address.protocol == IPv4_UDP) {
      a4.sin_family      = AF_INET;
      a4.sin_port        = htons(address.port);
      a4.sin_addr.s_addr = address.v4_address_as_u32;
    } else {
      a6.sin6_family = AF_INET6;
      a6.sin6_port   = htons(address.port);
      memcpy(a6.sin6_addr.s6_addr, address.v6_address, sizeof a6.sin6_addr.s6_addr);
    }

    if (bind(_sockets[slot].socket, p, p_len) == -1) {
      close(_sockets[slot].socket);

      fprintf(stderr, "ERROR: bind failed (errno %d)\n", errno);
      return 0;
    }

    if (getsockname(_sockets[slot].socket, p, &(socklen_t) {p_len}) == -1) {
      close(_sockets[slot].socket);

      fprintf(stderr, "ERROR: getsockname failed (errno %d)\n", errno);
      return 0;
    }

    if (p->sa_family == AF_INET)
      _sockets[slot].local_port = ntohs(a4.sin_port);
    else
      _sockets[slot].local_port = ntohs(a6.sin6_port);

    _sockets[slot].ready = 1;
    _sockets[slot].address = address;
  }

  if (local_port != NULL)
    *local_port = _sockets[slot].local_port;

  return 1;
}

i64 p_recv(u16 slot, IP_Address address, i64 size, u8 *data, u16 *local_port, IP_Address *remote_address) {
  assert(slot < MAX_NUM_SOCKETS);
  assert(address.protocol == IPv4_UDP || address.protocol == IPv6_UDP);

  if (!sockets_open(slot, address, local_port))
    return 0;
  if (size <= 0)
    return 0;

  struct sockaddr *p;
  i32              p_len;

  struct sockaddr_in  a4 = {0};
  struct sockaddr_in6 a6 = {0};

  if (address.protocol == IPv4_UDP) {
    p     = (struct sockaddr *) &a4;
    p_len = sizeof a4;
  } else {
    p     = (struct sockaddr *) &a6;
    p_len = sizeof a6;
  }

  i64 received = recvfrom(
    _sockets[slot].socket,
    data,
    size,
    MSG_DONTWAIT,
    p,
    &(socklen_t) {p_len}
  );

  if (received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 0;

    fprintf(stderr, "ERROR: recvfrom failed (errno %d)\n", errno);
    return 0;
  }

  if (remote_address != NULL) {
    memset(remote_address, 0, sizeof *remote_address);
    remote_address->protocol = address.protocol;

    if (address.protocol == IPv4_UDP) {
      remote_address->port              = ntohs(a4.sin_port);
      remote_address->v4_address_as_u32 = a4.sin_addr.s_addr;
    } else {
      remote_address->port = ntohs(a6.sin6_port);
      memcpy(remote_address->v6_address, a6.sin6_addr.s6_addr, sizeof remote_address->v6_address);
    }
  }

  return received;
}

i64 p_send(u16 slot, IP_Address address, i64 size, u8 *data, u16 *local_port) {
  assert(slot < MAX_NUM_SOCKETS);
  assert(address.protocol == IPv4_UDP || address.protocol == IPv6_UDP);

  IP_Address local_address = address;
  local_address.port       = 0;

  if (!sockets_open(slot, local_address, local_port))
    return 0;
  if (size <= 0)
    return 0;

  struct sockaddr *p;
  i32              p_len;

  struct sockaddr_in  a4 = {0};
  struct sockaddr_in6 a6 = {0};

  if (address.protocol == IPv4_UDP) {
    p     = (struct sockaddr *) &a4;
    p_len = sizeof a4;

    a4.sin_family      = AF_INET;
    a4.sin_port        = htons(address.port);
    a4.sin_addr.s_addr = address.v4_address_as_u32;
  } else {
    p     = (struct sockaddr *) &a6;
    p_len = sizeof a6;

    a6.sin6_family = AF_INET6;
    a6.sin6_port   = htons(address.port);
    memcpy(a6.sin6_addr.s6_addr, address.v6_address, sizeof a6.sin6_addr.s6_addr);
  }

  i64 sent = sendto(
    _sockets[0].socket,
    data,
    size,
    MSG_DONTWAIT,
    p,
    p_len
  );

  if (sent < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return 0;

    fprintf(stderr, "ERROR: sendto failed (errno %d)\n", errno);
    return 0;
  }

  return sent;
}

#endif

//  ================================================================
//
//  X11
//
//  ================================================================

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

static i16       _key_table[512]                       = {0};
static b8        _key_repeat[512]                      = {0};
static u32       _buffer[MAX_NUM_PIXELS]               = {0};
static Input_Key _input[MAX_INPUT_SIZE]                = {0};
static c8        _clipboard_buffer[MAX_CLIPBOARD_SIZE] = {0};
static XImage    _image                                = {0};
static Display * _display                              = NULL;
static GC        _gc                                   = NULL;
static XIM       _im                                   = NULL;
static XIC       _ic                                   = NULL;
static Window    _window                               = 0;
static Atom      _wm_delete_window                     = 0;
static Atom      _clipboard                            = 0;
static Atom      _targets                              = 0;
static Atom      _utf8_string                          = 0;
static Atom      _target                               = None;

void p_init(void) {
  _display = XOpenDisplay(NULL);
  assert(_display != NULL);

  _key_table[XKeysymToKeycode(_display, XK_Left)]         = KEY_LEFT;
  _key_table[XKeysymToKeycode(_display, XK_Right)]        = KEY_RIGHT;
  _key_table[XKeysymToKeycode(_display, XK_Up)]           = KEY_UP;
  _key_table[XKeysymToKeycode(_display, XK_Down)]         = KEY_DOWN;
  _key_table[XKeysymToKeycode(_display, XK_1)]            = '1';
  _key_table[XKeysymToKeycode(_display, XK_2)]            = '2';
  _key_table[XKeysymToKeycode(_display, XK_3)]            = '3';
  _key_table[XKeysymToKeycode(_display, XK_4)]            = '4';
  _key_table[XKeysymToKeycode(_display, XK_5)]            = '5';
  _key_table[XKeysymToKeycode(_display, XK_6)]            = '6';
  _key_table[XKeysymToKeycode(_display, XK_7)]            = '7';
  _key_table[XKeysymToKeycode(_display, XK_8)]            = '8';
  _key_table[XKeysymToKeycode(_display, XK_9)]            = '9';
  _key_table[XKeysymToKeycode(_display, XK_0)]            = '0';
  _key_table[XKeysymToKeycode(_display, XK_A)]            = 'a';
  _key_table[XKeysymToKeycode(_display, XK_B)]            = 'b';
  _key_table[XKeysymToKeycode(_display, XK_C)]            = 'c';
  _key_table[XKeysymToKeycode(_display, XK_D)]            = 'd';
  _key_table[XKeysymToKeycode(_display, XK_E)]            = 'e';
  _key_table[XKeysymToKeycode(_display, XK_F)]            = 'f';
  _key_table[XKeysymToKeycode(_display, XK_G)]            = 'g';
  _key_table[XKeysymToKeycode(_display, XK_H)]            = 'h';
  _key_table[XKeysymToKeycode(_display, XK_I)]            = 'i';
  _key_table[XKeysymToKeycode(_display, XK_J)]            = 'j';
  _key_table[XKeysymToKeycode(_display, XK_K)]            = 'k';
  _key_table[XKeysymToKeycode(_display, XK_L)]            = 'l';
  _key_table[XKeysymToKeycode(_display, XK_M)]            = 'm';
  _key_table[XKeysymToKeycode(_display, XK_N)]            = 'n';
  _key_table[XKeysymToKeycode(_display, XK_O)]            = 'o';
  _key_table[XKeysymToKeycode(_display, XK_P)]            = 'p';
  _key_table[XKeysymToKeycode(_display, XK_Q)]            = 'q';
  _key_table[XKeysymToKeycode(_display, XK_R)]            = 'r';
  _key_table[XKeysymToKeycode(_display, XK_S)]            = 's';
  _key_table[XKeysymToKeycode(_display, XK_T)]            = 't';
  _key_table[XKeysymToKeycode(_display, XK_U)]            = 'u';
  _key_table[XKeysymToKeycode(_display, XK_V)]            = 'v';
  _key_table[XKeysymToKeycode(_display, XK_W)]            = 'w';
  _key_table[XKeysymToKeycode(_display, XK_X)]            = 'x';
  _key_table[XKeysymToKeycode(_display, XK_Y)]            = 'y';
  _key_table[XKeysymToKeycode(_display, XK_Z)]            = 'z';
  _key_table[XKeysymToKeycode(_display, XK_space)]        = ' ';
  _key_table[XKeysymToKeycode(_display, XK_braceleft)]    = '[';
  _key_table[XKeysymToKeycode(_display, XK_braceright)]   = ']';
  _key_table[XKeysymToKeycode(_display, XK_colon)]        = ';';
  _key_table[XKeysymToKeycode(_display, XK_quotedbl)]     = '\'';
  _key_table[XKeysymToKeycode(_display, XK_asciitilde)]   = '`';
  _key_table[XKeysymToKeycode(_display, XK_backslash)]    = '\\';
  _key_table[XKeysymToKeycode(_display, XK_comma)]        = ',';
  _key_table[XKeysymToKeycode(_display, XK_greater)]      = '.';
  _key_table[XKeysymToKeycode(_display, XK_question)]     = '/';
  _key_table[XKeysymToKeycode(_display, XK_minus)]        = '-';
  _key_table[XKeysymToKeycode(_display, XK_equal)]        = '=';
  _key_table[XKeysymToKeycode(_display, XK_F1)]           = KEY_F_ + 1;
  _key_table[XKeysymToKeycode(_display, XK_F2)]           = KEY_F_ + 2;
  _key_table[XKeysymToKeycode(_display, XK_F3)]           = KEY_F_ + 3;
  _key_table[XKeysymToKeycode(_display, XK_F4)]           = KEY_F_ + 4;
  _key_table[XKeysymToKeycode(_display, XK_F5)]           = KEY_F_ + 5;
  _key_table[XKeysymToKeycode(_display, XK_F6)]           = KEY_F_ + 6;
  _key_table[XKeysymToKeycode(_display, XK_F7)]           = KEY_F_ + 7;
  _key_table[XKeysymToKeycode(_display, XK_F8)]           = KEY_F_ + 8;
  _key_table[XKeysymToKeycode(_display, XK_F9)]           = KEY_F_ + 9;
  _key_table[XKeysymToKeycode(_display, XK_F10)]          = KEY_F_ + 10;
  _key_table[XKeysymToKeycode(_display, XK_F11)]          = KEY_F_ + 11;
  _key_table[XKeysymToKeycode(_display, XK_F12)]          = KEY_F_ + 12;
  _key_table[XKeysymToKeycode(_display, XK_Control_L)]    = KEY_LCTRL;
  _key_table[XKeysymToKeycode(_display, XK_Control_R)]    = KEY_RCTRL;
  _key_table[XKeysymToKeycode(_display, XK_Shift_L)]      = KEY_LSHIFT;
  _key_table[XKeysymToKeycode(_display, XK_Shift_R)]      = KEY_RSHIFT;
  _key_table[XKeysymToKeycode(_display, XK_Alt_L)]        = KEY_LALT;
  _key_table[XKeysymToKeycode(_display, XK_Alt_R)]        = KEY_RALT;
  _key_table[XKeysymToKeycode(_display, XK_Escape)]       = KEY_ESCAPE;
  _key_table[XKeysymToKeycode(_display, XK_BackSpace)]    = '\b';
  _key_table[XKeysymToKeycode(_display, XK_Tab)]          = '\t';
  _key_table[XKeysymToKeycode(_display, XK_Return)]       = '\n';
  _key_table[XKeysymToKeycode(_display, XK_Print)]        = KEY_PRINTSCREEN;
  _key_table[XKeysymToKeycode(_display, XK_Delete)]       = KEY_DELETE;
  _key_table[XKeysymToKeycode(_display, XK_Pause)]        = KEY_PAUSE;
  _key_table[XKeysymToKeycode(_display, XK_Insert)]       = KEY_INSERT;
  _key_table[XKeysymToKeycode(_display, XK_Home)]         = KEY_HOME;
  _key_table[XKeysymToKeycode(_display, XK_End)]          = KEY_END;
  _key_table[XKeysymToKeycode(_display, XK_Page_Up)]      = KEY_PAGEUP;
  _key_table[XKeysymToKeycode(_display, XK_Page_Down)]    = KEY_PAGEDOWN;
  _key_table[XKeysymToKeycode(_display, XK_KP_0)]         = KEY_KP_ + '0';
  _key_table[XKeysymToKeycode(_display, XK_KP_1)]         = KEY_KP_ + '1';
  _key_table[XKeysymToKeycode(_display, XK_KP_2)]         = KEY_KP_ + '2';
  _key_table[XKeysymToKeycode(_display, XK_KP_3)]         = KEY_KP_ + '3';
  _key_table[XKeysymToKeycode(_display, XK_KP_4)]         = KEY_KP_ + '4';
  _key_table[XKeysymToKeycode(_display, XK_KP_5)]         = KEY_KP_ + '5';
  _key_table[XKeysymToKeycode(_display, XK_KP_6)]         = KEY_KP_ + '6';
  _key_table[XKeysymToKeycode(_display, XK_KP_7)]         = KEY_KP_ + '7';
  _key_table[XKeysymToKeycode(_display, XK_KP_8)]         = KEY_KP_ + '8';
  _key_table[XKeysymToKeycode(_display, XK_KP_9)]         = KEY_KP_ + '9';
  _key_table[XKeysymToKeycode(_display, XK_KP_Enter)]     = KEY_KP_ + '\n';
  _key_table[XKeysymToKeycode(_display, XK_KP_Divide)]    = KEY_KP_ + '/';
  _key_table[XKeysymToKeycode(_display, XK_KP_Multiply)]  = KEY_KP_ + '*';
  _key_table[XKeysymToKeycode(_display, XK_KP_Add)]       = KEY_KP_ + '+';
  _key_table[XKeysymToKeycode(_display, XK_KP_Subtract)]  = KEY_KP_ + '-';
  _key_table[XKeysymToKeycode(_display, XK_KP_Decimal)]   = KEY_KP_ + '.';
  _key_table[XKeysymToKeycode(_display, XK_KP_Separator)] = KEY_KP_ + ',';

  i32     screen = DefaultScreen(_display);
  i32     depth  = DefaultDepth (_display, screen);
  Visual *visual = DefaultVisual(_display, screen);

  _gc = DefaultGC(_display, screen);
  assert(_gc != NULL);

  XSetGraphicsExposures(_display, _gc, False);

  i32 _display_width  = DisplayWidth (_display, screen);
  i32 _display_height = DisplayHeight(_display, screen);

  if (platform.frame_width  <= 0)
    platform.frame_width  = 400;
  if (platform.frame_height <= 0)
    platform.frame_height = 300;

  i32 x = (_display_width  - platform.frame_width)  / 2;
  i32 y = (_display_height - platform.frame_height) / 2;

  _window = XCreateWindow(_display, XDefaultRootWindow(_display), x, y, platform.frame_width, platform.frame_height, 0, depth, InputOutput, visual, CWEventMask, &(XSetWindowAttributes) { .event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | VisibilityChangeMask | FocusChangeMask | StructureNotifyMask | SubstructureNotifyMask, });

  _im = XOpenIM(_display, NULL, NULL, NULL);
  assert(_im != NULL);

  _ic = XCreateIC(_im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, _window, NULL);
  assert(_ic != NULL);

  platform.pixels    = _buffer;
  platform.input     = _input;
  platform.clipboard = _clipboard_buffer;

  _image = (XImage) {
    .width            = platform.frame_width,
    .height           = platform.frame_height,
    .depth            = depth,
    .xoffset          = 0,
    .format           = ZPixmap,
    .data             = (c8 *) _buffer,
    .byte_order       = LSBFirst,
    .bitmap_unit      = 32,
    .bitmap_bit_order = LSBFirst,
    .bitmap_pad       = 32,
    .bits_per_pixel   = 32,
    .bytes_per_line   = 4 * platform.frame_width,
    .red_mask         = 0xff0000,
    .green_mask       = 0x00ff00,
    .blue_mask        = 0x0000ff,
  };

  XInitImage(&_image);

  _wm_delete_window = XInternAtom(_display, "WM_DELETE_WINDOW", False);
  _clipboard        = XInternAtom(_display, "CLIPBOARD",        False);
  _targets          = XInternAtom(_display, "TARGETS",          False);
  _utf8_string      = XInternAtom(_display, "UTF8_STRING",      False);

  XSetICFocus(_ic);
  XSetWMProtocols(_display, _window, &_wm_delete_window, 1);

  if (platform.title != NULL)
    XStoreName(_display, _window, platform.title);

  XMapWindow(_display, _window);

  XPutImage(_display, _window, _gc, &_image, 0, 0, 0, 0, platform.frame_width, platform.frame_height);
  XFlush(_display);
}

void p_cleanup(void) {
  if (!platform.graceful_exit)
    return;

  if (_window != 0)
    XDestroyWindow(_display, _window);
  if (_display != NULL)
    XCloseDisplay (_display);

  _display = NULL;
  _window  = 0;

  sockets_cleanup();
}

i32 p_handle_events(void) {
  i32 num_events = 0;

  memset(platform.key_pressed, 0, sizeof platform.key_pressed);
  memset(_key_repeat, 0, sizeof _key_repeat);

  platform.input_size = 0;
  platform.cursor_dx  = 0;
  platform.cursor_dy  = 0;
  platform.wheel_dy   = 0;

  XEvent ev;

  b8 requested_clipboard = 0;

  while (XEventsQueued(_display, QueuedAlready) > 0) {
    ++num_events;

    XNextEvent(_display, &ev);
    XFilterEvent(&ev, _window);

    switch (ev.type) {
      case DestroyNotify:
        platform.done = 1;
        break;

      case MotionNotify:
        platform.cursor_dx += ev.xmotion.x - platform.cursor_x;
        platform.cursor_dy += ev.xmotion.y - platform.cursor_y;
        platform.cursor_x   = ev.xmotion.x;
        platform.cursor_y   = ev.xmotion.y;
        break;

      case ButtonPress:
        platform.cursor_x = ev.xbutton.x;
        platform.cursor_y = ev.xbutton.y;
        switch (ev.xbutton.button) {
          case Button1:
            platform.key_down[BUTTON_LEFT]    = 1;
            platform.key_pressed[BUTTON_LEFT] = 1;
            break;
          case Button2:
            platform.key_down[BUTTON_MIDDLE]    = 1;
            platform.key_pressed[BUTTON_MIDDLE] = 1;
            break;
          case Button3:
            platform.key_down[BUTTON_RIGHT]    = 1;
            platform.key_pressed[BUTTON_RIGHT] = 1;
            break;
          case Button4: ++platform.wheel_dy; break;
          case Button5: --platform.wheel_dy; break;
          default:;
        }
        if (!requested_clipboard) {
          XConvertSelection(_display, _clipboard, _targets, _clipboard, _window, CurrentTime);
          requested_clipboard = 1;
        }
        break;

      case ButtonRelease:
        platform.cursor_x = ev.xbutton.x;
        platform.cursor_y = ev.xbutton.y;
        switch (ev.xbutton.button) {
          case Button1: platform.key_down[BUTTON_LEFT]   = 0; break;
          case Button2: platform.key_down[BUTTON_MIDDLE] = 0; break;
          case Button3: platform.key_down[BUTTON_RIGHT]  = 0; break;
          default:;
        }
        break;

      case KeyPress: {
        i16 k = _key_table[ev.xkey.keycode];
        platform.cursor_x = ev.xkey.x;
        platform.cursor_y = ev.xkey.y;

        platform.key_down[k] = 1;
        if (!_key_repeat[k])
          platform.key_pressed[k] = 1;

        platform.key_down[MOD_CTRL]   = !!(ev.xkey.state & ControlMask);
        platform.key_down[MOD_SHIFT]  = !!(ev.xkey.state & ShiftMask);
        platform.key_down[MOD_ALT]    = !!(ev.xkey.state & Mod1Mask);
        platform.key_down[MOD_CAPS]   = !!(ev.xkey.state & LockMask);
        platform.key_down[MOD_NUM]    = !!(ev.xkey.state & Mod2Mask);
        platform.key_down[MOD_SCROLL] = !!(ev.xkey.state & Mod3Mask);

        if (platform.input_size < MAX_INPUT_SIZE) {
          if (k < 32 || k >= 128)
            platform.input[platform.input_size++] = (Input_Key) {
              .ctrl   = !!(ev.xkey.state & ControlMask),
              .shift  = !!(ev.xkey.state & ShiftMask),
              .alt    = !!(ev.xkey.state & Mod1Mask),
              .caps   = !!(ev.xkey.state & LockMask),
              .num    = !!(ev.xkey.state & Mod2Mask),
              .scroll = !!(ev.xkey.state & Mod3Mask),
              .key    = k,
              .c      = 0,
            };
          else {
            c8  buf[16];
            i32 len = Xutf8LookupString(_ic, &ev.xkey, buf, sizeof buf - 1, NULL, NULL);
            if (len > 0)
              platform.input[platform.input_size++] = (Input_Key) {
                .ctrl   = !!(ev.xkey.state & ControlMask),
                .shift  = !!(ev.xkey.state & ShiftMask),
                .alt    = !!(ev.xkey.state & Mod1Mask),
                .caps   = !!(ev.xkey.state & LockMask),
                .num    = !!(ev.xkey.state & Mod2Mask),
                .scroll = !!(ev.xkey.state & Mod3Mask),
                .key    = k,
                .c      = utf8_read(len, buf),
              };
          }
        }

        if (!requested_clipboard) {
          XConvertSelection(_display, _clipboard, _targets, _clipboard, _window, CurrentTime);
          requested_clipboard = 1;
        }
      } break;

      case KeyRelease: {
        i16 k = _key_table[ev.xkey.keycode];

        platform.cursor_x = ev.xkey.x;
        platform.cursor_y = ev.xkey.y;

        platform.key_down[k] = 0;
        _key_repeat[k]       = 1;

        platform.key_down[MOD_CTRL]   = !!(ev.xkey.state & ControlMask);
        platform.key_down[MOD_SHIFT]  = !!(ev.xkey.state & ShiftMask);
        platform.key_down[MOD_ALT]    = !!(ev.xkey.state & Mod1Mask);
        platform.key_down[MOD_CAPS]   = !!(ev.xkey.state & LockMask);
        platform.key_down[MOD_NUM]    = !!(ev.xkey.state & Mod2Mask);
        platform.key_down[MOD_SCROLL] = !!(ev.xkey.state & Mod3Mask);
      } break;

      case SelectionRequest:
        if (ev.xselectionrequest.requestor           != _window    &&
            ev.xselectionrequest.selection           == _clipboard &&
            XGetSelectionOwner(_display, _clipboard) == _window) {
          if (ev.xselectionrequest.property != None) {
            if (ev.xselectionrequest.target == _targets)
              XChangeProperty(
                ev.xselectionrequest.display,
                ev.xselectionrequest.requestor,
                ev.xselectionrequest.property,
                XA_ATOM,
                32,
                PropModeReplace,
                (u8 *) &_utf8_string,
                1
              );
            else if (ev.xselectionrequest.target == _utf8_string)
              XChangeProperty(
                ev.xselectionrequest.display,
                ev.xselectionrequest.requestor,
                ev.xselectionrequest.property,
                ev.xselectionrequest.target,
                8,
                PropModeReplace,
                (u8 *) platform.clipboard,
                platform.clipboard_size
              );
          }

          XSendEvent(_display, ev.xselectionrequest.requestor, 0, 0, (XEvent *) &(XSelectionEvent) {
            .type       = SelectionNotify,
            .serial     = ev.xselectionrequest.serial,
            .send_event = ev.xselectionrequest.send_event,
            .display    = ev.xselectionrequest.display,
            .requestor  = ev.xselectionrequest.requestor,
            .selection  = ev.xselectionrequest.selection,
            .target     = ev.xselectionrequest.target,
            .property   = ev.xselectionrequest.property,
            .time       = ev.xselectionrequest.time,
          });
        }
        break;

      case SelectionNotify:
        if (ev.xselection.property != None) {
          i64 len  = 0;
          u8 *data = NULL;

          XGetWindowProperty(
            _display,
            _window,
            _clipboard,
            0,
            MAX_CLIPBOARD_SIZE / 4,
            False,
            AnyPropertyType,
            &(Atom) {0},
            &(int)  {0},
            (unsigned long *) &len,
            &(unsigned long) {0},
            &data
          );

          if (ev.xselection.target == _targets) {
            _target    = None;
            Atom *list = (Atom *) data;

            for (i64 i = 0; i < len / 4; i++)
              if (list[i] == XA_STRING)
                _target = XA_STRING;
              else if (list[i] == _utf8_string) {
                _target = _utf8_string;
                break;
              }

            if (_target != None)
              XConvertSelection(_display, _clipboard, _target, _clipboard, _window, CurrentTime);
          } else if (ev.xselection.target == _target) {
            if (len > MAX_CLIPBOARD_SIZE)
              len = MAX_CLIPBOARD_SIZE;
            platform.clipboard_size = len;
            if (len > 0)
              memcpy(platform.clipboard, data, len);
          }

          if (data)
            XFree(data);
        }
        break;

      case EnterNotify: platform.has_cursor = 1; break;
      case LeaveNotify: platform.has_cursor = 0; break;
      case FocusOut:    platform.has_focus  = 0; break;

      case FocusIn:
        platform.has_focus = 1;
        if (!requested_clipboard) {
          XConvertSelection(_display, _clipboard, _targets, _clipboard, _window, CurrentTime);
          requested_clipboard = 1;
        }
        break;

      case MappingNotify:
        XRefreshKeyboardMapping(&ev.xmapping);
        break;

      case ClientMessage:
        if ((Atom) ev.xclient.data.l[0] == _wm_delete_window)
          platform.done = 1;
        break;

      default:;
    }
  }

  XWindowAttributes attrs;
  XGetWindowAttributes(_display, _window, &attrs);

  if ((platform.frame_width != attrs.width || platform.frame_height != attrs.height) && attrs.width * attrs.height * 4 <= (i32) sizeof _buffer) {
    if (attrs.width > 0 && attrs.height > 0) {
      _image.width          = attrs.width;
      _image.height         = attrs.height;
      _image.bytes_per_line = 4 * attrs.width;
    }

    platform.frame_width  = attrs.width;
    platform.frame_height = attrs.height;
  }

  return num_events;
}

i32 p_wait_events(void) {
  i32 num_events = 0;

  do {
    num_events = p_handle_events();
    sched_yield();
  } while (num_events == 0);

  return num_events;
}

void p_render_frame(void) {
  if (platform.done)
    return;

  XPutImage(_display, _window, _gc, &_image, 0, 0, 0, 0, platform.frame_width, platform.frame_height);
  XFlush(_display);
}

void p_clipboard_write(i64 size, c8 *data) {
  assert(size <= MAX_CLIPBOARD_SIZE);

  XSetSelectionOwner(_display, _clipboard, _window, CurrentTime);

  platform.clipboard_size = size < MAX_CLIPBOARD_SIZE ? size : MAX_CLIPBOARD_SIZE;
  if (platform.clipboard_size > 0)
    memcpy(platform.clipboard, data, platform.clipboard_size);
}

#endif

//  ================================================================
//
//  ALSA
//
//  ================================================================

#ifdef __linux__

void p_handle_audio(i64 time_elapsed) {
  (void) time_elapsed;
  assert(0);
}

void p_queue_sound(i64 delay, i64 num_samples, f32 *samples) {
  (void) delay;
  (void) num_samples;
  (void) samples;
  assert(0);
}

#endif

//  ================================================================

#endif // REDUCED_SYSTEM_LAYER_IMPL_GUARD_
#endif // REDUCED_SYSTEM_LAYER_HEADER
