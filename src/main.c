#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


int InotifyFD = -1;
int InotifyWatch = -1;

SDL_Window *Window = NULL;
SDL_Renderer *Renderer = NULL;

TTF_Font *Font = NULL;
SDL_Texture *FontTex = NULL;

SDL_Texture *Logo = NULL;

#define WATCHED_FILE_NAME "/tmp/pr-counter"

#define INIT_SDL_FLAGS (SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO)
#define INIT_IMG_FLAGS (IMG_INIT_JPG | IMG_INIT_PNG)

#define FPS_MINIMUM 3
#define FPS_TICKS (1000 / FPS_MINIMUM)

const SDL_Colour BackgroundColour = { .r = 0x15, .g = 0x23, .b = 0x47, .a = 255 };
const SDL_Colour TextColour = { .r = 0xff, .g = 0xf9, .b = 0x22, .a = 255 };

void init_inotify(void) {
	InotifyFD = inotify_init1(IN_NONBLOCK);
	if(InotifyFD == -1) {
		perror("inotify_init1() failed");
		exit(EXIT_FAILURE);
	}

	InotifyWatch = inotify_add_watch(InotifyFD, WATCHED_FILE_NAME, IN_MODIFY);
	if(InotifyWatch == -1) {
		perror("inotify_add_watch() failed");
		exit(EXIT_FAILURE);
	}
}

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

	SDL_Surface *surf = IMG_Load("assets/logo1920.png");
	if(surf == NULL) {
		fprintf(stderr, "IMG_Load() failed: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}

	Logo = SDL_CreateTextureFromSurface(Renderer, surf);
	SDL_FreeSurface(surf);
	if(Logo == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface() failed: %s\n", SDL_GetError());
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

void renderText(const char *const buffer) {
	if(FontTex != NULL) {
		SDL_DestroyTexture(FontTex);
		FontTex = NULL;
	}

	SDL_Surface *surf = TTF_RenderUTF8_Blended(Font, buffer, TextColour);
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

	if(Logo != NULL) {
		int logoW, logoH;
		SDL_QueryTexture(Logo, NULL, NULL, &logoW, &logoH);

		SDL_Rect dest = (SDL_Rect) {
			.x = (windowW - logoW) / 2,
			.y = 0,
			.w = logoW,
			.h = logoH
		};
		SDL_RenderCopy(Renderer, Logo, NULL, &dest);
	}

	if(FontTex != NULL) {
		int textW, textH;
		SDL_QueryTexture(FontTex, NULL, NULL, &textW, &textH);

		SDL_Rect dest = (SDL_Rect) {
			.x = (windowW - textW) / 2,
			.y = windowH - textH - (windowH / 20),
			.w = textW,
			.h = textH
		};
		SDL_RenderCopy(Renderer, FontTex, NULL, &dest);
	}

	SDL_RenderPresent(Renderer);
}

void read_file_contents(char *const buffer, const size_t bufsize) {
	FILE *f = fopen(WATCHED_FILE_NAME, "r");
	if(f == NULL) {
		perror("fopen() failed");
		return;
	}

	char *ret = fgets(buffer, bufsize, f);
	fclose(f);

	if(ret == NULL) {
		perror("fgets() failed");
		return;
	}

	size_t len = strlen(buffer);
	while(len > 0 && buffer[len-1] <= ' ') {
		buffer[--len] = '\0';
	}
}

int check_inotify(void) {
	int any_changes = 0;

	struct inotify_event inev;
	ssize_t bytes;

	while((bytes = read(InotifyFD, &inev, sizeof(struct inotify_event))) > 0) {
		any_changes = 1;
	}

	return any_changes;
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
	init_inotify();
	init_libs();
	SDL_ShowWindow(Window);

	char textBuffer[256];
	read_file_contents(textBuffer, sizeof(textBuffer));
	renderText(textBuffer);
	
	while(!quit_requested()) {
		if(check_inotify()) {
			read_file_contents(textBuffer, sizeof(textBuffer));
			renderText(textBuffer);
		}

		draw_frame();
		SDL_Delay(FPS_TICKS);
	}

	SDL_HideWindow(Window);
	deinit_libs();
	
	return 0;
}
