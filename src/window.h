#ifndef PRCD_WINDOW_H
#define PRCD_WINDOW_H

#include <SDL2/SDL.h>

#define WINDOW_W 1920
#define WINDOW_H 1080

struct Window {
	SDL_Window *window;
	SDL_Renderer *renderer;
};

extern struct Window Window;

#endif
