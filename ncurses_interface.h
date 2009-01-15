#ifndef _NCURSES_INTERFACE_H
#define _NCURSES_INTERFACE_H
#ifdef __cplusplus
extern "C"{
#endif

#include <curses.h>

#define NO_KEY          -1
#define UP_KEY          0x100
#define DOWN_KEY        0x101
#define LEFT_KEY        0x102
#define RIGHT_KEY       0x103
#define INS_KEY         0x104
#define DEL_KEY         0x105
#define HOME_KEY        0x106
#define END_KEY         0x107
#define PGUP_KEY        0x108
#define PGDN_KEY        0x109
#define ENTER_KEY       0x110
#define DELETE_KEY      0x111
#define BACKSPACE_KEY   0x112
#define TAB_KEY         0x113
#define ALT0_KEY        0x200
#define ALT_KEY(n)      (ALT0_KEY+(n))
#define F0_KEY          0x400
#define F_KEY(n)        (F0_KEY+(n))
#define CTRL_KEY(x) 	x-'A'+1


extern uScreenDriver screen_ncurses;

#ifdef __cplusplus
}
#endif
#endif // _NCURSES_INTERFACE_H

