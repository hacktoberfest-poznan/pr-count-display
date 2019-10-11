#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define INIT_SDL_FLAGS (SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO)
#define INIT_IMG_FLAGS (IMG_INIT_JPG | IMG_INIT_PNG)

void init_libs(void) {
	int err;

	err = SDL_Init(INIT_SDL_FLAGS);
	if(err != 0) {
		fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	err = IMG_Init(INIT_IMG_FLAGS);
	if(err != INIT_IMG_FLAGS) {
		fprintf(stderr, "IMG_Init() failed: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}

	err = TTF_Init();
	if(err != 0) {
		fprintf(stderr, "TTF_Init() failed: %s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
}

void deinit_libs(void) {
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(void) {
	init_libs();
	deinit_libs();

	return 0;
}
