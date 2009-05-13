#include <stdlib.h>
#include <stdio.h>
#include "rllib.h"

void init_msg() {
  set_msg_type(msg_normal, 0, 0, 80, 10);
  set_msg_style(style_normal, yellow, blue);
}

int rlmain() {
  key k;
  create_window(80, 50, "Example", "font.rlf");
  init_msg();

  print_msg(msg_normal, style_normal, "This code uses the RLLib library. This is an example of use.");
  
  print_msg(msg_normal, style_normal, "Press any key to continue.");
  
  k = get_key();
  
  print_msg(msg_normal, style_normal, "You typed %c.", k.c);
  
  print_msg(msg_normal, style_normal, "Press any key to exit.");
  
  get_key();
  destroy_window();
  
  return 0;
}
