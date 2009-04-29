#ifndef RLCORE_H
#define RLCORE_H
#ifdef __cplusplus
extern "C"{
#endif


/*
  The header file for RLLib.
  These functions are implemented in rlbase.c, rlfont.c, rlcolor.c, and rlmsg.c.
*/

/*
  Basic window management functions.
*/

/* Creates a window. You need to call this before using anything else in RLLib. */
void create_window(int x_size, int y_size, char* title, uint8_t *font_data, uint32_t len);

/* Destroys the window. You should always call this at the end of your program. */
void destroy_window(void);

/* Get the window width and height. */
int window_width_in_chars(void);
int window_height_in_chars(void);
int font_width_in_pixels(void);
int font_height_in_pixels(void);
int window_resized(void);

/*
  Color functions.
*/

/* A color. */
typedef struct {
  int r;
  int g;
  int b;
} rgbcolor;

/* Converts red, green, and blue components into a color. */
rgbcolor rgb(int r, int g, int b);

/*
  Keyboard input functions.
*/

typedef struct {
  int shift;
  int ctrl;
  unsigned short c;
  int is_numpad;
} key;

/* Gets a key. */
key get_key();

/* Add this to a letter to get the ASCII code for ctrl+the letter. For example, 'q' + ctrl_offset is ctrl+q. */
#define ctrl_offset 1-'a'

/*
  Character display functions.
*/

/* Display a character */
void display_char(int char_num, rgbcolor fore, rgbcolor back, int x, int y);
void display_char_offset_x(int char_num, rgbcolor fore, rgbcolor back, int x, int y);
void display_char_offset_y(int char_num, rgbcolor fore, rgbcolor back, int x, int y);
void display_char_offset_both(int char_num, rgbcolor fore, rgbcolor back, int x, int y);



/*
  Low-level functions. You shouldn't need to use these at all.
*/

/* Most of these are low-level functions that are called by other functions. */
void init_window(int x_size, int y_size, char* title);
void init_colors();
rgbcolor get_pixel(int x, int y);
void set_pixel(int x, int y, rgbcolor color);
int window_width_in_pixels();
int window_height_in_pixels();

/* This isn't needed anymore because the screen is updated whenever you get a key. */
void update_window();

/* Returns whether two colors are different. */
int colors_different(rgbcolor a, rgbcolor b);

void display_char_at_pixel(int char_num, rgbcolor fore, rgbcolor back, int start_x, int start_y);

enum
{
	RLKEY_START_CODE = 0xC000,

	RLKEY_F1 = RLKEY_START_CODE,
	RLKEY_F2,
	RLKEY_F3,
	RLKEY_F4,
	RLKEY_F5,
	RLKEY_F6,
	RLKEY_F7,
	RLKEY_F8,
	RLKEY_F9,
	RLKEY_F10,
	RLKEY_F11,
	RLKEY_F12,
	RLKEY_LEFT,
	RLKEY_RIGHT,
	RLKEY_UP,
	RLKEY_DOWN,
	RLKEY_INSERT,
	RLKEY_DELETE,
	RLKEY_HOME,
	RLKEY_END,
	RLKEY_PRIOR,
	RLKEY_NEXT,
	RLKEY_BACK,
};

void clear(void);
void maximise_window(void);
int window_isshutdown(void);


#ifdef __cplusplus
}
#endif
#endif // _MAIN_H

