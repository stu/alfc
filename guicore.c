/*
  Core functions of RLLib.
*/

#ifndef WIN32
#define xlib
#endif

#ifdef xlib
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <signal.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "guicore.h"

#ifdef xlib
static Display* screen_display;
static Window screen_win;
static GC screen_gc;
static XImage* screen_image;
static int screen_num;
static XGCValues screen_gc_values;
static XEvent event;
static key curkey;
static int curkey_k;
#else
static HINSTANCE hinst;
static HWND hwnd;
static PAINTSTRUCT paintstruct;
static HDC hdc, imghdc;
static HBITMAP scr_bmp;
static int first_paint = 1;
static BITMAPINFO di_bmp_info;
static uint8_t* bmpinfo;

static key curkey;
static int curkey_k;
static int gotkey;
#endif

static int screen_x_size;
static int screen_y_size;
static int driver_redraw;
static int driver_shutdown;

static uint8_t* font_data[256];
static int cur_byte;
static int cur_bit;
static int char_width;
static int char_height;

typedef struct
{
	char c;
	rgbcolor f, b;
} displayed_char;

displayed_char* stored_display;

int displayed_chars_different(displayed_char d, int char_num, rgbcolor fore, rgbcolor back)
{
	return d.c != char_num || colors_different(d.f, fore) || colors_different(d.b, back);
}

displayed_char get_displayed_char(int char_num, rgbcolor fore, rgbcolor back)
{
	displayed_char d;

	d.c = char_num;
	d.f = fore;
	d.b = back;

	return d;
}

int font_width_in_pixels()
{
	return char_width;
}

int font_height_in_pixels()
{
	return char_height;
}

int window_width_in_chars()
{
	return window_width_in_pixels() / font_width_in_pixels();
}

int window_height_in_chars()
{
	return window_height_in_pixels() / font_height_in_pixels();
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
		font_data[char_num] = (uint8_t*) malloc(char_width*char_height);
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

	read_font(&char_width, &char_height, fdata, flen);
	init_window(x_size*char_width+4, y_size*char_height+4, title);

	total_size = x_size * y_size * 4;
	stored_display = (displayed_char*) malloc(total_size * sizeof(displayed_char));

	for (i = 0; i < total_size; i++)
		stored_display[i].c = 0;
}

void display_char_at_pixel(int char_num, rgbcolor fore, rgbcolor back, int start_x, int start_y)
{
	int x, y;

	for (y = 0; y < char_height; y++)
	{
		for (x = 0; x < char_width; x++)
		{
			if (font_data[char_num][y*char_width+x])
				set_pixel(x+start_x, y+start_y, fore);
			else
				set_pixel(x+start_x, y+start_y, back);
		}
	}
}

void display_char(int char_num, rgbcolor fore, rgbcolor back, int x, int y)
{
	if (displayed_chars_different(stored_display[(y * window_width_in_chars() + x)*4], char_num, fore, back))
	{
		display_char_at_pixel(char_num, fore, back, x * char_width + 2, y * char_height + 2);
		stored_display[(y * window_width_in_chars() + x)*4] = get_displayed_char(char_num, fore, back);
	}
}

int window_isshutdown(void)
{
	return driver_shutdown;
}

int window_resized(void)
{
	int x = driver_redraw;
	driver_redraw = 0;
	return x;
}

int window_width_in_pixels(void)
{
	return screen_x_size;
}

int window_height_in_pixels(void)
{
	return screen_y_size;
}


