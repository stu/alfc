#include "headers.h"

static void update_backscreen(uGlobalData *gd)
{
	DrawAll(gd);
}

static int GetKeyLength(uint32_t key)
{
	int xx;

	if(key >= ALFC_KEY_ALT && key < ALFC_KEY_CTRL)
		xx = 4; // ALT+
	else if(key >= ALFC_KEY_CTRL)
		xx = 5; // CTRL+
	else switch(key)
	{
		default:
			xx = 2;
			break;

		case ALFC_KEY_F01:
		case ALFC_KEY_F02:
		case ALFC_KEY_F03:
		case ALFC_KEY_F04:
		case ALFC_KEY_F05:
		case ALFC_KEY_F06:
		case ALFC_KEY_F07:
		case ALFC_KEY_F08:
		case ALFC_KEY_F09:
			xx = 3;
			break;

		case ALFC_KEY_F10:
		case ALFC_KEY_F11:
		case ALFC_KEY_F12:
		case ALFC_KEY_INS:
		case ALFC_KEY_DEL:
		case ALFC_KEY_END:
		case ALFC_KEY_TAB:
			xx = 4; //
			break;


		case ALFC_KEY_HOME:
			xx = 5;
			break;

		case ALFC_KEY_ENTER:
		case ALFC_KEY_SPACE:
			xx = 6;
			break;
	}

	return xx;
}

void DrawMenu(uGlobalData *gd)
{
	char buff[128];
	int i;
	int w;
	int len;
	int mi, msi;
	int mcount;
	int midx = 0;
	int menu_idx = 0;
	int menu_sub_idx = 0;

	uint32_t key;


	w = gd->screen->get_screen_width();

	while(1)
	{
		memset(buff, ' ', 128);

		gd->screen->set_style(STYLE_TITLE);
		gd->screen->set_cursor(1,1);
		gd->screen->erase_eol();

		midx = 0;
		for(i=0, len=0, mi=0, msi=0; i < MAX_MENU; i++)
		{
			if(gd->menu[i] != NULL)
			{
				if( len + strlen(gd->menu[i]->name) < w )
				{
					sprintf(buff, " %s ", gd->menu[i]->name);

					if(mi == menu_idx)
					{
						gd->screen->set_style(STYLE_HIGHLIGHT);
						midx = i;
					}
					else
						gd->screen->set_style(STYLE_TITLE);

					gd->screen->set_cursor(1, 1+len);
					gd->screen->print_abs(buff);

					if(mi == menu_idx)
					{
						int j;
						int maxwidth;
						int len2 = len;
						uWindow w;

						len += strlen(buff);


						for(j=0, maxwidth=0; j<gd->menu[i]->count; j++)
						{
							int xx;

							xx = strlen(gd->menu[i]->child[j]->name);
							xx += 2;
							xx += GetKeyLength(gd->menu[i]->child[j]->key);

							if( maxwidth < xx )
								maxwidth = xx;
						}

						maxwidth += 4;
						w.offset_row = 1;
						w.offset_col = len2;
						w.width = maxwidth;
						w.height = gd->menu[i]->count + 2;
						w.screen = gd->screen;
						w.gd = gd;
						gd->screen->set_style(STYLE_TITLE);
						gd->screen->draw_border(&w);

						for(j=0; j<gd->menu[i]->count; j++)
						{
							char *p, *q;

							memset(buff, ' ', maxwidth);
							buff[maxwidth-2] = 0;

							memmove(buff + 1, gd->menu[i]->child[j]->name, strlen(gd->menu[i]->child[j]->name));

							p = buff + maxwidth - 2;
							p -= GetKeyLength(gd->menu[i]->child[j]->key);
							q = ConvertKeyToName(gd->menu[i]->child[j]->key);
							sprintf(p, "%s ", q);

							gd->screen->set_cursor(3 + j, 2+len2);
							gd->screen->set_style(STYLE_TITLE);
							if(msi == menu_sub_idx)
								gd->screen->set_style(STYLE_HIGHLIGHT);

							gd->screen->print_abs(buff);

							msi += 1;
						}
					}
					else
						len += strlen(buff);

					mi += 1;
				}
			}
		}

		mcount = mi;

		key = gd->screen->get_keypress();
		switch(key)
		{
			case ALFC_KEY_LEFT:
				menu_idx -= 1;
				if(menu_idx < 0)
					menu_idx = mcount-1;
				update_backscreen(gd);
				break;
			case ALFC_KEY_RIGHT:
				menu_idx += 1;
				if(menu_idx >= mcount)
					menu_idx = 0;
				update_backscreen(gd);
				break;
			case ALFC_KEY_UP:
				menu_sub_idx -= 1;
				if(menu_sub_idx < 0)
					menu_sub_idx = gd->menu[midx]->count-1;
				break;
			case ALFC_KEY_DOWN:
				menu_sub_idx += 1;
				if(menu_sub_idx >= gd->menu[midx]->count)
					menu_sub_idx = 0;
				break;

			case ALFC_KEY_ESCAPE:
				return;
				break;

			default:
				break;
		}
	}
}


