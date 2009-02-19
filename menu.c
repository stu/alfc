
#include "headers.h"

void DrawMenu(uGlobalData *gd)
{
	char buff[128];
	int i;
	int w;
	int len;
	int mi, msi;

	int menu_idx = 0;
	int menu_sub_idx = 0;


	w = gd->screen->get_screen_height();
	memset(buff, ' ', 128);

	for(i=0, len=0, mi=0, msi=0; i < MAX_MENU; i++)
	{
		if(gd->menu[i] != NULL)
		{
			if( len + strlen(gd->menu[i]->name) < w )
			{
				sprintf(buff, " %s ", gd->menu[i]->name);

				if(mi == menu_idx)
					gd->screen->set_style(STYLE_HIGHLIGHT);
				else
					gd->screen->set_style(STYLE_TITLE);

				gd->screen->set_cursor(1, 1+len);
				gd->screen->print_abs(buff);

				if(mi == menu_idx)
				{
					int j;
					int maxwidth;
					int len2 = len;

					for(j=0, maxwidth=0; j<gd->menu[i]->count; j++)
					{
						if( maxwidth < strlen(gd->menu[i]->child[j]->name) )
							maxwidth = strlen(gd->menu[i]->child[j]->name);
					}
					maxwidth += 2;

					memset(buff, ' ', maxwidth);
					buff[maxwidth] = 0;
					gd->screen->set_cursor(2, 1+len2);
					gd->screen->print_abs(buff);

					for(j=0; j<gd->menu[i]->count; j++)
					{

						memset(buff, ' ', maxwidth);
						buff[maxwidth] = 0;

						memmove(buff + 1, gd->menu[i]->child[j]->name, strlen(gd->menu[i]->child[j]->name));

						gd->screen->set_cursor(3 + j, 1+len2);

						gd->screen->set_style(STYLE_TITLE);
						if(msi == menu_sub_idx)
							gd->screen->set_style(STYLE_HIGHLIGHT);

						gd->screen->print_abs(buff);

						msi += 1;
					}

					memset(buff, ' ', maxwidth);
					buff[maxwidth] = 0;
					gd->screen->set_cursor(3+j, 1+len2);
					gd->screen->print_abs(buff);
				}

				len += strlen(buff);
				mi += 1;
			}
		}
	}

	menu_sub_idx++;

	gd->screen->get_keypress();
}

