# Blip

*Spec draft for review.*

A small software rasterizer. Its product is not a game but a sequence of scenes, ranging from text-mode to 2D to 3D-ish graphics, delivered as video or image.

## Output

### Concept

Mid-range resolution (roughly 320 to 720 lines tall), with a frame rate of about 24 to 60 fps depending on the scene. Free-form aspect ratio: 2:3, 3:4, 1:1, 19:16, 16:19. Output is video or image.

### Current take

- 640x480 at 24 fps, one constant frame rate for the whole piece.
- Size is a per-piece setting, not a compile-time constant. Both dimensions must be even (video encoding).

## Style

### Concept

Bitmap illustration. Low-poly render. Fine pixel art instead of blocky pixels. Flat fill with shadows, or textured fill (texture mapping is a maybe). The look is simulated at the pixel level instead of as a filter pass over the finished frame. Grain mimics analog film grain on an analog camera. Look reference: old French films, such as A Man and a Woman.

### Current take

- Full color, not monochrome.
- Grain: clumped, new every frame (screen-space), strength depends on brightness, interacts with the dither.
- A slight halation and lifted blacks.

### Palette

- Named hue ramps of 4 to 6 steps, chosen per scene.
- Tone is a value from 0 to 1 that maps onto a ramp. The quantizer dithers between neighboring steps.
- Ramps are muted, warm at the top and cool at the bottom. Black and white are lifted, never pure.

### Stages

- Render stage: fills produce palette pixels with dither and grain, at the logical resolution.
- Display stage: halation, and the CRT simulation if it is used, turn those pixels into the final frame.

## Core

- Framebuffer: width x height, RGBA8, row-major, origin top-left, y down. The caller of init\_fb owns it. The library never frees memory it did not allocate.
- Coordinates: pixel (x, y) covers \[x, x+1) x \[y, y+1), with its center at (x+0.5, y+0.5). Shapes test coverage at pixel centers. Floats for geometry, ints for pixels.
- Scene: a pure function of the framebuffer and a Frame.

```c
typedef struct { int index; float t; int fps; uint32_t seed; Input in; } Frame;
void scene(Framebuffer *fb, Frame f);   // t = index / fps, never from a clock
// in = keys and mouse for this frame; a recorded session is the list of inputs
```

- Determinism: same scene, frame, seed and input give the same bytes in the same context (same machine, same binary). Small differences across machines are allowed. Randomness comes only from a hash of x, y, frame and seed: no rand(), no wall clock, no uninitialised memory. Keep the hash integer-only.
- Storage is 8-bit. Floats only inside passes that need them (display stage).
- Color: four uint8\_t fields, with a union view as uint32\_t for comparing and hashing, and a size assertion.

```c
typedef union { struct { uint8_t r, g, b, a; }; uint32_t packed; } Color;
_Static_assert(sizeof(Color) == 4, "Color must be 4 bytes");
```

## API

- Everything public starts with blip\_; types are CamelCase.
- Argument order is destination, what, how: blip\_fill\_rect(fb, rect, fill).
- Every draw call clips to the framebuffer and is safe for shapes partly or fully off screen. Bad input (NULL, size <= 0) is a no-op. Functions that allocate return a status.
- Fill contract: Color fn(const FillParams \*p, void \*ctx). FillParams carries pixel x and y, shape-local u and v in 0..1, depth, and the Frame. A fill uses only these and never assumes a world range such as -1..1.
- Field: returns a float 0..1. Quantizer: turns that float into a palette color. Any field works with any quantizer.
- The caller owns ctx and keeps it alive for the duration of the call.

## Layers

A layer may only use the layers below it.

- 0 Base: types, Vec2/Vec3, math, hash
- 1 Core: framebuffer, put/blend pixel, clipping
- 2 Color: palette, ramps, lerp
- 3 Fills: fields, quantizers (Bayer, grain), combinators (fog, mask)
- 4 Shapes: line, rect, circle/ellipse, tri, poly; find covered pixels and call the fill
- 5 Projection: ortho, iso, perspective, all with the same signature
- 6 Compose: masks, layers, stamps/sprites
- 7 Display: halation, CRT, downsample
- 8 Output: image writer, video pipe

Outside the library: scenes, the preview window (raylib lives here only), and main.

## Scope

### Goals

- Real-time interaction in the preview. A session can be recorded as an input log and replayed offline into video.
- An own physics library: motion and collisions that scenes use. It never draws.
- Own color management: palettes, ramps, and the linear-light work of the display stage.
- A scene graph: objects in a tree, where a child moves with its parent (a mug on a table in a room).

### Only when needed

- Anti-aliasing.
- A z-buffer.
- External libraries: a small set (today raylib for the preview and ffmpeg for video output).

### Later

- OBJ loading. Meshes are hard-coded for now.
- An asset pipeline or a minimal UI, once the render pipeline works.

### Non-goals

- No GPU or shaders: CPU only.
- No audio, for now.
- No bit-identical output across different machines. Only the same context (machine, binary, inputs) must match.
- No scripting language for now: C only. Add another language only if it is really needed.

## Decisions

- Palette: passed in, not a global.
- Depth is part of the fill input. It is 0 for flat 2D scenes.
- Text-mode scenes: out of scope for now. A bitmap font could be added to the Compose layer later.
- Texture mapping: a stretch goal. Procedural textures come first.
- Color storage: the struct/union above.
- Display-stage budget: about one to two seconds per frame for a final render.
- Skeletal animation: not now.
- The physics library and the scene graph sit beside math, outside the drawing layers.

## Milestones

Each milestone ends in a picture.

### Stage 1: base, continuous color

- M1: determinism. Framebuffer, hash, and noise written to a PPM. Rendering the same frame twice gives identical bytes.
- M2: shapes (rect, circle, triangle, line) filled with continuous color through the fill contract, including simple shading.
- M3: a scene as a function of Frame, rendered to video and shown live in the preview.

### Stage 2: texture

- M4: palette ramps and the Bayer quantizer: dithered gradients.
- M5: film grain (clumped, new every frame, brightness-dependent) on top of the dither.
- M6: textured fills, procedural first. Texture mapping is a stretch.
