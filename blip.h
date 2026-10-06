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
#include <stdbool.h>


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


// ++++ View modes +++++
typedef enum {
    BLIP_VIEW_FILL,
    BLIP_VIEW_WIRE,
    BLIP_VIEW_FILL_WIRE
} BlipView;

void blip_set_view(BlipView mode);


// ++++ Framebuffer +++++
Framebuffer blip_init_fb(int width, int height, ColorRGB *pixels);
void blip_clear_fb(Framebuffer *fb, ColorRGB color);


// ++++ Pixel +++++
void blip_put_pixel(Framebuffer *fb, int x, int y, ColorRGB color);
ColorRGB blip_get_pixel(const Framebuffer *fb, int x, int y);


// ++++ Math +++++
// [X] Point
// [ ] Vec2
// [ ] Vec3

#define BLIP_TAU 6.28318530718f

typedef struct {
    int x;
    int y;
} Point;


// ++++ Draw +++++
// [X] draw_line
// [X] draw_rect
// [X] draw_path
// [X] draw_tri
// [X] draw_poly
// [X] draw_circle
// [ ] draw_bezier

void blip_draw_line(Framebuffer *fb, Point a, Point b, ColorRGB color);
void blip_draw_rect(Framebuffer *fb, Point origin, int w, int h, ColorRGB color);
void blip_draw_path(Framebuffer *fb, const Point *pts, int count, bool closed, ColorRGB color);
void blip_draw_tri(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color);
void blip_draw_poly(Framebuffer *fb, const Point *pts, int count, ColorRGB color);
void blip_draw_circle(Framebuffer *fb, Point c, int r, ColorRGB color);
// non-core
void circle_by_angle(Framebuffer *fb, Point c, float r, int n, ColorRGB color);


// ++++ Shapes +++++
// [X] fill_rect
// [X] fill_circle
// [X] fill_tri
// [X] fill_poly_convex
// [ ] fill_poly_concave
// [X] fill_quad

void blip_fill_rect(Framebuffer *fb, Point origin, int w, int h, ColorRGB color);
void blip_fill_circle(Framebuffer *fb, Point c, int r, ColorRGB color);
void blip_fill_tri(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color);
void blip_fill_poly_convex(Framebuffer *fb, const Point *pts, int count, ColorRGB color);
void blip_fill_quad(Framebuffer *fb, Point a, Point b, Point c, Point d, ColorRGB color);

#endif
