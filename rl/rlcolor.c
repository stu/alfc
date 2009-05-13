#include "rllib.h"

rgbcolor black, gray, red, blue, yellow, green, darkgray, darkgreen, lightblue, white;

void init_colors() {
  black = rgb(0, 0, 0), 
  gray = rgb(0xc0, 0xc0, 0xc0), 
  red = rgb(0xff, 0, 0), 
  blue = rgb(0x00, 0, 0xff), 
  yellow = rgb(0xff, 0xff, 0), 
  green = rgb(0, 0xff, 0),
  darkgray = rgb(0x40, 0x40, 0x40),
  darkgreen = rgb(0, 0x80, 0),
  lightblue = rgb(0x40, 0x40, 0xff);
  white = rgb(0xff, 0xff, 0xff);
}