#ifndef xlib
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			{
				//CREATESTRUCT *x = (CREATESTRUCT*)lParam;
				//fprintf(stderr, "createxxx  %i.%i\n", x->cx, x->cy);
			}
			break;

		case WM_SIZE:
			if(first_paint == 0)
			{
				DeleteObject(scr_bmp);

				screen_x_size =  LOWORD(lParam);
				screen_y_size =  HIWORD(lParam); // >> 16;

				// round to 4.. seems to fix screen corruption bug...
				screen_x_size = (screen_x_size + 3) & 0xFFFC;

				scr_bmp = CreateCompatibleBitmap(hdc, screen_x_size, screen_y_size);

				di_bmp_info.bmiHeader.biWidth = screen_x_size;
				di_bmp_info.bmiHeader.biHeight = screen_y_size;

				bmpinfo = (uint8_t*) realloc(bmpinfo, screen_x_size*screen_y_size*4);
				stored_display = (displayed_char*) realloc(stored_display, (screen_x_size*screen_y_size*4) * sizeof(displayed_char));

				first_paint = 1;
				driver_redraw = 1;
			}
			break;

		case WM_PAINT:
			hdc = BeginPaint(hwnd, &paintstruct);
			if(first_paint == 1)
			{
				if(scr_bmp != NULL)
					DeleteObject(scr_bmp);

				scr_bmp = CreateCompatibleBitmap(hdc, screen_x_size, screen_y_size);

				di_bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				di_bmp_info.bmiHeader.biWidth = screen_x_size;
				di_bmp_info.bmiHeader.biHeight = screen_y_size;
				di_bmp_info.bmiHeader.biPlanes = 1;
				di_bmp_info.bmiHeader.biBitCount = 24;
				di_bmp_info.bmiHeader.biCompression = 0;

				if(bmpinfo != NULL)
					free(bmpinfo);

				bmpinfo = (uint8_t*) malloc(screen_x_size*screen_y_size*4);
			}

			imghdc = CreateCompatibleDC(hdc);

			if (first_paint == 1)
			{
				int x;

				GetDIBits(imghdc, scr_bmp, 0, screen_y_size, bmpinfo, &di_bmp_info, DIB_RGB_COLORS);

				for (x = 0; x < screen_x_size*screen_y_size*4; x++)
					bmpinfo[x] = 0;
			}

			SetDIBits(imghdc, scr_bmp, 0, screen_y_size, bmpinfo, &di_bmp_info, DIB_RGB_COLORS);
			SelectObject(imghdc, scr_bmp);
			BitBlt(hdc, 0, 0, screen_x_size, screen_y_size, imghdc, 0, 0, SRCCOPY);
			DeleteDC(imghdc);
			EndPaint(hwnd, &paintstruct);

			first_paint = 0;
			break;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			{
				int keycode = wParam;
				WORD val = 0;
				unsigned short shift;
				unsigned short ctrl;

				BYTE keystates[256];
				int i;

				shift = 0;
				ctrl = 0;

				for (i = 0; i < 256; i++)
				{
					short cur = GetKeyState(i);
					keystates[i] = 0;
					if (cur & 1)
					{
						keystates[i] = 1;
					}
					if (cur >> 15)
					{
						keystates[i] |= (1 << 7);
					}
				}

				ToAscii(keycode, MapVirtualKey(keycode, 0), keystates, &val, 0);
				curkey.c = val;
				curkey_k = keycode;

				switch(keycode)
				{
					case VK_F1: curkey_k = RLKEY_F1; break;
					case VK_F2: curkey_k = RLKEY_F2; break;
					case VK_F3: curkey_k = RLKEY_F3; break;
					case VK_F4: curkey_k = RLKEY_F4; break;
					case VK_F5: curkey_k = RLKEY_F5; break;
					case VK_F6: curkey_k = RLKEY_F6; break;
					case VK_F7: curkey_k = RLKEY_F7; break;
					case VK_F8: curkey_k = RLKEY_F8; break;
					case VK_F9: curkey_k = RLKEY_F9; break;
					case VK_F10: curkey_k = RLKEY_F10; break;
					case VK_F11: curkey_k = RLKEY_F11; break;
					case VK_F12: curkey_k = RLKEY_F12; break;
					case VK_LEFT: curkey_k = RLKEY_LEFT; break;
					case VK_RIGHT: curkey_k = RLKEY_RIGHT; break;
					case VK_UP: curkey_k = RLKEY_UP; break;
					case VK_DOWN: curkey_k = RLKEY_DOWN; break;
					case VK_INSERT: curkey_k = RLKEY_INSERT; break;
					case VK_DELETE: curkey_k = RLKEY_DELETE; break;
					case VK_HOME: curkey_k = RLKEY_HOME; break;
					case VK_END: curkey_k = RLKEY_END; break;
					case VK_PRIOR: curkey_k = RLKEY_PRIOR; break;
					case VK_NEXT: curkey_k = RLKEY_NEXT; break;
				}

				if( GetAsyncKeyState(VK_SHIFT) != 0)
					shift |= 1;

				if( GetAsyncKeyState(VK_CONTROL) != 0)
					ctrl |= 1;

				if((lParam & 0x2000) == 0x2000)
					ctrl |= 2;

				if( GetAsyncKeyState(VK_MENU) != 0)
					ctrl |= 2;

				curkey.shift = shift;
				curkey.ctrl = ctrl;

				gotkey = 1;
				break;
			}

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
#endif

