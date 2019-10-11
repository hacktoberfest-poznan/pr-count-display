#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "image.h"
#include "text.h"
#include "window.h"

int InotifyFD = -1;
int InotifyWatch = -1;

struct Text *PrCount, *PrHeader;
struct Image *Logo;

struct Text *HacktoberfestSponsors, *MeetupSponsors, *MediaPatrons;
struct Image *DOandDEV;
struct Image *Sonalake, *Allegro;
struct Image *PoIT, *OSWorld, *Linuxiarze;

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
		if(errno != ENOENT) {
			perror("inotify_add_watch() failed");
			exit(EXIT_FAILURE);
		}

		FILE *f = fopen(WATCHED_FILE_NAME, "w");
		if(f == NULL) {
			perror("fopen() failed");
			exit(EXIT_FAILURE);
		}

		if(fwrite("0\n", 1, 2, f) < 2) {
			perror("fwrite() failed");
			exit(EXIT_FAILURE);
		}

		fclose(f); // Ignore close failures

		InotifyWatch = inotify_add_watch(InotifyFD, WATCHED_FILE_NAME, IN_MODIFY);
		if(InotifyWatch == -1) {
			perror("inotify_add_watch() failed");
			exit(EXIT_FAILURE);
		}
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

	Logo = image_load("assets/logo1920.png");
	if(Logo == NULL) exit(EXIT_FAILURE);

	DOandDEV = image_load("assets/DO-and-DEV.png");
	if(DOandDEV == NULL) exit(EXIT_FAILURE);

	Sonalake = image_load("assets/sonalake.png");
	if(Sonalake == NULL) exit(EXIT_FAILURE);

	Allegro = image_load("assets/allegro.png");
	if(Allegro == NULL) exit(EXIT_FAILURE);

	PoIT = image_load("assets/poit.png");
	if(PoIT == NULL) exit(EXIT_FAILURE);

	OSWorld = image_load("assets/osworld.png");
	if(OSWorld == NULL) exit(EXIT_FAILURE);

	Linuxiarze = image_load("assets/linuxiarze.png");
	if(Linuxiarze == NULL) exit(EXIT_FAILURE);

	PrCount = text_init(Window.h / 5);
	if(PrCount == NULL) exit(EXIT_FAILURE);

	PrHeader = text_init(Window.h / 10);
	if(PrHeader == NULL) exit(EXIT_FAILURE);
	
	MeetupSponsors = text_init(Window.h / 12);
	if(MeetupSponsors == NULL) exit(EXIT_FAILURE);
	
	MediaPatrons = text_init(Window.h / 12);
	if(MediaPatrons == NULL) exit(EXIT_FAILURE);
	
	HacktoberfestSponsors = text_init(Window.h / 12);
	if(HacktoberfestSponsors == NULL) exit(EXIT_FAILURE);
	
	text_renderString(PrHeader, TextColour, "Pull Request count:");
	text_renderString(MeetupSponsors, TextColour, "POZNAN MEETUP SPONSORS");
	text_renderString(MediaPatrons, TextColour, "MEDIA PATRONS");
	text_renderString(HacktoberfestSponsors, TextColour, "HACKTOBERFEST SPONSORS");
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

void draw_counter(void) {
	if(PrCount->tex == NULL) return;

	SDL_Rect countDest = (SDL_Rect){
		.x = (Window.w - PrCount->w) / 2,
		.y = Window.h - PrCount->h - (Window.h / 25),
		.w = PrCount->w,
		.h = PrCount->h
	};
	SDL_RenderCopy(Window.renderer, PrCount->tex, NULL, &countDest);

	if(PrHeader->tex == NULL) return;

	SDL_Rect headerDest = (SDL_Rect){
		.x = (Window.w - PrHeader->w) / 2,
		.y = countDest.y - PrHeader->h - (PrHeader->h / 5),
		.w = PrHeader->w,
		.h = PrHeader->h
	};
	SDL_RenderCopy(Window.renderer, PrHeader->tex, NULL, &headerDest);
}

void draw_meetup_sponsors(void) {
	SDL_Rect sonaDest, alleDest;

	sonaDest.h = Window.h / 9;
	sonaDest.w = sonaDest.h * Sonalake->w / Sonalake->h;

	alleDest.h = (Window.h / 9) * 11 / 10; // make Allegro slightly larger
	alleDest.w = alleDest.h * Allegro->w / Allegro->h;

	const int center = Window.w / 2 + (sonaDest.w - alleDest.w) / 2;

	sonaDest.x = center - (Window.w / 20) - sonaDest.w;
	sonaDest.y = Window.h - (Window.h / 10) - sonaDest.h;

	alleDest.x = center + (Window.w / 20);
	alleDest.y = Window.h - (Window.h / 10) - (alleDest.h * 9 / 10);

	SDL_RenderCopy(Window.renderer, Sonalake->tex, NULL, &sonaDest);
	SDL_RenderCopy(Window.renderer, Allegro->tex, NULL, &alleDest);

	SDL_Rect headerDest = (SDL_Rect){
		.x = (Window.w - MeetupSponsors->w) / 2,
		.y = (alleDest.y < sonaDest.y ? alleDest.y : sonaDest.y) - MeetupSponsors->h - (MeetupSponsors->h / 5),
		.w = MeetupSponsors->w,
		.h = MeetupSponsors->h
	};
	SDL_RenderCopy(Window.renderer, MeetupSponsors->tex, NULL, &headerDest);
}

