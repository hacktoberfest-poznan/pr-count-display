#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "image.h"
#include "window.h"

struct Image *image_load(const char *const path) {
	struct Image *img = malloc(sizeof(struct Image));
	if(img == NULL) {
		perror("malloc() failed");
		return NULL;
	}

	SDL_Surface *surf = IMG_Load(path);
	if(surf == NULL) {
		fprintf(stderr, "IMG_Load() failed: %s\n", IMG_GetError());
		free(img);
		return NULL;
	}

	img->tex = SDL_CreateTextureFromSurface(Window.renderer, surf);
	SDL_FreeSurface(surf);

	if(img->tex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface() failed: %s\n", SDL_GetError());
		free(img);
		return NULL;
	}

	img->w = surf->w;
	img->h = surf->h;
	return img;
}

void image_free(struct Image *img) {
	if(img == NULL) return;

	if(img->tex != NULL) {
		SDL_DestroyTexture(img->tex);
		img->tex = NULL;
	}

	free(img);
}
