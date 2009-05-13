//
// SGEO : simple hack to dump a font as png.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_byteorder.h>
#include <png.h>


SDL_Surface *screen;
TTF_Font *fntX;


int IMG_SavePNG(const char *file, SDL_Surface *surf,int compression){
	SDL_RWops *fp;
	int ret;

	fp=SDL_RWFromFile(file,"wb");

	if( fp == NULL ) {
		return (-1);
	}

	ret=IMG_SavePNG_RW(fp,surf,compression);
	SDL_RWclose(fp);
	return ret;
}

static void png_write_data(png_structp png_ptr,png_bytep data, png_size_t length){
	SDL_RWops *rp = (SDL_RWops*) png_get_io_ptr(png_ptr);
	SDL_RWwrite(rp,data,1,length);
}

int IMG_SavePNG_RW(SDL_RWops *src, SDL_Surface *surf,int compression){
	png_structp png_ptr;
	png_infop info_ptr;
	SDL_PixelFormat *fmt=NULL;
	SDL_Surface *tempsurf=NULL;
	int ret,funky_format,used_alpha;
	unsigned int i,temp_alpha;
	png_colorp palette;
	Uint8 *palette_alpha=NULL;
	png_byte **row_pointers=NULL;
	png_ptr=NULL;info_ptr=NULL;palette=NULL;ret=-1;
	funky_format=0;

	if( !src || !surf) {
		goto savedone; /* Nothing to do. */
	}

	row_pointers=(png_byte **)malloc(surf->h * sizeof(png_byte*));
	if (!row_pointers) {
		SDL_SetError("Couldn't allocate memory for rowpointers");
		goto savedone;
	}

	png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
	if (!png_ptr){
		SDL_SetError("Couldn't allocate memory for PNG file");
		goto savedone;
	}
	info_ptr= png_create_info_struct(png_ptr);
	if (!info_ptr){
		SDL_SetError("Couldn't allocate image information for PNG file");
		goto savedone;
	}
	/* setup custom writer functions */
	png_set_write_fn(png_ptr,(voidp)src,png_write_data,NULL);

	if (setjmp(png_jmpbuf(png_ptr))){
		SDL_SetError("Unknown error writing PNG");
		goto savedone;
	}

	if(compression>Z_BEST_COMPRESSION)
		compression=Z_BEST_COMPRESSION;

	if(compression == Z_NO_COMPRESSION) // No compression
	{
		png_set_filter(png_ptr,0,PNG_FILTER_NONE);
		png_set_compression_level(png_ptr,Z_NO_COMPRESSION);
	}
        else if(compression<0) // Default compression
		png_set_compression_level(png_ptr,Z_DEFAULT_COMPRESSION);
        else
		png_set_compression_level(png_ptr,compression);

	fmt=surf->format;
	if(fmt->BitsPerPixel==8){ /* Paletted */
		png_set_IHDR(png_ptr,info_ptr,
			surf->w,surf->h,8,PNG_COLOR_TYPE_PALETTE,
			PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT);
		palette=(png_colorp) malloc(fmt->palette->ncolors * sizeof(png_color));
		if (!palette) {
			SDL_SetError("Couldn't create memory for palette");
			goto savedone;
		}
		for (i=0;i<fmt->palette->ncolors;i++) {
			palette[i].red=fmt->palette->colors[i].r;
			palette[i].green=fmt->palette->colors[i].g;
			palette[i].blue=fmt->palette->colors[i].b;
		}
		png_set_PLTE(png_ptr,info_ptr,palette,fmt->palette->ncolors);
		if (surf->flags&SDL_SRCCOLORKEY) {
			palette_alpha=(Uint8 *)malloc((fmt->colorkey+1)*sizeof(Uint8));
			if (!palette_alpha) {
				SDL_SetError("Couldn't create memory for palette transparency");
				goto savedone;
			}
			/* FIXME: memset? */
			for (i=0;i<(fmt->colorkey+1);i++) {
				palette_alpha[i]=255;
			}
			palette_alpha[fmt->colorkey]=0;
			png_set_tRNS(png_ptr,info_ptr,palette_alpha,fmt->colorkey+1,NULL);
		}
	}else{ /* Truecolor */
		if (fmt->Amask) {
			png_set_IHDR(png_ptr,info_ptr,
				surf->w,surf->h,8,PNG_COLOR_TYPE_RGB_ALPHA,
				PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
		} else {
			png_set_IHDR(png_ptr,info_ptr,
				surf->w,surf->h,8,PNG_COLOR_TYPE_RGB,
				PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
		}
	}
	png_write_info(png_ptr, info_ptr);

	if (fmt->BitsPerPixel==8) { /* Paletted */
		for(i=0;i<surf->h;i++){
			row_pointers[i]= ((png_byte*)surf->pixels) + i*surf->pitch;
		}
		if(SDL_MUSTLOCK(surf)){
			SDL_LockSurface(surf);
		}
		png_write_image(png_ptr, row_pointers);
		if(SDL_MUSTLOCK(surf)){
			SDL_UnlockSurface(surf);
		}
	}else{ /* Truecolor */
		if(fmt->BytesPerPixel==3){
			if(fmt->Amask){ /* check for 24 bit with alpha */
				funky_format=1;
			}else{
				/* Check for RGB/BGR/GBR/RBG/etc surfaces.*/
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				if(fmt->Rmask!=0xFF0000
				|| fmt->Gmask!=0x00FF00
				|| fmt->Bmask!=0x0000FF){
#else
				if(fmt->Rmask!=0x0000FF
				|| fmt->Gmask!=0x00FF00
				|| fmt->Bmask!=0xFF0000){
#endif
					funky_format=1;
				}
			}
		}else if (fmt->BytesPerPixel==4){
			if (!fmt->Amask) { /* check for 32bit but no alpha */
				funky_format=1;
			}else{
				/* Check for ARGB/ABGR/GBAR/RABG/etc surfaces.*/
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				if(fmt->Rmask!=0xFF000000
				|| fmt->Gmask!=0x00FF0000
				|| fmt->Bmask!=0x0000FF00
				|| fmt->Amask!=0x000000FF){
#else
				if(fmt->Rmask!=0x000000FF
				|| fmt->Gmask!=0x0000FF00
				|| fmt->Bmask!=0x00FF0000
				|| fmt->Amask!=0xFF000000){
#endif
					funky_format=1;
				}
			}
		}else{ /* 555 or 565 16 bit color */
			funky_format=1;
		}
		if (funky_format) {
			/* Allocate non-funky format, and copy pixeldata in*/
			if(fmt->Amask){
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				tempsurf = SDL_CreateRGBSurface(SDL_SWSURFACE, surf->w, surf->h, 24,
										0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
				tempsurf = SDL_CreateRGBSurface(SDL_SWSURFACE, surf->w, surf->h, 24,
										0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
			}else{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				tempsurf = SDL_CreateRGBSurface(SDL_SWSURFACE, surf->w, surf->h, 24,
										0xff0000, 0x00ff00, 0x0000ff, 0x00000000);
#else
				tempsurf = SDL_CreateRGBSurface(SDL_SWSURFACE, surf->w, surf->h, 24,
										0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000);
#endif
			}
			if(!tempsurf){
				SDL_SetError("Couldn't allocate temp surface");
				goto savedone;
			}
			if(surf->flags&SDL_SRCALPHA){
				temp_alpha=fmt->alpha;
				used_alpha=1;
				SDL_SetAlpha(surf,0,255); /* Set for an opaque blit */
			}else{
				used_alpha=0;
			}
			if(SDL_BlitSurface(surf,NULL,tempsurf,NULL)!=0){
				SDL_SetError("Couldn't blit surface to temp surface");
				SDL_FreeSurface(tempsurf);
				goto savedone;
			}
			if (used_alpha) {
				SDL_SetAlpha(surf,SDL_SRCALPHA,(Uint8)temp_alpha); /* Restore alpha settings*/
			}
			for(i=0;i<tempsurf->h;i++){
				row_pointers[i]= ((png_byte*)tempsurf->pixels) + i*tempsurf->pitch;
			}
			if(SDL_MUSTLOCK(tempsurf)){
				SDL_LockSurface(tempsurf);
			}
			png_write_image(png_ptr, row_pointers);
			if(SDL_MUSTLOCK(tempsurf)){
				SDL_UnlockSurface(tempsurf);
			}
			SDL_FreeSurface(tempsurf);
		} else {
			for(i=0;i<surf->h;i++){
				row_pointers[i]= ((png_byte*)surf->pixels) + i*surf->pitch;
			}
			if(SDL_MUSTLOCK(surf)){
				SDL_LockSurface(surf);
			}
			png_write_image(png_ptr, row_pointers);
			if(SDL_MUSTLOCK(surf)){
				SDL_UnlockSurface(surf);
			}
		}
	}

	png_write_end(png_ptr, NULL);
	ret=0; /* got here, so nothing went wrong. YAY! */

savedone: /* clean up and return */
	png_destroy_write_struct(&png_ptr,&info_ptr);
	if (palette) {
		free(palette);
	}
	if (palette_alpha) {
		free(palette_alpha);
	}
	if (row_pointers) {
		free(row_pointers);
	}
	return ret;
}



Uint32 MapColour(SDL_Surface *x, uint32_t c)
{
	return SDL_MapRGB(x->format, (c>>16)&0xFF/*R*/, (c>>8)&0xFF/*G*/, c&0xFF/*B*/);
}


int main( int argc, char* argv[] )
{
	SDL_Color clrFg = {255,255,255,0};
	SDL_Rect rcDest = {0,0,0,0};
	SDL_Surface *sText;
	SDL_Rect r;
	Uint32 color;

	uint8_t buff[512];
	int i, j;
	int fsize = 8;

	if( SDL_Init(SDL_INIT_VIDEO)==(-1) )
	{
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
		exit(-1);
	}


	screen = SDL_SetVideoMode(800, 1600, 32, SDL_SWSURFACE );
	if( screen == NULL )
	{
		fprintf(stderr, "Couldn't set %ix%ix%i video mode: %s\n", 800, 600, 32, SDL_GetError());
 		exit(1);
	}
	atexit(SDL_Quit);
	SDL_ShowCursor( SDL_DISABLE );

	TTF_Init();
	atexit(TTF_Quit);

	for(fsize = 8; fsize < 33; fsize++)
	{
		char fn[64];

		fntX = TTF_OpenFontIndex( "/home/sgeorge/.fonts/8514oem.fon", fsize, 0);
		if(!fntX)
		{
			printf("TTF_OpenFont: %s\n", TTF_GetError());
			exit(0);
		}
		TTF_SetFontStyle(fntX, TTF_STYLE_NORMAL);

		color = MapColour(screen, 0);
		r.x = 0;
		r.y = 0;
		r.w = screen->w;
		r.h = screen->h;
		SDL_FillRect(screen, &r, color);

		rcDest.y = 0;

		for(i=0; i<16; i++)
		{
			int w, h;

			for(j=0; j<16; j++)
			{
				buff[j] = (i*16) + j;
				if(buff[j] == 0)
					buff[j] = ' ';
			}

			buff[16] = 0;
			TTF_SizeText(fntX, buff, &w, &h);

			sText = TTF_RenderText_Blended( fntX, buff, clrFg);
			SDL_BlitSurface(sText, NULL, screen, &rcDest );

			rcDest.y += TTF_FontHeight(fntX); //h;
		}

		sText = TTF_RenderText_Solid( fntX, buff, clrFg);
		SDL_BlitSurface(sText, NULL, screen, &rcDest );

		sprintf(fn, "font%02i.png", fsize);

		screen->w = rcDest.w;
		screen->h = rcDest.h * 16;
		IMG_SavePNG(fn, screen, 9);

		screen->w = 800;
		screen->h = 600;

		SDL_FreeSurface(sText);
	}

	TTF_CloseFont( fntX );

	exit(0);
}

