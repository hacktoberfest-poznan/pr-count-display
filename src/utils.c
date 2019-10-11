#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "utils.h"

float progress(const float value, const float min, const float max) {
	return (value - min) / (max - min);
}

void scale_rect(SDL_Rect *rect, const float scale) {
	const int oldW = rect->w;
	const int oldH = rect->h;

	rect->w = rect->w * scale;
	rect->h = rect->h * scale;

	rect->x -= (rect->w - oldW) / 2;
	rect->y -= (rect->h - oldH) / 2;
}

struct tm *tm_now(void) {
	time_t now = time(NULL);
	return localtime(&now);
}
