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
#else
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "rllib.h"

static int screen_x_size;
static int screen_y_size;

int window_width_in_pixels() {
  return screen_x_size;
}

int window_height_in_pixels() {
  return screen_y_size;
}

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
static char* bmpinfo;

static key curkey;
static int curkey_k;
static int gotkey;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &paintstruct);
            if (first_paint)
            {
                scr_bmp = CreateCompatibleBitmap(hdc, screen_x_size, screen_y_size);

                di_bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                di_bmp_info.bmiHeader.biWidth = screen_x_size;
                di_bmp_info.bmiHeader.biHeight = screen_y_size;
                di_bmp_info.bmiHeader.biPlanes = 1;
                di_bmp_info.bmiHeader.biBitCount = 24;
                di_bmp_info.bmiHeader.biCompression = 0;

                bmpinfo = (char*) malloc(screen_x_size * screen_y_size * 3);
            }

            imghdc = CreateCompatibleDC(hdc);

            if (first_paint)
            {
                int x;

                GetDIBits(imghdc, scr_bmp, 0, screen_y_size, bmpinfo, &di_bmp_info, DIB_RGB_COLORS);
                for (x = 0; x < screen_x_size*screen_y_size*3; x++)
                {
                    bmpinfo[x] = 0;
                }
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
                    case VK_F1:
                    case VK_F2:
                    case VK_F3:
                    case VK_F4:
                    case VK_F5:
                    case VK_F6:
                    case VK_F7:
                    case VK_F8:
                    case VK_F9:
                    case VK_F10:
                    case VK_F11:
                    case VK_F12:
                    case VK_F13:
                    case VK_F14:
                    case VK_F15:
                    case VK_F16:
                    case VK_F17:
                    case VK_F18:
                    case VK_F19:
                    case VK_F20:
                    case VK_F21:
                    case VK_F22:
                    case VK_F23:
                    case VK_F24:

                    case VK_LEFT:
                    case VK_RIGHT:
                    case VK_UP:
                    case VK_DOWN:
                    case VK_INSERT:
                    case VK_DELETE:
                    case VK_HOME:
                    case VK_END:
                    case VK_PRIOR: // page up
                    case VK_NEXT: // page down
                         curkey_k |= 0x800000;
                         break;
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
#endif
    screen_x_size = x_size;
    screen_y_size = y_size;

#ifdef xlib
    screen_display = XOpenDisplay(0);
    if (!screen_display)
    {
        printf("Can't open display.\n");
        exit(1);
    }
    screen_num = DefaultScreen(screen_display);
    screen_win = XCreateSimpleWindow(screen_display, RootWindow(screen_display, screen_num), 0, 0, screen_x_size, screen_y_size, 0, BlackPixel(screen_display, screen_num), BlackPixel(screen_display, screen_num));
    XStoreName(screen_display, screen_win, title);
    XMapWindow(screen_display, screen_win);

    screen_gc_values.foreground = WhitePixel(screen_display, screen_num);

    screen_gc = XCreateGC(screen_display, screen_win, GCForeground, &screen_gc_values);
    XSelectInput(screen_display, screen_win, ExposureMask | KeyPressMask);

    XFlush(screen_display);

    while (1)
    {
        XNextEvent(screen_display, &event);
        if (event.type == Expose)
        {
            screen_image = XGetImage(screen_display, screen_win, 0, 0, screen_x_size, screen_y_size, AllPlanes, ZPixmap);
            break;
        }
    }
    XFlush(screen_display);
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
    init_colors();
    update_window();
}

void destroy_window()
{
#ifdef xlib
    XDestroyImage(screen_image);
    XCloseDisplay(screen_display);
#else
    DestroyWindow(hwnd);
#endif
}

void update_window()
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
    char* ptr = screen_image->data + (y * screen_image->bytes_per_line + 4 * x);
#else
    char* ptr = bmpinfo + (screen_y_size-y-1)*screen_x_size*3 + x*3;
#endif
    r.r = ptr[2];
    r.g = ptr[1];
    r.b = ptr[0];
    return r;
}

void set_pixel(int x, int y, rgbcolor color)
{
#ifdef xlib
    char* ptr = screen_image->data + (y * screen_image->bytes_per_line + 4 * x);
#else
    char* ptr = bmpinfo + (screen_y_size-y-1)*screen_x_size*3 + x*3;
#endif
    ptr[2] = color.r;
    ptr[1] = color.g;
    ptr[0] = color.b;
}

static key key_to_keycode(key k, int kcode)
{
    key result = k;
	result.is_numpad = 0;

    if(kcode >= 0x800000)
        result.c = kcode;

    //if (k.c == 13)
    //    result.c = '\n';

    return result;
}

key get_key(void)
{
    key result;
#ifndef xlib
    MSG msg;
#endif
    update_window();
    restart:
#ifdef xlib
    while (1)
    {
        XNextEvent(screen_display, &event);
        switch (event.type)
        {
            case Expose:
                if (event.xexpose.count)
                    break;
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
					if(event.xkey.state & ShiftMask)
						curkey.shift = 1;

					if(event.xkey.state & ControlMask)
						curkey.ctrl |= 0x01;

					if(event.xkey.state & Mod1Mask)
						curkey.ctrl |= 0x02;

                    curkey.c = buffer[0];
                    curkey_k = key;

					switch(curkey_k)
					{
						case XK_F1:
						case XK_F2:
						case XK_F3:
						case XK_F4:
						case XK_F5:
						case XK_F6:
						case XK_F7:
						case XK_F8:
						case XK_F9:
						case XK_F10:
						case XK_F11:
						case XK_F12:
						case XK_Left:
						case XK_Right:
						case XK_Up:
						case XK_Down:
						case XK_Insert:
						case XK_Delete:
						case XK_Home:
						case XK_End:
						case XK_Page_Up:
						case XK_Page_Down:
						//case XK_BackSpace:
							curkey_k |= 0x800000;
							break;

						case XK_Escape:
						case XK_BackSpace:
						case XK_Tab:
						case XK_Return:
							curkey.c = curkey_k & 0xFF;
							break;
					}

                    goto gotkey;
                }
        }
    }
    gotkey:
#else
    gotkey = 0;
    curkey.c = 0;
    while (!gotkey)
    {
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
    if (result.c == 0)
    {
        goto restart;
    }

    return result;
}

int colors_different(rgbcolor a, rgbcolor b)
{
    return a.r != b.r || a.g != b.g || a.b != b.b;
}

extern int rlmain(void);
#ifdef xlib
int main()
#else
int WINAPI WinMain(HINSTANCE inst, HINSTANCE x, LPSTR y, int z)
#endif
{
#ifndef xlib
    hinst = inst;
#endif
    return rlmain();
}
