#ifndef __HAL_H__
#define __HAL_H__

#include "nes.h"

struct Pixel {
	int x, y;
	int c;
};
typedef struct Pixel Pixel;

struct PixelBuf {
	Pixel buf[264 * 264];
	int size;
};
typedef struct PixelBuf PixelBuf;

extern PixelBuf bg, bbg, fg;

// clear a pixel buffer
#define pixbuf_clean(bf) \
	do { \
		(bf).size = 0; \
	} while (0)

// add a pending pixel into a buffer
#define pixbuf_add(bf, xa, ya, ca) \
	do { \
		if ((xa) < SCREEN_WIDTH && (ya) < SCREEN_HEIGHT) { \
			(bf).buf[(bf).size].x = (xa); \
			(bf).buf[(bf).size].y = (ya); \
			(bf).buf[(bf).size].c = (ca); \
			(bf).size++; \
		} \
	} while (0)
/*
#define pixbuf_add(bf, xa, ya, ca) \
	do { \
		if ((xa) < SCREEN_WIDTH && (ya) < SCREEN_HEIGHT) { \
			draw_pixel(xa, ya, ca);\
			(bf).size++; \
		} \
	} while (0)
*/
// fill the screen with background color
void nes_set_bg_color(int c);

// draw a pixel into current video buffer
void nes_draw_pixel(Pixel *p);

// display and empty the current video buffer
void nes_flip_display();

// initialization
void nes_hal_init();

// query key-press status
int nes_key_state(int b);

#endif