void init_window(int x_size, int y_size, char* title)
{
#ifndef xlib
	int real_x_size;
	int real_y_size;
	WNDCLASS wndcls;
#else
	//XWindowAttributes xwa;
#endif
	screen_x_size = x_size;
	screen_y_size = y_size;
#ifdef xlib

	screen_win = XCreateSimpleWindow(screen_display, RootWindow(screen_display, screen_num), 0, 0, screen_x_size, screen_y_size, 0, BlackPixel(screen_display, screen_num), BlackPixel(screen_display, screen_num));
	XStoreName(screen_display, screen_win, title);
	XMapWindow(screen_display, screen_win);

	screen_gc_values.foreground = WhitePixel(screen_display, screen_num);
	screen_gc = XCreateGC(screen_display, screen_win, GCForeground, &screen_gc_values);
	XSelectInput(screen_display, screen_win, ExposureMask | KeyPressMask | StructureNotifyMask);
	XFlush(screen_display);
	XSync(screen_display, False);

	XSynchronize(screen_display, True);

	while (1)
	{
		XNextEvent(screen_display, &event);

		if (event.type == Expose)
		{
			XSync(screen_display, False);
			screen_image = XGetImage(screen_display, screen_win, 0, 0, screen_x_size, screen_y_size, AllPlanes, ZPixmap);
			XSync(screen_display, False);
			break;
		}
	}

	XSynchronize(screen_display, False);

	XFlush(screen_display);
	XSync(screen_display, False);
#else
	real_x_size = screen_x_size + GetSystemMetrics(SM_CXFIXEDFRAME)*2;
	real_y_size = screen_y_size + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + GetSystemMetrics(SM_CYCAPTION);

	wndcls.style = 0;
	wndcls.lpfnWndProc = WndProc;
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hInstance = hinst;
	wndcls.hIcon = LoadIcon(0, IDI_APPLICATION);
	wndcls.hCursor = LoadCursor(0, IDC_ARROW);
	wndcls.hbrBackground = GetStockObject(BLACK_BRUSH);
	wndcls.lpszMenuName = 0;
	wndcls.lpszClassName = "rllib";

	RegisterClass(&wndcls);

	hwnd = CreateWindow("rllib", title, WS_OVERLAPPEDWINDOW, 0, 0, real_x_size, real_y_size, 0, 0, hinst, 0);

	ShowWindow(hwnd, SW_SHOW);
#endif
	update_window();
}

void destroy_window(void)
{
#ifdef xlib
	XDestroyImage(screen_image);
	XCloseDisplay(screen_display);
#else
	DestroyWindow(hwnd);
#endif
}

void update_window(void)
{
#ifdef xlib
	XPutImage(screen_display, screen_win, screen_gc, screen_image, 0, 0, 0, 0, screen_x_size, screen_y_size);
	XFlush(screen_display);
#else
	RedrawWindow(hwnd, 0, 0, RDW_INVALIDATE);
	UpdateWindow(hwnd);
#endif
}

rgbcolor rgb(int r, int g, int b)
{
	rgbcolor color;
	color.r = r;
	color.g = g;
	color.b = b;
	return color;
}

rgbcolor get_pixel(int x, int y)
{
	rgbcolor r;
#ifdef xlib
	uint8_t* ptr = (uint8_t*)screen_image->data + (y * screen_image->bytes_per_line + 4 * x);
#else
	uint8_t* ptr = bmpinfo + (screen_y_size-y-1)*screen_x_size*3 + x*3;
#endif
	r.r = ptr[2];
	r.g = ptr[1];
	r.b = ptr[0];
	return r;
}

