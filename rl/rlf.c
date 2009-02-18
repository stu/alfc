#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <png.h>

SDL_Surface *screen;
SDL_Surface *x;

uint8_t *buff;

SDL_Surface *LoadPNG(char *fn)
{
	return IMG_Load(fn);
}

int main( int argc, char* argv[] )
{
	int q1, q2, j, k;
	SDL_Surface *z;
	int a;
	int w, h;
	FILE *fp;

	int offs;

	if( SDL_Init(SDL_INIT_VIDEO)==(-1) )
	{
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		exit(-1);
	}
	screen = SDL_SetVideoMode(800, 600, 8, SDL_SWSURFACE );
	if( screen == NULL )
	{
		fprintf(stderr, "Couldn't set %ix%ix%i video mode: %s\n", 800, 600, 32, SDL_GetError());
 		exit(1);
	}
	atexit(SDL_Quit);

	x = LoadPNG("xfont_med.png");
	z = SDL_DisplayFormat(x);
	//SDL_FreeSurface(x);

	w = 10;
	h = 20;

	buff = malloc(64 + (256 * (w*h)) );
	memset(buff, 0x0, 64 + (256 * (w*h)));

	buff[0] = 0xf3;
	buff[1] = 0x9e;
	buff[2] = 'R';
	buff[3] = 'L';
	buff[4] = 'F';
	buff[5] = w;
	buff[6] = h;

	a = 7;
	offs = 0;

	for(q1 = 0; q1<16; q1++)
	{
		uint8_t *ppp;

		for(q2=0; q2<16; q2++)
		{
			for(j=0; j<h; j++)
			{
				ppp = z->pixels;
				ppp += q1 * 16 * w * h;
				ppp += q2 * w;
				ppp += j * (16*w);

				for(k=0; k<w; k++)
				{
					if( ppp[ k ] != 0)
						buff[a] |= (0x01 << offs);

					offs += 1;
					if(offs == 8)
					{
						a++;
						buff[a] = 0;
						offs = 0;
					}
				}
			}
		}
	}

	fp = fopen("font2.rlf", "wb");
	fwrite(buff, 1, 8 + (256*w*h)/8, fp);
	fclose(fp);

	SDL_FreeSurface(x);

	exit(0);
}

