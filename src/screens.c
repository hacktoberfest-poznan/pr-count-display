#include <SDL2/SDL.h>

#include "image.h"
#include "screens.h"
#include "text.h"
#include "utils.h"
#include "window.h"

struct Text *PrCount, *PrHeader;
struct Image *Logo, *CornerLeft, *CornerRight;

struct Text *Clock;

struct Text *HacktoberfestSponsors, *MeetupSponsors, *MediaPatrons;
struct Image *DOandDEV;
struct Image *Sonalake, *Allegro;
struct Image *PoIT, *OSWorld, *Linuxiarze;

void draw_corners(void) {
	SDL_Rect leftDest = (SDL_Rect){
		.x = 0,
		.y = WINDOW_H - CornerLeft->h,
		.w = CornerLeft->w,
		.h = CornerLeft->h
	};
	SDL_RenderCopy(Window.renderer, CornerLeft->tex, NULL, &leftDest);

	SDL_Rect rightDest = (SDL_Rect){
		.x = WINDOW_W - CornerRight->w,
		.y = WINDOW_H - CornerRight->h,
		.w = CornerRight->w,
		.h = CornerRight->h
	};
	SDL_RenderCopy(Window.renderer, CornerRight->tex, NULL, &rightDest);
}

#define LOGO_HEARTBEAT_REST_TIME    5000
#define LOGO_HEARTBEAT_BUILDUP_TIME  200
#define LOGO_HEARTBEAT_ECHO_TIME     800
#define LOGO_HEARTBEAT_TOTAL_TIME  (LOGO_HEARTBEAT_REST_TIME + LOGO_HEARTBEAT_BUILDUP_TIME + LOGO_HEARTBEAT_ECHO_TIME)

#define LOGO_HEARTBEAT_BUILDUP_SCALE  1.05f
#define LOGO_HEARTBEAT_ECHO_SCALE     1.25f
#define LOGO_HEARTBEAT_ECHO_OPACITY   0.35f

void draw_logo(void) {
	float logoScale = -1.0f, echoScale = -1.0f, echoOpacity = 0.0f;

	Uint32 ticks = SDL_GetTicks() % LOGO_HEARTBEAT_TOTAL_TIME;
	if(ticks >= LOGO_HEARTBEAT_REST_TIME) {
		if(ticks < LOGO_HEARTBEAT_REST_TIME + LOGO_HEARTBEAT_BUILDUP_TIME) {
			float stageProgress = progress(ticks, LOGO_HEARTBEAT_REST_TIME, LOGO_HEARTBEAT_REST_TIME + LOGO_HEARTBEAT_BUILDUP_TIME);
			logoScale = 1.0f + ((LOGO_HEARTBEAT_BUILDUP_SCALE - 1.0f) * stageProgress);
		} else {
			float stageProgress = progress(ticks, LOGO_HEARTBEAT_REST_TIME + LOGO_HEARTBEAT_BUILDUP_TIME, LOGO_HEARTBEAT_TOTAL_TIME);
			logoScale = LOGO_HEARTBEAT_BUILDUP_SCALE - ((LOGO_HEARTBEAT_BUILDUP_SCALE - 1.0f) * stageProgress);
			echoScale = LOGO_HEARTBEAT_BUILDUP_SCALE + ((LOGO_HEARTBEAT_ECHO_SCALE - LOGO_HEARTBEAT_BUILDUP_SCALE) * stageProgress);
			echoOpacity = LOGO_HEARTBEAT_ECHO_OPACITY - (LOGO_HEARTBEAT_ECHO_OPACITY * stageProgress);
		}
	}

	SDL_Rect logoDest = (SDL_Rect) {
		.x = (WINDOW_W - Logo->w) / 2,
		.y = 0,
		.w = Logo->w,
		.h = Logo->h
	};
	SDL_Rect echoDest = logoDest;
	
	if(logoScale > 0.0f) scale_rect(&logoDest, logoScale);
	SDL_SetTextureAlphaMod(Logo->tex, 255);
	SDL_RenderCopy(Window.renderer, Logo->tex, NULL, &logoDest);

	if(echoScale > 0.0f) {
		scale_rect(&echoDest, echoScale);
		SDL_SetTextureAlphaMod(Logo->tex, 255 * echoOpacity);
		SDL_RenderCopy(Window.renderer, Logo->tex, NULL, &echoDest);
	}
}

#define CLOCK_MARGIN (WINDOW_H / 100)

void draw_clock(void) {
	static struct tm old_time;
	struct tm now_time = *tm_now();

	if(now_time.tm_min != old_time.tm_min) {
		strftime(Clock->buffer, TEXT_BUFFER_SIZE, "%H:%M", &now_time);
		text_render(Clock, (SDL_Colour){ .r = 127, .g = 127, .b = 127, .a = 255 });
	}

	if(Clock->tex == NULL) return;

	SDL_Rect dest = (SDL_Rect) {
		.x = CLOCK_MARGIN,
		.y = CLOCK_MARGIN,
		.w = Clock->w,
		.h = Clock->h
	};
	SDL_RenderCopy(Window.renderer, Clock->tex, NULL, &dest);
}

