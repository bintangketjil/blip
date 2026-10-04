// blip.c

#include <stdlib.h>
#include <stdint.h>
#include "blip.h"


// ++++ Framebuffer +++++
Framebuffer blip_init_fb(int width, int height, ColorRGB *pixels)
{
    Framebuffer fb = {
        .width = width,
        .height = height,
        .pixels = pixels
    };

    return fb;
}

void blip_clear_fb(Framebuffer *fb, ColorRGB color)
{
    for (int i = 0; i < fb->width * fb->height; i++) {
        fb->pixels[i] = color;
    }
}


// ++++ Pixel +++++
static inline int in_bound(const Framebuffer *fb, int x, int y)
{
    return x >= 0 && x < fb->width && y >= 0 && y < fb->height;
}

void blip_put_pixel(Framebuffer *fb, int x, int y, ColorRGB color)
{
    if (!in_bound(fb, x, y)) return;

    fb->pixels[y * fb->width + x] = color;
}

ColorRGB blip_get_pixel(const Framebuffer *fb, int x, int y)
{
    if (!in_bound(fb, x, y)) {
        return (ColorRGB) {
            0, 0, 0, 0
        };
    }

    return fb->pixels[y * fb->width + x];
}


// ++++ Draw +++++
void blip_draw_line(Framebuffer *fb, Point a, Point b, ColorRGB color)
{
    // Bresenham line, all octants.
    //
    // dx = |x1 - x0|, dy = -|y1 - y0|, sx/sy = step direction.
    // The ideal line is dx*(y - y0) + dy*(x - x0) = 0, and err is that
    // expression evaluated at the current pixel (its distance from the line)
    // Each iteration, with e2 = 2*err (avoids half-pixel fractions)
    //   e2 >= dy  ->  step x, err += dy
    //   e2 <= dx  ->  step y, err += dx
    // Both can fire at once, which gives a diagonal step.

    int x0 = a.x, y0 = a.y;
    int x1 = b.x, y1 = b.y;

    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1; // step direction in x
    int sy = y0 < y1 ? 1 : -1; // step direction in y
    int err = dx + dy;

    for (;;) {
        blip_put_pixel(fb, x0, y0, color);

        if (x0 == x1 && y0 == y1)  break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            // step in x
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            // step in y
            err += dx;
            y0 += sy;
        }
    }
}
