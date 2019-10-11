#include <errno.h>
#include <stdio.h>

#include "text.h"
#include "window.h"

struct Text *text_init(const int fontSize) {
	struct Text *text = malloc(sizeof(struct Text));
	if(text == NULL) {
		perror("malloc() failed");
		return NULL;
	}

	text->ttf = TTF_OpenFont("assets/Orbitron-Regular.ttf", fontSize);
	if(text->ttf == NULL) {
		fprintf(stderr, "TTF_OpenFont() failed: %s\n", TTF_GetError());
		return NULL;
	}

	text->tex = NULL;
	text->buffer[0] = '\0';
	text->w = text->h = -1;

	return text;
}

void text_free(struct Text *text) {
	if(text == NULL) return;

	if(text->tex != NULL) {
		SDL_DestroyTexture(text->tex);
		text->tex = NULL;
	}

	if(text->ttf != NULL) {
		TTF_CloseFont(text->ttf);
		text->ttf = NULL;
	}

	free(text);
}

int text_render(struct Text *text, const SDL_Colour colour) {
	if(text->tex != NULL) {
		SDL_DestroyTexture(text->tex);
		text->tex = NULL;
	}

	SDL_Surface *surf = TTF_RenderUTF8_Blended(text->ttf, text->buffer, colour);
	if(surf == NULL) {
		fprintf(stderr, "TTF_RenderUTF8_Blended() failed: %s\n", TTF_GetError());
		return 0;
	}

	text->w = surf->w;
	text->h = surf->h;
	text->tex = SDL_CreateTextureFromSurface(Window.renderer, surf);
	SDL_FreeSurface(surf);

	if(text->tex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface() failed: %s\n", SDL_GetError());
		return 0;
	}

	return 1;
}
