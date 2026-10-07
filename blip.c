// blip.c

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "blip.h"


// ++++ Framebuffer +++++
Framebuffer blip_init_fb(int width, int height, ColorRGB *pixels) {
    Framebuffer fb = {
        .width = width,
        .height = height,
        .pixels = pixels
    };
    return fb;
}

void blip_clear_fb(Framebuffer *fb, ColorRGB color) {
    for (int i = 0; i < fb->width * fb->height; i++) {
        fb->pixels[i] = color;
    }
}


// ++++ Pixel +++++
static inline int in_bound(const Framebuffer *fb, int x, int y) {
    return x >= 0 && x < fb->width && y >= 0 && y < fb->height;
}

void blip_put_pixel(Framebuffer *fb, int x, int y, ColorRGB color) {
    if (!in_bound(fb, x, y)) return;

    fb->pixels[y * fb->width + x] = color;
}

ColorRGB blip_get_pixel(const Framebuffer *fb, int x, int y) {
    if (!in_bound(fb, x, y)) {
        return (ColorRGB) {
            0, 0, 0, 0
        };
    }

    return fb->pixels[y * fb->width + x];
}


// ++++ Draw +++++
static BlipView g_view = BLIP_VIEW_FILL;

void blip_set_view(BlipView mode) {
    g_view = mode;
}


