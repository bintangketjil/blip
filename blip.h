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


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Hash +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
uint32_t blip_hash(int x, int y, uint32_t seed);
float blip_hash01(int x, int y, uint32_t seed); // [0, 1]


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Types +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
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


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Default Colors +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
#define BLIP_RED0      ((ColorRGB){129, 34, 41, 255})
#define BLIP_RED1      ((ColorRGB){212, 94, 102, 255})
#define BLIP_GREEN0    ((ColorRGB){34, 129, 91, 255})
#define BLIP_GREEN1    ((ColorRGB){94, 212, 165, 255})
#define BLIP_BLUE0     ((ColorRGB){34, 63, 129, 255})
#define BLIP_BLUE1     ((ColorRGB){94, 129, 212, 255})
#define BLIP_CYAN0     ((ColorRGB){34, 110, 129, 255})
#define BLIP_CYAN1     ((ColorRGB){94, 188, 212, 255})
#define BLIP_MAGENTA0  ((ColorRGB){63, 34, 129, 255})
#define BLIP_MAGENTA1  ((ColorRGB){129, 94, 212, 255})
#define BLIP_YELLOW0   ((ColorRGB){129, 101, 34, 255})
#define BLIP_YELLOW1   ((ColorRGB){212, 177, 94, 255})
#define BLIP_BG0       ((ColorRGB){31, 31, 31, 255})
#define BLIP_BG1       ((ColorRGB){35, 45, 35, 255})
#define BLIP_SHADOW0   ((ColorRGB){20, 22, 30, 255})
#define BLIP_INK       ((ColorRGB){214, 219, 229, 255})
#define BLIP_ERROR     ((ColorRGB){255, 0, 255, 255})


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Geometry +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
// [X] Point
// [ ] Vec2
// [ ] Vec3
#define BLIP_TAU 6.28318530718f

typedef struct {
    int x;
    int y;
} Point;


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ View mode +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
    BLIP_VIEW_FILL,
    BLIP_VIEW_WIRE,
    BLIP_VIEW_FILL_WIRE
} BlipView;
void blip_set_view(BlipView mode);


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Fill mode +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
// [X] fill_flat
// [X] fill_gradient
// [X] fill_dither
// [X] fill_fallback
typedef ColorRGB (*FillFn)(int x, int y, void *ctx);

typedef enum {
    BLIP_FILL_FLAT,
    BLIP_FILL_GRADIENT,
    BLIP_FILL_DITHER,
    BLIP_FILL_DITHER_GLOW,
    BLIP_FILL_FALLBACK,
} BlipFillMode;

typedef struct {
    BlipFillMode mode;
    union {
        struct {
            ColorRGB color;
        } flat;

        struct {
            Point p0, p1;
            const ColorRGB *stops;
            int stop_count;
        } gradient;

        struct {
            Point origin;
            float radius;
            ColorRGB color0, color1;
        } dither;

        struct {
            Point origin;
            float radius;
            float core;
            ColorRGB color0, color1;
        } glow;

        struct {
            FillFn fn;
            void *ctx;
        } fallback;
    };
} FillParams;


// ++++ fill_flat
static inline FillParams blip_flat(ColorRGB c)
{
    return (FillParams) {
        .mode = BLIP_FILL_FLAT,
        .flat = {
            c
        }
    };
}


// ++++ fill_gradient
static inline FillParams blip_gradient(Point p0, Point p1, const ColorRGB *stops, int n)
{
    return (FillParams) {
        .mode = BLIP_FILL_GRADIENT,
        .gradient = {
            p0, p1, stops, n
        }
    };
}

static inline FillParams blip_gradient_h(Point origin, int w, const ColorRGB *stops, int n)
{
    return blip_gradient(origin, (Point) {
        origin.x + w - 1, origin.y
    }, stops, n);
}

static inline FillParams blip_gradient_v(Point origin, int h, const ColorRGB *stops, int n)
{
    return blip_gradient(origin, (Point) {
        origin.x, origin.y + h - 1
    }, stops, n);
}


// ++++ fill_dither
static inline FillParams blip_dither(Point origin, float radius, ColorRGB color0, ColorRGB color1)
{
    return (FillParams) {
        .mode = BLIP_FILL_DITHER,
        .dither = {origin, radius, color0, color1}
    };
}

static inline FillParams blip_dither_n(Point origin, float size, float frac, ColorRGB color0, ColorRGB color1)
{
    return blip_dither(origin, size * frac, color0, color1);
}

static inline FillParams blip_dither_glow(Point origin, float radius, float core, ColorRGB color0, ColorRGB color1)
{
    if (core < 0.0f) core = 0.0f;
    if (core > 0.99f) core = 0.99f;
    return (FillParams) {
        .mode = BLIP_FILL_DITHER_GLOW,
        .glow = {origin, radius, core, color0, color1}
    };
}

// ++++ fill_fallback
static inline FillParams blip_fallback(FillFn fn, void *ctx)
{
    return (FillParams) {
        .mode = BLIP_FILL_FALLBACK,
        .fallback = {
            fn, ctx
        },
    };
}


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Framebuffer +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
Framebuffer blip_init_fb(int width, int height, ColorRGB *pixels);
void blip_clear_fb(Framebuffer *fb, ColorRGB color);


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ pixel +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
void blip_put_pixel(Framebuffer *fb, int x, int y, ColorRGB color);
ColorRGB blip_get_pixel(const Framebuffer *fb, int x, int y);


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Draw +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
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


// +++++++++++++++++++++++++++++++++++++++++++++++
// ++++ Shapes +++++
// +++++++++++++++++++++++++++++++++++++++++++++++
// [X] fill_rect
// [X] fill_circle
// [X] fill_tri
// [X] fill_poly_convex
// [ ] fill_poly_concave
// [X] fill_quad

void blip_fill_rect_ex(Framebuffer *fb, Point origin, int w, int h, const FillParams *fp);
void blip_fill_circle_ex(Framebuffer *fb, Point c, int r, const FillParams *fp);
void blip_fill_tri_ex(Framebuffer *fb, Point a, Point b, Point c, const FillParams *fp);
void blip_fill_poly_convex_ex(Framebuffer *fb, const Point *pts, int count, const FillParams *fp);
void blip_fill_quad_ex(Framebuffer *fb, Point a, Point b, Point c, Point d, const FillParams *fp);

void blip_fill_rect(Framebuffer *fb, Point origin, int w, int h, ColorRGB color);
void blip_fill_circle(Framebuffer *fb, Point c, int r, ColorRGB color);
void blip_fill_tri(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color);
void blip_fill_poly_convex(Framebuffer *fb, const Point *pts, int count, ColorRGB color);
void blip_fill_quad(Framebuffer *fb, Point a, Point b, Point c, Point d, ColorRGB color);

#endif