void draw_counter(void) {
	if(PrCount->tex == NULL) return;

	SDL_Rect countDest = (SDL_Rect){
		.x = (WINDOW_W - PrCount->w) / 2,
		.y = WINDOW_H - PrCount->h - (WINDOW_H / 25),
		.w = PrCount->w,
		.h = PrCount->h
	};
	SDL_RenderCopy(Window.renderer, PrCount->tex, NULL, &countDest);

	if(PrHeader->tex == NULL) return;

	SDL_Rect headerDest = (SDL_Rect){
		.x = (WINDOW_W - PrHeader->w) / 2,
		.y = countDest.y - PrHeader->h - (PrHeader->h / 5),
		.w = PrHeader->w,
		.h = PrHeader->h
	};
	SDL_RenderCopy(Window.renderer, PrHeader->tex, NULL, &headerDest);
}

void draw_meetup_sponsors(void) {
	SDL_Rect sonaDest, alleDest;

	sonaDest.h = WINDOW_H / 9;
	sonaDest.w = sonaDest.h * Sonalake->w / Sonalake->h;

	alleDest.h = (WINDOW_H / 9) * 11 / 10; // make Allegro slightly larger
	alleDest.w = alleDest.h * Allegro->w / Allegro->h;

	const int center = WINDOW_W / 2 + (sonaDest.w - alleDest.w) / 2;

	sonaDest.x = center - (WINDOW_W / 20) - sonaDest.w;
	sonaDest.y = WINDOW_H - (WINDOW_H / 10) - sonaDest.h;

	alleDest.x = center + (WINDOW_W / 20);
	alleDest.y = WINDOW_H - (WINDOW_H / 10) - (alleDest.h * 9 / 10);

	SDL_RenderCopy(Window.renderer, Sonalake->tex, NULL, &sonaDest);
	SDL_RenderCopy(Window.renderer, Allegro->tex, NULL, &alleDest);

	SDL_Rect headerDest = (SDL_Rect){
		.x = (WINDOW_W - MeetupSponsors->w) / 2,
		.y = (alleDest.y < sonaDest.y ? alleDest.y : sonaDest.y) - MeetupSponsors->h - (MeetupSponsors->h / 5),
		.w = MeetupSponsors->w,
		.h = MeetupSponsors->h
	};
	SDL_RenderCopy(Window.renderer, MeetupSponsors->tex, NULL, &headerDest);
}

void draw_media_patrons(void) {
	SDL_Rect poitDest, oswDest, linDest;

	linDest.w = WINDOW_W / 4;
	linDest.h = linDest.w * Linuxiarze->h / Linuxiarze->w;

	oswDest.w = WINDOW_W / 4;
	oswDest.h = oswDest.w * OSWorld->h / OSWorld->w;

	poitDest.h = linDest.h * 5 / 3;
	poitDest.w = poitDest.h * PoIT->w / PoIT->h;

	linDest.y = WINDOW_H - (WINDOW_H / 10) - linDest.h;
	oswDest.y = WINDOW_H - (WINDOW_H / 10) - oswDest.h;
	poitDest.y = WINDOW_H - (WINDOW_H / 10) - (poitDest.h * 4 / 5);

	const int spaceLeft = WINDOW_W - poitDest.w - oswDest.w - linDest.w;
	const int spacing = spaceLeft / 6;
	const int margin = spacing * 2;

	poitDest.x = margin;
	oswDest.x = poitDest.x + poitDest.w + spacing;
	linDest.x = oswDest.x + oswDest.w + spacing;

	SDL_RenderCopy(Window.renderer, PoIT->tex, NULL, &poitDest);
	SDL_RenderCopy(Window.renderer, OSWorld->tex, NULL, &oswDest);
	SDL_RenderCopy(Window.renderer, Linuxiarze->tex, NULL, &linDest);

	SDL_Rect headerDest = (SDL_Rect){
		.x = (WINDOW_W - MediaPatrons->w) / 2,
		.y = poitDest.y - MediaPatrons->h - (MediaPatrons->h / 5),
		.w = MediaPatrons->w,
		.h = MediaPatrons->h
	};
	SDL_RenderCopy(Window.renderer, MediaPatrons->tex, NULL, &headerDest);
}

void draw_hacktoberfest_sponsors(void) {
	SDL_Rect imageDest = (SDL_Rect){
		.x = (WINDOW_W - DOandDEV->w) / 2,
		.y = WINDOW_H - (WINDOW_H / 25) - DOandDEV->h,
		.w = DOandDEV->w,
		.h = DOandDEV->h
	};
	SDL_RenderCopy(Window.renderer, DOandDEV->tex, NULL, &imageDest);

	SDL_Rect headerDest = (SDL_Rect){
		.x = (WINDOW_W - HacktoberfestSponsors->w) / 2,
		.y = imageDest.y - HacktoberfestSponsors->h - (HacktoberfestSponsors->h / 5),
		.w = HacktoberfestSponsors->w,
		.h = HacktoberfestSponsors->h
	};
	SDL_RenderCopy(Window.renderer, HacktoberfestSponsors->tex, NULL, &headerDest);
}
