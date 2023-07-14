#include "headers.h"

static void update_backscreen(uGlobalData *gd, void *v)
{
	if(gd->mode == eMode_Directory)
		DrawAll(gd);
	else if(gd->mode == eMode_Viewer)
		ViewerDrawAll(v);
	else if(gd->mode == eMode_VB_List)
		ListDrawAll(v);
	else
		LogInfo("UNKNOWN DRAW ALL IN MENU");
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

void DrawMenu(uGlobalData *gd, void *v, int menu_to_open)
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

	uMenu **menu;

	menu = GetActMenu(gd);

	uint32_t key;

	for(i=0, midx = 0; i < MAX_MENU; i++)
	{
		if(menu[i] != NULL)
		{
			if(midx == menu_to_open)
				menu_idx = i;

			midx++;
		}
	}

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
			if(menu[i] != NULL)
			{
				if( len + strlen(menu[i]->name) < w )
				{
					sprintf(buff, " %s ", menu[i]->name);

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

						for(j=0, maxwidth=0; j<menu[i]->count; j++)
						{
							int xx;

							xx = strlen(menu[i]->child[j]->name);
							xx += 2;
							xx += GetKeyLength(menu[i]->child[j]->key);

							if( maxwidth < xx )
								maxwidth = xx;
						}

						maxwidth += 4;
						w.offset_row = 1;
						w.offset_col = len2;
						w.width = maxwidth;
						w.height = menu[i]->count + 2;
						w.screen = gd->screen;
						w.gd = gd;
						gd->screen->set_style(STYLE_TITLE);
						gd->screen->draw_border(&w);

						for(j=0; j < menu[i]->count; j++)
						{
							char *p, *q;

							memset(buff, ' ', maxwidth);
							buff[maxwidth-2] = 0;

							memmove(buff + 1, menu[i]->child[j]->name, strlen(menu[i]->child[j]->name));

							p = buff + maxwidth - 2;
							p -= GetKeyLength(menu[i]->child[j]->key);
							q = ConvertKeyToName(menu[i]->child[j]->key);
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

		for(i=0; i<menu[midx]->count;  i++)
		{
			if(key == menu[midx]->child[i]->key)
			{
				if(gd->mode == eMode_Directory)
					ExecuteGlobalString(gd, menu[midx]->child[i]->code);
				else if(gd->mode == eMode_Viewer)
					ExecuteGlobalViewerString(v, menu[midx]->child[i]->code);
				else if(gd->mode == eMode_VB_List)
					ExecuteGlobalListString(v, menu[midx]->child[i]->code);
				else
					LogInfo("XXX TODO");
				return;
			}
		}

		if(key != 0)
		{
			switch(key)
			{
				case ALFC_KEY_LEFT:
					menu_idx -= 1;
					if(menu_idx < 0)
						menu_idx = mcount-1;
					update_backscreen(gd, v);
					menu_sub_idx = 0;
					break;

				case ALFC_KEY_RIGHT:
					menu_idx += 1;
					if(menu_idx >= mcount)
						menu_idx = 0;
					update_backscreen(gd, v);
					menu_sub_idx = 0;
					break;

				case ALFC_KEY_UP:
					menu_sub_idx -= 1;
					if(menu_sub_idx < 0)
						menu_sub_idx = menu[midx]->count-1;
					break;

				case ALFC_KEY_DOWN:
					menu_sub_idx += 1;
					if(menu_sub_idx >= menu[midx]->count)
						menu_sub_idx = 0;
					break;

				case ALFC_KEY_ESCAPE:
				case ALFC_KEY_ESCAPE_ESCAPE:
					return;
					break;

				case ALFC_KEY_ENTER:
					if(gd->mode == eMode_Directory)
						ExecuteGlobalString(gd, menu[midx]->child[menu_sub_idx]->code);
					else if(gd->mode == eMode_Viewer)
						ExecuteGlobalViewerString(v, menu[midx]->child[menu_sub_idx]->code);
					else if(gd->mode == eMode_VB_List)
						ExecuteGlobalListString(v, menu[midx]->child[menu_sub_idx]->code);
					else
						LogInfo("XXX TODO");
					return;
					break;

				default:
					LogInfo("Key was %08X\n", key);
					break;
			}
		}
	}
}


