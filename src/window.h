#ifndef PRCD_WINDOW_H
#define PRCD_WINDOW_H

#include <SDL2/SDL.h>

struct Window {
	SDL_Window *window;
	SDL_Renderer *renderer;

	int w, h;
};

extern struct Window Window;

#endif
