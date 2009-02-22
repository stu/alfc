/*
  Functions for printing text.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "rllib.h"


static char* font_data[256];
static int cur_byte;
static int cur_bit;
static int char_width;
static int char_height;
static int screen_x_size_in_chars;
static int screen_y_size_in_chars;

typedef struct {
  char c;
  rgbcolor f, b;
} displayed_char;

int displayed_chars_different(displayed_char d, int char_num, rgbcolor fore, rgbcolor back) {
  return d.c != char_num || colors_different(d.f, fore) || colors_different(d.b, back);
}

displayed_char get_displayed_char(int char_num, rgbcolor fore, rgbcolor back) {
  displayed_char d;
  d.c = char_num;
  d.f = fore;
  d.b = back;
  return d;
}


static displayed_char* stored_display;

int window_width_in_chars() {
  return screen_x_size_in_chars;
}

int window_height_in_chars() {
  return screen_y_size_in_chars;
}

static void read_font(int* cwidth, int* cheight, uint8_t *fdata, uint32_t flen)
{
	int char_num;
	int x;
	int y;

	int offs;

	cur_byte = 0;
	cur_bit = 0;

	if(! (memcmp(fdata+2, "RLF", 3) == 0 && (fdata[0] = 0xF3 && fdata[1] == 0x9E)))
	{
		printf("Error loading font: not RLF file.\n");
		exit(3);
	}

	offs = 5;

	char_width = fdata[offs++];
	char_height = fdata[offs++];

	for (char_num = 0; char_num < 256; char_num++)
	{
		font_data[char_num] = (char*) malloc(char_width*char_height);
		for (y = 0; y < char_height; y++)
		{
			for (x = 0; x < char_width; x++)
			{
				int color;

				if (cur_bit == 0)
					cur_byte = fdata[offs++];

				color = cur_byte & (1 << cur_bit);
				cur_bit++;

				if (cur_bit == 8)
					cur_bit = 0;

				if (color != 0)
					font_data[char_num][y*char_width+x] = 1;
				else
					font_data[char_num][y*char_width+x] = 0;
			}
		}
	}

	(*cwidth) = char_width;
	(*cheight) = char_height;
}

void create_window(int x_size, int y_size, char* title, uint8_t *fdata, uint32_t flen)
{
	int char_width, char_height, i, total_size;

	screen_x_size_in_chars = x_size;
	screen_y_size_in_chars = y_size;

	read_font(&char_width, &char_height, fdata, flen);

	init_window(x_size*char_width + 4, y_size*char_height + 4, title);

	total_size = x_size * y_size * 4;
	stored_display = (displayed_char*) malloc(total_size * sizeof(displayed_char));
	for (i = 0; i < total_size; i++)
		stored_display[i].c = 0;
}

void display_char_at_pixel(int char_num, rgbcolor fore, rgbcolor back, int start_x, int start_y) {
  int x, y;
  for (y = 0; y < char_height; y++) {
    for (x = 0; x < char_width; x++) {
      if (font_data[char_num][y*char_width+x])
        set_pixel(x+start_x, y+start_y, fore);
      else
        set_pixel(x+start_x, y+start_y, back);
    }
  }
}

void display_char(int char_num, rgbcolor fore, rgbcolor back, int x, int y) {
  if (displayed_chars_different(stored_display[(y * screen_x_size_in_chars + x)*4], char_num, fore, back)) {
    display_char_at_pixel(char_num, fore, back, x * char_width + 2, y * char_height + 2);
    stored_display[(y * screen_x_size_in_chars + x)*4] = get_displayed_char(char_num, fore, back);
  }
}

void display_char_offset_x(int char_num, rgbcolor fore, rgbcolor back, int x, int y) {
  if (displayed_chars_different(stored_display[(y * screen_x_size_in_chars + x)*4+1], char_num, fore, back)) {
    display_char_at_pixel(char_num, fore, back, x * char_width + 2 + char_width/2, y * char_height + 2);
    stored_display[(y * screen_x_size_in_chars + x)*4+1] = get_displayed_char(char_num, fore, back);
  }
}

void display_char_offset_y(int char_num, rgbcolor fore, rgbcolor back, int x, int y) {
  if (displayed_chars_different(stored_display[(y * screen_x_size_in_chars + x)*4+2], char_num, fore, back)) {
    display_char_at_pixel(char_num, fore, back, x * char_width + 2, y * char_height + 2 + char_height/2);
    stored_display[(y * screen_x_size_in_chars + x)*4+2] = get_displayed_char(char_num, fore, back);
  }
}

void display_char_offset_both(int char_num, rgbcolor fore, rgbcolor back, int x, int y) {
  if (displayed_chars_different(stored_display[(y * screen_x_size_in_chars + x)*4+3], char_num, fore, back)) {
    display_char_at_pixel(char_num, fore, back, x * char_width + 2 + char_width/2, y * char_height + 2 + char_height/2);
    stored_display[(y * screen_x_size_in_chars + x)*4+3] = get_displayed_char(char_num, fore, back);
  }
}
