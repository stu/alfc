
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "rllib.h"


message_type msg_types[num_msg_types];
message_style msg_styles[num_style_types];

char* to_str(int i) {
  /* Any 32-bit integer has less than 15 digits. */
  char* s = (char*) malloc(15);
  sprintf(s, "%i", i);
  return s;
}

static int _print_msg(char* msg, rgbcolor f, rgbcolor b, int x, int y) {
  int i;
  for (i = 0; msg[i]; i++) {
    display_char(msg[i], f, b, x++, y);
  }
  return x;
}

void print_msg(int type, int style, char* msg, ...) {
  va_list vl;
  int first_char = 1;
  int i = 0;
  rgbcolor f = msg_styles[style].fore;
  rgbcolor b = msg_styles[style].back;
  int x = msg_types[type].start_x;
  int y = msg_types[type].cur_y;

  if (msg_types[type].cur_y == msg_types[type].end_y-1) {
    _print_msg("--more--", gray, black, x, y);
    while (get_key().c != ' ');
    _print_msg("        ", gray, black, x, y);
    clear_msg(type);
    x = msg_types[type].start_x;
    y = msg_types[type].cur_y;
  }

  va_start(vl, msg);

  while (msg[i]) {
    if (msg[i] == '%') {
      char* m2;
      switch (msg[i+1]) {
        case 's':
          m2 = va_arg(vl, char*);
          if (first_char) {
            display_char(toupper(m2[0]), f, b, x, y);
            x = _print_msg(m2+1, f, b, x+1, y);
            first_char = 0;
          }
          else {
            x = _print_msg(m2, f, b, x, y);
          }
          break;
        case 'c':
          display_char(va_arg(vl, int), f, b, x, y);
          x++;
          break;
        case 'i': {
          char* int_str = to_str(va_arg(vl, int));
          x = _print_msg(int_str, f, b, x, y);
          free(int_str);
          break;
        }
        default:
          i++;
          goto display;
      }
      i += 2;
      continue;
    }
    display:
    if (first_char) {
      display_char(toupper(msg[i]), f, b, x, y);
      first_char = 0;
    }
    else {
      display_char(msg[i], f, b, x, y);
    }
    x++;
    i++;
  }
  va_end(vl);
  msg_types[type].cur_y++;
}

void clear_msg(int type) {
  int minx = msg_types[type].start_x;
  int miny = msg_types[type].start_y;
  int maxx = msg_types[type].end_x-1;
  int maxy = msg_types[type].end_y-1;
  int x, y;

  for (y = miny; y < maxy; y++) {
    for (x = minx; x < maxx; x++) {
      display_char(' ', gray, black, x, y);
    }
  }

  msg_types[type].cur_y = msg_types[type].start_y;
}

void set_msg_type(int type, int start_x, int start_y, int end_x, int end_y) {
  msg_types[type].start_x = start_x;
  msg_types[type].start_y = start_y;
  msg_types[type].end_x = end_x;
  msg_types[type].end_y = end_y;
  msg_types[type].cur_y = start_y;
}

void set_msg_style(int style, rgbcolor fore, rgbcolor back) {
  msg_styles[style].fore = fore;
  msg_styles[style].back = back;
}
