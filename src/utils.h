#ifndef PRCD_UTILS_H
#define PRCD_UTILS_H

#include <time.h>

#include <SDL2/SDL.h>

extern float progress(const float value, const float min, const float max);

extern void scale_rect(SDL_Rect *rect, const float scale);

extern struct tm *tm_now(void);

#endif
