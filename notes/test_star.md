**NOTES**

# Test: star with `draw_path`

## What it tests

`draw_path` with `closed = true` on a self-crossing path. It checks that consecutive points are connected, that the last point connects back to the first, and that crossing segments don't break anything. It also exercises `draw_line` on many slopes and directions at once.

## How the path works

`draw_path` draws `P0→P1`, `P1→P2`, ... and, if `closed`, one more segment from the last point back to `P0`. There is no new math: every segment is a Bresenham `draw_line`.

## Where the points come from

A star's vertices are points on a circle, the same formula as clock hands:

```
x = cx + R * sin(t)
y = cy - R * cos(t)      (minus because y points down)
t = spin + k * step      for k = 0 .. n-1
```

- `t = 0` points straight up.
- `R` is the radius, `(cx, cy)` is the center, `spin` rotates the whole shape.

## The shape comes from the step, not the positions

| step          | n | result           |
|---------------|---|------------------|
| `TAU / 5`     | 5 | pentagon         |
| `2 * TAU / 5` | 5 | pentagram (star) |
| `TAU / 3`     | 3 | triangle         |
| `TAU / 6`     | 6 | hexagon          |

Stepping by `m * TAU / n` visits `n / gcd(n, m)` distinct points before repeating.

- A star needs `gcd(n, m) = 1`, so one continuous path works for 5 points with `m = 2`.
- A hexagram can't be drawn as one path; draw two triangles 60° apart instead.
- `2 * TAU / 6 = TAU / 3` (120°), so the path closes after 3 points: a triangle.

## Code

```c
#define BLIP_TAU 6.28318530718f   // 2π, in blip.h

float R = 100.0f;
Point pts[5];

for (int k = 0; k < 5; k++) {
    float t = k * (2.0f * BLIP_TAU / 5.0f);
    pts[k].x = cx + (int)lroundf(R * sinf(t));
    pts[k].y = cy - (int)lroundf(R * cosf(t));
}

blip_draw_path(&fb, pts, 5, true, green);
```

## Rules used

- Do the math in float, round to int once at the end with `lroundf`. A plain `(int)` cast truncates and nudges points toward the top-left.
- Add `spin = GetTime()` to `t` to rotate the star.
- Compile with `-lm`.

## Checks

- All five points are exactly `R` from the center, so the arms are equal. A hand-placed version was close but slightly uneven.
- A closed square path should be pixel-identical to `draw_rect` of the same size.
- Dim or dashed lines on screen came from display scaling, not the code: reading pixels back with `blip_get_pixel` showed `200 0 0` on every edge.

## Status

`draw_line`, `draw_rect` and `draw_path` are done.

Next: `draw_tri` and `draw_poly` as wrappers over `draw_path`, then `draw_circle`, then `Vec2` and `draw_bezier`.
