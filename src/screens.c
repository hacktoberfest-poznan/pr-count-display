#include <SDL2/SDL.h>

#include "image.h"
#include "screens.h"
#include "text.h"
#include "window.h"

struct Text *PrCount, *PrHeader;
struct Image *Logo;

struct Text *HacktoberfestSponsors, *MeetupSponsors, *MediaPatrons;
struct Image *DOandDEV;
struct Image *Sonalake, *Allegro;
struct Image *PoIT, *OSWorld, *Linuxiarze;

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
