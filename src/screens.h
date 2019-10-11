#ifndef PRCD_SCREENS_H
#define PRCD_SCREENS_H

#include "image.h"
#include "text.h"

extern struct Text *PrCount, *PrHeader;
extern struct Image *Logo;

extern struct Text *Clock;

extern struct Text *HacktoberfestSponsors, *MeetupSponsors, *MediaPatrons;
extern struct Image *DOandDEV;
extern struct Image *Sonalake, *Allegro;
extern struct Image *PoIT, *OSWorld, *Linuxiarze;

extern void draw_logo(void);
extern void draw_clock(void);
extern void draw_counter(void);

extern void draw_meetup_sponsors(void);
extern void draw_media_patrons(void);
extern void draw_hacktoberfest_sponsors(void);

#endif
