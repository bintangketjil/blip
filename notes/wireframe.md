# Wireframe / fill view switch

## What it is

A global view mode in `blip.c` that changes how filled shapes are drawn. The mode is checked in one place, `blip_fill_tri`, so every filled shape follows it without any other change.

## How it works

Every filled shape ends in `fill_tri`:

```
fill_quad ─────────┐
fill_poly_convex ──┼──► fill_tri ──► (fill spans)  or  (draw outline)
your own calls ────┘
```

- `fill_quad` calls `fill_poly_convex`.
- `fill_poly_convex` cuts the shape into triangles (a fan from vertex 0) and calls `fill_tri` once per triangle.
- `fill_tri` reads the current mode and either fills the triangle with spans, draws its three edges with `draw_tri`, or does both.

A triangle's outer edges coincide with the shape's outline. The inner edges, where two triangles meet, only exist because the shape was cut into triangles. In filled mode they are invisible, since neighbors have the same color and tile exactly. Wire mode stops filling, so the inner edges (the joints) show.

- Rect as 2 triangles: 1 inner edge (the diagonal).
- Hexagon as a fan of 4 triangles: 3 inner edges, all starting at vertex 0.
- A convex polygon with `n` points: `n - 2` triangles, `n - 3` inner edges.

## The code

`blip.h`:

```c
typedef enum {
    BLIP_VIEW_FILL,        // normal
    BLIP_VIEW_WIRE,        // outlines only, in the fill color
    BLIP_VIEW_FILL_WIRE    // fill, with every triangle edge on top in white
} BlipView;

void blip_set_view(BlipView mode);
```

`blip.c`:

```c
static BlipView g_view = BLIP_VIEW_FILL;   // static: private to this file, keeps its value

void blip_set_view(BlipView mode) { g_view = mode; }

static void fill_tri_spans(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color)
{
    // the real scanline fill
}

void blip_fill_tri(Framebuffer *fb, Point a, Point b, Point c, ColorRGB color)
{
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
        blip_draw_tri(fb, a, b, c, (ColorRGB){255, 255, 255, 255});
        break;
    }
}
```

Shapes that are not triangle-based (`fill_rect`, `fill_circle`) check the mode at the top:

```c
if (g_view == BLIP_VIEW_WIRE) { blip_draw_rect(fb, origin, w, h, color); return; }
if (g_view == BLIP_VIEW_WIRE) { blip_draw_circle(fb, c, r, color); return; }
```

## How to use it

In the main loop, space cycles the three modes:

```c
static int view = 0;
if (IsKeyPressed(KEY_SPACE)) view = (view + 1) % 3;
blip_set_view((BlipView)view);
```

Call `blip_set_view` before drawing each frame. The mode stays set until changed.

## Notes

- It is global state. Fine for a single-threaded debug tool. For several framebuffers with different modes, move the field into `Framebuffer`.
- Wire mode uses `draw_tri`, which includes its last pixels, while `fill_tri` uses the top-left rule (bottom and right edges excluded). So in `FILL_WIRE` mode the white edge sits one pixel past the fill on the bottom and right.
- Shared edges are drawn by both neighbors in wire mode, so they look like any other edge. That is only visual and does not mean the fill rule is broken.
- The concave pentagram through `fill_poly_convex` shows the fan failing: triangles spill outside the star. That is why `fill_poly` comes last.
- Adding a mode means editing only `fill_tri` (and the enum). Every filled shape gets it for free.

## Why this design

Keep the real work in one place and build everything else on top of it, the same pattern as `draw_line` for outlines and `fill_tri` for fills. The decision is made at the bottom of the chain, so the shape code never changes.
