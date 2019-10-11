#ifndef PRCD_IMAGE_H
#define PRCD_IMAGE_H

#include <SDL2/SDL.h>

struct Image {
	SDL_Texture *tex;
	int w, h;
};

extern struct Image *image_load(const char *const path);
extern void image_free(struct Image *img);

#endif
