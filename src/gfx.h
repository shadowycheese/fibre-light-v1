#ifndef GFX_H
#define GFX_H

#include <Arduino.h>

uint8_t set_pallete(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

extern void draw_pixel(uint8_t x, uint8_t y, uint8_t colour);

extern void fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colour);

extern void draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t colour);

extern void draw_line(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t colour);

extern void commit_frame();

extern void setup_gfx();

#endif