void clear(void)
{
	int i, j;
	rgbcolor c;

	c.r = 0;
	c.g = 0;
	c.b = 0;

	for(i=0; i < screen_y_size; i++)
	{
		for(j=0; j<screen_x_size; j++)
		{
			set_pixel(i, j, c);
		}
	}
}

void set_pixel(int x, int y, rgbcolor color)
{
	if(x < 0 || x >= screen_x_size) return;
	if(y < 0 || y >= screen_y_size) return;

#ifdef xlib
	uint8_t* ptr = (uint8_t*)screen_image->data + (y * screen_image->bytes_per_line + 4 * x);
#else
	uint8_t* ptr = bmpinfo + (screen_y_size-y-1)*screen_x_size*3 + x*3;
#endif
	ptr[2] = color.r;
	ptr[1] = color.g;
	ptr[0] = color.b;
}

static key key_to_keycode(key k, int kcode)
{
	key result = k;
	result.is_numpad = 0;

	if(kcode >= RLKEY_START_CODE)
		result.c = kcode;

	return result;
}

key get_key(void)
{
	key result;
#ifndef xlib
	MSG msg;
#endif
	update_window();

	//restart:
#ifdef xlib
	while (1)
	{
		XNextEvent(screen_display, &event);
		switch (event.type)
		{
			default:
				goto gotkey_out;
				break;

			case DestroyNotify:
				driver_shutdown = 1;
				memset(&result, 0x0, sizeof(key));
				return result;
				break;

			case ConfigureNotify:
				if (screen_x_size / font_width_in_pixels() != event.xconfigure.width/font_width_in_pixels() || screen_y_size/ font_height_in_pixels() != event.xconfigure.height/font_height_in_pixels())
				{
					screen_x_size = event.xconfigure.width;
					screen_y_size = event.xconfigure.height;

					screen_image = XGetImage(screen_display, screen_win, 0, 0, screen_x_size, screen_y_size, AllPlanes, ZPixmap);
					stored_display = (displayed_char*) realloc(stored_display, (screen_x_size*screen_y_size*4) * sizeof(displayed_char));
					update_window();

					driver_redraw = 1;
					memset(&result, 0x0, sizeof(key));
					return result;
				}
				else
					update_window();
				break;

			case Expose:
				if (event.xexpose.count)
					break;

				//screen_image = XGetImage(screen_display, screen_win, 0, 0, screen_x_size, screen_y_size, AllPlanes, ZPixmap);
				update_window();
				break;

			case KeyPress:
				{
					char buffer[1];
					KeySym key;
					XComposeStatus crap;
					int charcount;

					charcount = XLookupString(&event.xkey, buffer, 1, &key, &crap);

					curkey.shift = 0;
					curkey.ctrl = 0;

					curkey.c = buffer[0];
					curkey_k = key;

					switch(curkey_k)
					{
						case XK_F1:				curkey_k = RLKEY_F1; break;
						case XK_F2:				curkey_k = RLKEY_F2; break;
						case XK_F3:				curkey_k = RLKEY_F3; break;
						case XK_F4:				curkey_k = RLKEY_F4; break;
						case XK_F5:				curkey_k = RLKEY_F5; break;
						case XK_F6:				curkey_k = RLKEY_F6; break;
						case XK_F7:				curkey_k = RLKEY_F7; break;
						case XK_F8:				curkey_k = RLKEY_F8; break;
						case XK_F9:				curkey_k = RLKEY_F9; break;
						case XK_F10:			curkey_k = RLKEY_F10; break;
						case XK_F11:			curkey_k = RLKEY_F11; break;
						case XK_F12:			curkey_k = RLKEY_F12; break;
						case XK_Left:			curkey_k = RLKEY_LEFT; break;
						case XK_Right:			curkey_k = RLKEY_RIGHT; break;
						case XK_Up:				curkey_k = RLKEY_UP; break;
						case XK_Down:			curkey_k = RLKEY_DOWN; break;
						case XK_Insert:			curkey_k = RLKEY_INSERT; break;
						case XK_Delete:			curkey_k = RLKEY_DELETE; break;
						case XK_Home:			curkey_k = RLKEY_HOME; break;
						case XK_End:			curkey_k = RLKEY_END; break;
						case XK_Page_Up:		curkey_k = RLKEY_PRIOR; break;
						case XK_Page_Down:		curkey_k = RLKEY_NEXT; break;

						case XK_Escape:
						case XK_BackSpace:
						case XK_Tab:
						case XK_Return:
							curkey_k = curkey_k & 0xFF;
							break;
					}

					if(event.xkey.state & ShiftMask)
						curkey.shift = 1;

					if(event.xkey.state & ControlMask)
						curkey.ctrl |= 0x01;

					if(event.xkey.state & Mod1Mask)
						curkey.ctrl |= 0x02;
					if(event.xkey.state & Mod2Mask)
						curkey.ctrl |= 0x02;

					//if(curkey.c != 0)
						goto gotkey_out;
				}
		}
	}
	gotkey_out:
#else
	gotkey = 0;
	curkey.c = 0;
	while (!gotkey)
	{
		if(driver_redraw == 1 )
		{
			memset(&result, 0x0, sizeof(key));
			return result;
		}

		if (!GetMessage(&msg, 0, 0, 0))
		{
			destroy_window();
			exit(0);
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif
	result = key_to_keycode(curkey, curkey_k);

	return result;
}

int colors_different(rgbcolor a, rgbcolor b)
{
	return a.r != b.r || a.g != b.g || a.b != b.b;
}


#ifdef xlib
void sig_abrt(int x)
{
	destroy_window();
}
#endif

extern int rlmain(int argc, char *argv[]);
#ifdef xlib
int main(int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE inst, HINSTANCE x, LPSTR y, int z)
#endif
{
#ifdef xlib
	Screen *s;

	screen_display = XOpenDisplay(0);
	if (!screen_display)
	{
		printf("Can't open display.\n");
		exit(1);
	}
	screen_num = DefaultScreen(screen_display);
	s = XDefaultScreenOfDisplay(screen_display);

	screen_x_size = s->width;
	screen_y_size = s->height;

	signal(SIGABRT, sig_abrt);
	driver_redraw = 0;
	return rlmain(argc, argv);
#else
	char **cli;
	int count;
	char *q, *p;

	DEVMODE dvmdOrig;
	HDC hdc = GetDC(NULL);

	dvmdOrig.dmPelsWidth = GetDeviceCaps(hdc, HORZRES);
	dvmdOrig.dmPelsHeight = GetDeviceCaps(hdc, VERTRES);
	dvmdOrig.dmBitsPerPel = GetDeviceCaps(hdc, BITSPIXEL);
	dvmdOrig.dmDisplayFrequency = GetDeviceCaps(hdc, VREFRESH);
	ReleaseDC(NULL, hdc);

	screen_y_size = dvmdOrig.dmPelsHeight;
	screen_x_size = dvmdOrig.dmPelsWidth;

	hinst = inst;
	driver_redraw = 0;
	count = 0;

	q = y;
	p = q;
	cli = calloc(64, sizeof(char*));

	cli[count++] = strdup("fakeexec");

	while(*p != 0)
	{
		while(*p == 0x20 && *p != 0x0)
			p++;

		q = p;
		while(*p != 0 && *p != 0x20)
			p++;

		if(*p == 0x20 || *p == 0x0)
		{
			cli[count] = calloc(1, (p - q) + 8);
			memmove(cli[count], q, (p-q));
			count++;
		}
	}

	return rlmain(count, cli);
#endif

}

#ifdef xlib
void maximise_window(void)
{
	XEvent xev;
	Atom wm_state = XInternAtom(screen_display, "_NET_WM_STATE", False);
	//Atom fullscreen = XInternAtom(screen_display, "_NET_WM_STATE_FULLSCREEN", False);
	Atom fv = XInternAtom(screen_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	Atom fh = XInternAtom(screen_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = screen_win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 2;
	//xev.xclient.data.l[1] = fullscreen;
	//xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[1] = fh;
	xev.xclient.data.l[2] = fv;
	xev.xclient.data.l[3] = 0;

	XSendEvent(screen_display, DefaultRootWindow(screen_display), False, SubstructureNotifyMask, &xev);

	driver_redraw = 1;
}
#endif
