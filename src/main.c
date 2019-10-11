#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


SDL_Window *Window = NULL;
SDL_Renderer *Renderer = NULL;

TTF_Font *Font = NULL;
SDL_Texture *FontTex = NULL;

#define INIT_SDL_FLAGS (SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO)
#define INIT_IMG_FLAGS (IMG_INIT_JPG | IMG_INIT_PNG)

#define FPS_MINIMUM 3
#define FPS_TICKS (1000 / FPS_MINIMUM)

const SDL_Colour BackgroundColour = { .r = 0x15, .g = 0x23, .b = 0x47, .a = 255 };
const SDL_Colour TextColour = { .r = 0xff, .g = 0xf9, .b = 0x22, .a = 255 };

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

	Window = SDL_CreateWindow(
		"Pull Request Count Display",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		0, 0,
		SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_HIDDEN
	);
	if(Window == NULL) {
		fprintf(stderr, "SDL_OpenWindow() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
	if(Renderer == NULL) {
		fprintf(stderr, "SDL_CreateRenderer() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	int height;
	SDL_GetWindowSize(Window, NULL, &height);

	Font = TTF_OpenFont("assets/Orbitron-Regular.ttf", height / 5);
	if(Font == NULL) {
		fprintf(stderr, "TTF_OpenFont() failed: %s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
}

int quit_requested(void) {
	SDL_Event ev;
	while(SDL_PollEvent(&ev) > 0) {
		if(ev.type == SDL_QUIT)
			return 1;
		else if(ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
			return 1;
	}
	
	return 0;
}

void renderText(void) {
	if(FontTex != NULL) {
		SDL_DestroyTexture(FontTex);
		FontTex = NULL;
	}

	SDL_Surface *surf = TTF_RenderUTF8_Blended(Font, "0", TextColour);
	if(surf == NULL) {
		fprintf(stderr, "TTF_RenderUTF8_Blended() failed: %s\n", TTF_GetError());
		return;
	}

	FontTex = SDL_CreateTextureFromSurface(Renderer, surf);
	SDL_FreeSurface(surf);
	
	if(FontTex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface() failed: %s\n", SDL_GetError());
	}
}

void draw_frame(void) {
	SDL_SetRenderDrawColor(Renderer, BackgroundColour.r, BackgroundColour.g, BackgroundColour.b, BackgroundColour.a);
	SDL_RenderClear(Renderer);

	int windowW, windowH;
	SDL_GetWindowSize(Window, &windowW, &windowH);

	if(FontTex != NULL) {
		int textW, textH;
		SDL_QueryTexture(FontTex, NULL, NULL, &textW, &textH);

		SDL_Rect dest = (SDL_Rect) {
			.x = (windowW - textW) / 2,
			.y = windowH - textH - (windowH / 10),
			.w = textW,
			.h = textH
		};
		SDL_RenderCopy(Renderer, FontTex, NULL, &dest);
	}

	SDL_RenderPresent(Renderer);
}

void deinit_libs(void) {
	SDL_DestroyTexture(FontTex);
	TTF_CloseFont(Font);

	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(void) {
	init_libs();
	SDL_ShowWindow(Window);

	renderText();
	while(!quit_requested()) {
		draw_frame();
		SDL_Delay(FPS_TICKS);
	}

	SDL_HideWindow(Window);
	deinit_libs();
	
	return 0;
}