void draw_media_patrons(void) {
	SDL_Rect poitDest, oswDest, linDest;

	linDest.w = Window.w / 4;
	linDest.h = linDest.w * Linuxiarze->h / Linuxiarze->w;

	oswDest.w = Window.w / 4;
	oswDest.h = oswDest.w * OSWorld->h / OSWorld->w;

	poitDest.h = linDest.h * 5 / 3;
	poitDest.w = poitDest.h * PoIT->w / PoIT->h;

	linDest.y = Window.h - (Window.h / 10) - linDest.h;
	oswDest.y = Window.h - (Window.h / 10) - oswDest.h;
	poitDest.y = Window.h - (Window.h / 10) - (poitDest.h * 4 / 5);

	const int spaceLeft = Window.w - poitDest.w - oswDest.w - linDest.w;
	const int spacing = spaceLeft / 6;
	const int margin = spacing * 2;

	poitDest.x = margin;
	oswDest.x = poitDest.x + poitDest.w + spacing;
	linDest.x = oswDest.x + oswDest.w + spacing;

	SDL_RenderCopy(Window.renderer, PoIT->tex, NULL, &poitDest);
	SDL_RenderCopy(Window.renderer, OSWorld->tex, NULL, &oswDest);
	SDL_RenderCopy(Window.renderer, Linuxiarze->tex, NULL, &linDest);

	SDL_Rect headerDest = (SDL_Rect){
		.x = (Window.w - MediaPatrons->w) / 2,
		.y = poitDest.y - MediaPatrons->h - (MediaPatrons->h / 5),
		.w = MediaPatrons->w,
		.h = MediaPatrons->h
	};
	SDL_RenderCopy(Window.renderer, MediaPatrons->tex, NULL, &headerDest);
}

void draw_hacktoberfest_sponsors(void) {
	SDL_Rect imageDest = (SDL_Rect){
		.x = (Window.w - DOandDEV->w) / 2,
		.y = Window.h - (Window.h / 10) - DOandDEV->h,
		.w = DOandDEV->w,
		.h = DOandDEV->h
	};
	SDL_RenderCopy(Window.renderer, DOandDEV->tex, NULL, &imageDest);

	SDL_Rect headerDest = (SDL_Rect){
		.x = (Window.w - HacktoberfestSponsors->w) / 2,
		.y = imageDest.y - HacktoberfestSponsors->h - (HacktoberfestSponsors->h / 5),
		.w = HacktoberfestSponsors->w,
		.h = HacktoberfestSponsors->h
	};
	SDL_RenderCopy(Window.renderer, HacktoberfestSponsors->tex, NULL, &headerDest);
}

void draw_frame(void) {
	SDL_SetRenderDrawColor(Window.renderer, BackgroundColour.r, BackgroundColour.g, BackgroundColour.b, BackgroundColour.a);
	SDL_RenderClear(Window.renderer);

	SDL_Rect dest = (SDL_Rect) {
		.x = (Window.w - Logo->w) / 2,
		.y = 0,
		.w = Logo->w,
		.h = Logo->h
	};
	SDL_RenderCopy(Window.renderer, Logo->tex, NULL, &dest);

	Uint32 seconds = SDL_GetTicks() / 1000;
	switch((seconds / 2) % 4){
		case 0: draw_counter(); break;
		case 1: draw_meetup_sponsors(); break;
		case 2: draw_media_patrons(); break;
		case 3: draw_hacktoberfest_sponsors(); break;
	}

	SDL_RenderPresent(Window.renderer);
}

void read_file_contents(void) {
	FILE *f = fopen(WATCHED_FILE_NAME, "r");
	if(f == NULL) {
		perror("fopen() failed");
		return;
	}

	char *ret = fgets(PrCount->buffer, TEXT_BUFFER_SIZE, f);
	fclose(f);

	if(ret == NULL) {
		perror("fgets() failed");
		return;
	}

	size_t len = strlen(PrCount->buffer);
	while(len > 0 && PrCount->buffer[len-1] <= ' ') {
		PrCount->buffer[--len] = '\0';
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
	image_free(Logo);
	text_free(PrCount);
	text_free(PrHeader);

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
	text_render(PrCount, TextColour);
	
	while(!quit_requested()) {
		if(check_inotify()) {
			read_file_contents();
			text_render(PrCount, TextColour);
		}

		draw_frame();
		SDL_Delay(FPS_TICKS);
	}

	SDL_HideWindow(Window.window);
	deinit_libs();
	
	return 0;
}
