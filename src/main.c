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

struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	int w, h;
} Window;

#define TEXT_BUFFER_SIZE 256

struct {
	TTF_Font *ttf;

	char buffer[TEXT_BUFFER_SIZE];

	SDL_Texture *tex;
	int w, h;
} Text;

struct {
	SDL_Texture *tex;
	int w, h;
} Logo;

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

	Window.window = SDL_CreateWindow(
		"Pull Request Count Display",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		0, 0,
		SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_HIDDEN
	);
	if(Window.window == NULL) {
		fprintf(stderr, "SDL_OpenWindow() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GetWindowSize(Window.window, &Window.w, &Window.h);

	Window.renderer = SDL_CreateRenderer(Window.window, -1, SDL_RENDERER_ACCELERATED);
	if(Window.renderer == NULL) {
		fprintf(stderr, "SDL_CreateWindow.renderer() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_Surface *surf = IMG_Load("assets/logo1920.png");
	if(surf == NULL) {
		fprintf(stderr, "IMG_Load() failed: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}

	Logo.w = surf->w;
	Logo.h = surf->h;
	Logo.tex = SDL_CreateTextureFromSurface(Window.renderer, surf);
	SDL_FreeSurface(surf);
	if(Logo.tex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface() failed: %s\n", SDL_GetError());
	}

	Text.ttf = TTF_OpenFont("assets/Orbitron-Regular.ttf", Window.h / 5);
	if(Text.ttf == NULL) {
		fprintf(stderr, "TTF_OpenFont() failed: %s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}
}

int quit_requested(void) {
	SDL_Event ev;
	while(SDL_PollEvent(&ev) > 0) {
		if(ev.type == SDL_QUIT)
			return 1;
		if(ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
			return 1;

		if(ev.type == SDL_WINDOWEVENT && ev.window.type == SDL_WINDOWEVENT_RESIZED) {
			Window.w = ev.window.data1;
			Window.h = ev.window.data2;
		}
	}
	
	return 0;
}

void renderText(void) {
	if(Text.tex != NULL) {
		SDL_DestroyTexture(Text.tex);
		Text.tex = NULL;
	}

	SDL_Surface *surf = TTF_RenderUTF8_Blended(Text.ttf, Text.buffer, TextColour);
	if(surf == NULL) {
		fprintf(stderr, "TTF_RenderUTF8_Blended() failed: %s\n", TTF_GetError());
		return;
	}

	Text.w = surf->w;
	Text.h = surf->h;
	Text.tex = SDL_CreateTextureFromSurface(Window.renderer, surf);
	SDL_FreeSurface(surf);
	
	if(Text.tex == NULL) {
		fprintf(stderr, "SDL_CreateTextureFromSurface() failed: %s\n", SDL_GetError());
	}
}

void draw_frame(void) {
	SDL_SetRenderDrawColor(Window.renderer, BackgroundColour.r, BackgroundColour.g, BackgroundColour.b, BackgroundColour.a);
	SDL_RenderClear(Window.renderer);

	if(Logo.tex != NULL) {
		SDL_Rect dest = (SDL_Rect) {
			.x = (Window.w - Logo.w) / 2,
			.y = 0,
			.w = Logo.w,
			.h = Logo.h
		};
		SDL_RenderCopy(Window.renderer, Logo.tex, NULL, &dest);
	}

	if(Text.tex != NULL) {
		SDL_Rect dest = (SDL_Rect) {
			.x = (Window.w - Text.w) / 2,
			.y = Window.h - Text.h - (Window.h / 20),
			.w = Text.w,
			.h = Text.h
		};
		SDL_RenderCopy(Window.renderer, Text.tex, NULL, &dest);
	}

	SDL_RenderPresent(Window.renderer);
}

void read_file_contents(void) {
	FILE *f = fopen(WATCHED_FILE_NAME, "r");
	if(f == NULL) {
		perror("fopen() failed");
		return;
	}

	char *ret = fgets(Text.buffer, TEXT_BUFFER_SIZE, f);
	fclose(f);

	if(ret == NULL) {
		perror("fgets() failed");
		return;
	}

	size_t len = strlen(Text.buffer);
	while(len > 0 && Text.buffer[len-1] <= ' ') {
		Text.buffer[--len] = '\0';
	}
}

int check_inotify(void) {
	int any_changes = 0;

	struct inotify_event inev;
	while(read(InotifyFD, &inev, sizeof(struct inotify_event)) > 0) {
		any_changes = 1;
	}

	return any_changes;
}

void deinit_libs(void) {
	SDL_DestroyTexture(Logo.tex);
	SDL_DestroyTexture(Text.tex);
	TTF_CloseFont(Text.ttf);

	SDL_DestroyRenderer(Window.renderer);
	SDL_DestroyWindow(Window.window);

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(void) {
	init_inotify();
	init_libs();
	SDL_ShowWindow(Window.window);

	read_file_contents();
	renderText();
	
	while(!quit_requested()) {
		if(check_inotify()) {
			read_file_contents();
			renderText();
		}

		draw_frame();
		SDL_Delay(FPS_TICKS);
	}

	SDL_HideWindow(Window.window);
	deinit_libs();
	
	return 0;
}
