#ifndef PRCD_TEXT_H
#define PRCD_TEXT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define TEXT_BUFFER_SIZE (256 - 2*sizeof(void*) - 2*sizeof(int))

struct Text {
	TTF_Font *ttf;

	SDL_Texture *tex;
	int w, h;

	char buffer[TEXT_BUFFER_SIZE];
};

extern struct Text *text_init(const int fontSize);
extern void text_free(struct Text *text);

extern int text_render(struct Text *text, const SDL_Colour colour);
extern int text_renderString(struct Text *text, const SDL_Colour colour, const char *const string);

#endif
