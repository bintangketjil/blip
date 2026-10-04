#ifndef BLIP_H
#define BLIP_H

// blip.h
//
// # Framebuffer
// - width x height,
// - RGBA8,
// - row-major,
// - origin top-left, y down.
// - The caller of init_fb owns it. The library never frees memory it did not allocate.

#include <stdint.h>


// ++++ Types +++++
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} ColorRGB;

typedef struct {
    int width;
    int height;
    ColorRGB *pixels;
} Framebuffer;


// ++++ Framebuffer +++++
Framebuffer blip_init_fb(int width, int height, ColorRGB *pixels);
void blip_clear_fb(Framebuffer *fb, ColorRGB color);


// ++++ Pixel +++++
void blip_put_pixel(Framebuffer *fb, int x, int y, ColorRGB color);
ColorRGB blip_get_pixel(const Framebuffer *fb, int x, int y);


// ++++ Math +++++
// [X] Vec2
// [ ] Vec3

typedef struct {
    int x;
    int y;
} Point;


// ++++ Draw +++++
// [X] draw_line
// [ ] draw_path
// [ ] draw_rect
// [ ] draw_circle
// [ ] draw_tri
// [ ] draw_poly

void blip_draw_line(Framebuffer *fb, Point a, Point b, ColorRGB color);


// ++++ Shapes +++++
// [ ] fill_rect
// [ ] fill_circle
// [ ] fill_tri
// [ ] fill_poly

#endif