void blip_draw_line(Framebuffer *fb, Point a, Point b, ColorRGB color) {
    // Bresenham line, all octants.
    //
    // dx = |x1 - x0|, dy = -|y1 - y0|, sx/sy = step direction.
    // The ideal line is dx*(y - y0) + dy*(x - x0) = 0, and err is that
    // expression evaluated at the current pixel (its distance from the line)
    // Each iteration, with e2 = 2*err (avoids half-pixel fractions)
    //   e2 >= dy  ->  step x, err += dy
    //   e2 <= dx  ->  step y, err += dx
    // Both can fire at once, which gives a diagonal step.
    if (!fb) return;

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

void blip_draw_rect(Framebuffer *fb, Point origin, int w, int h, ColorRGB color) {
    // size is the number of pixels covered,
    // so rect at (0,0) with w 10, h 10 covers pixels 0-9
    if (!fb) return;

    if (w <= 0 || h <= 0) return;

    int x0 = origin.x;
    int y0 = origin.y;
    int x1 = origin.x + w - 1;
    int y1 = origin.y + h - 1;
    blip_draw_line(fb, (Point) {
        x0, y0
    }, (Point) {
        x1, y0
    }, color); // top
    blip_draw_line(fb, (Point) {
        x1, y0
    }, (Point) {
        x1, y1
    }, color); // right
    blip_draw_line(fb, (Point) {
        x1, y1
    }, (Point) {
        x0, y1
    }, color); // bottom
    blip_draw_line(fb, (Point) {
        x0, y1
    }, (Point) {
        x0, y0
    }, color); // left
}

void blip_draw_path(Framebuffer *fb, const Point *pts, int count, bool closed, ColorRGB color) {
    if (!fb || !pts) return;

    if (count < 1) return;

    if (count == 1) {
        blip_put_pixel(fb, pts[0].x, pts[0].y, color);
        return;
    }

    for (int i = 0; i < count - 1; i++) {
        blip_draw_line(fb, pts[i], pts[i + 1], color);
    }

    if (closed && count > 2) {
        blip_draw_line(fb, pts[count - 1], pts[0], color);
    }
}

void blip_draw_tri(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color) {
    if (!fb) return;

    Point pts[3] =  {a, b, c};
    blip_draw_path(fb, pts, 3, true, color);
}

void blip_draw_poly(Framebuffer *fb, const Point *pts, int count, ColorRGB color) {
    if (!fb || pts == NULL) return;

    blip_draw_path(fb, pts, count, true, color);
}

void blip_draw_circle(Framebuffer *fb, Point c, int r, ColorRGB color) {
    // Midpoint circle. Walk one octant from (0, r) to x == y,
    // mirror each point into all 8 octants.
    // d is f(x+1, y-1/2) = (x+1)^2 + (y-1/2)^2 - r^2, scaled to integers:
    //   d < 0  -> keep y,  d += 2x + 1
    //   d >= 0 -> y--,     d += 2(x - y) + 1   (after x++)
    if (!fb || r < 0) return;

    int x = 0;
    int y = r;
    int d = 1 - r;

    while (x <= y) {
        blip_put_pixel(fb, c.x + x, c.y + y, color);
        blip_put_pixel(fb, c.x - x, c.y + y, color);
        blip_put_pixel(fb, c.x + x, c.y - y, color);
        blip_put_pixel(fb, c.x - x, c.y - y, color);
        blip_put_pixel(fb, c.x + y, c.y + x, color);
        blip_put_pixel(fb, c.x - y, c.y + x, color);
        blip_put_pixel(fb, c.x + y, c.y - x, color);
        blip_put_pixel(fb, c.x - y, c.y - x, color);
        x++;

        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}

void circle_by_angle(Framebuffer *fb, Point c, float r, int n, ColorRGB color) {
    if (!fb || n < 1) return;

    Point pts[n];

    for (int k = 0; k < n; k++) {
        float t = k * (BLIP_TAU / n);
        pts[k].x = c.x + (int)lroundf(r * sinf(t));
        pts[k].y = c.y - (int)lroundf(r * cosf(t));
    }

    blip_draw_poly(fb, pts, n, color);
}


// ++++ Shapes +++++
static void hline(Framebuffer *fb, int x0, int x1, int y, ColorRGB color) {
    if (!fb || !fb->pixels) return;

    if (y < 0 || y >= fb->height) return;

    if (x0 > x1) {
        int t = x0;
        x0 = x1;
        x1 = t;
    }

    if (x0 < 0) x0 = 0;

    if (x1 >= fb->width) x1 = fb->width - 1;

    if (x0 > x1) return;

    ColorRGB *row = fb->pixels + y * fb->width;

    for (int x = x0; x <= x1; x++) row[x] = color;
}

static int edge_x(Point a, Point b, int y) {
    // x where edge a->b crosses row y. Requires a.y < b.y
    if (a.y == b.y) return a.x;

    float t = (float)(y - a.y) / (float)(b.y - a.y);
    return a.x + (int)floorf((b.x - a.x) * t + 0.5f);
}

static void fill_rect_spans(Framebuffer *fb, Point origin, int w, int h, ColorRGB color) {
    if (!fb || w <= 0 || h <= 0) return;

    int y0 = origin.y < 0 ? 0 : origin.y;
    int y1 = origin.y + h > fb->height ? fb->height : origin.y + h;

    for (int y = y0; y < y1; y++) {
        hline(fb, origin.x, origin.x + w - 1, y, color);
    }
}

void blip_fill_rect(Framebuffer *fb, Point origin, int w, int h, ColorRGB color) {
    if (g_view == BLIP_VIEW_WIRE) {
        blip_draw_rect(fb, origin, w, h, (ColorRGB) {
            255, 0, 0, 255
        });
        return;
    }

    fill_rect_spans(fb, origin, w, h, color);
}

static void fill_circle_spans(Framebuffer *fb, Point c, int r, ColorRGB color) {
    if (!fb || r < 0) return;

    int x = 0;
    int y = r;
    int d = 1 - r;

    while (x <= y) {
        hline(fb, c.x - x, c.x + x, c.y + y, color);
        hline(fb, c.x - x, c.x + x, c.y - y, color);
        hline(fb, c.x - y, c.x + y, c.y + x, color);
        hline(fb, c.x - y, c.x + y, c.y - x, color);
        x++;

        if (d < 0) {
            d += 2 * x + 1;
        } else {
            y--;
            d += 2 * (x - y) + 1;
        }
    }
}

void blip_fill_circle(Framebuffer *fb, Point c, int r, ColorRGB color) {
    if (g_view == BLIP_VIEW_WIRE) {
        blip_draw_circle(fb, c, r, (ColorRGB) {
            255, 0, 0, 255
        });
        return;
    }

    fill_circle_spans(fb, c, r, color);
}

static void fill_tri_spans(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color) {
    // Scanline fill, top-left rule: rows [a.y, c.y) and spans [lo, hi),
    // so the bottom row and right end of each span are not drawn.
    // Edges are always evaluated top to bottom (smaller y to larger),
    // so triangles sharing an edge compute identical x values for it.
    if (!fb) return;

    // sort by y: a = top, b = middle, c = bottom
    Point t;

    if (a.y > b.y) {
        t = a;
        a = b;
        b = t;
    }

    if (b.y > c.y) {
        t = b;
        b = c;
        c = t;
    }

    if (a.y > b.y) {
        t = a;
        a = b;
        b = t;
    }

    if (a.y == c.y) return; // zero height, nothing to fill

    int y0 = a.y < 0 ? 0 : a.y;
    int y1 = c.y > fb->height ? fb->height : c.y; // exclusive

    for (int y = y0; y < y1; y++) {
        int x_long = edge_x(a, c, y);
        int x_short = (y < b.y) ? edge_x(a, b, y) : edge_x(b, c, y);
        int lo = x_long < x_short ? x_long : x_short;
        int hi = x_long < x_short ? x_short : x_long;

        if (hi > lo) hline(fb, lo, hi - 1, y, color); // hline is inclusive, span is [lo, hi)
    }
}

void blip_fill_tri(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color) {
    if (!fb) return;

    switch (g_view) {
    case BLIP_VIEW_FILL:
        fill_tri_spans(fb, a, b, c, color);
        break;

    case BLIP_VIEW_WIRE:
        blip_draw_tri(fb, a, b, c, color);
        break;

    case BLIP_VIEW_FILL_WIRE:
        fill_tri_spans(fb, a, b, c, color);
        blip_draw_tri(fb, a, b, c, (ColorRGB) {
            255, 255, 255, 255
        });
        break;
    }
}

void blip_fill_poly_convex(Framebuffer *fb, const Point *pts, int count, ColorRGB color) {
    if (!fb || !pts || count < 3) return;

    for (int i = 1; i < count - 1; i++) {
        blip_fill_tri(fb, pts[0], pts[i], pts[i + 1], color);
    }
}

void blip_fill_quad(Framebuffer *fb, Point a, Point b, Point c, Point d, ColorRGB color) {
    Point pts[4] = {a, b, c, d};
    blip_fill_poly_convex(fb, pts, 4, color);
}
