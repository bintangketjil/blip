#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "blip.h"
#include "raylib.h"

int main(void)
{
    int screenW = 640;
    int screenH = 480;
    ColorRGB *pixels = malloc(screenW * screenH * sizeof(ColorRGB));

    if (pixels == NULL) return 1;

    Framebuffer fb = blip_init_fb(screenW, screenH, pixels);
    InitWindow(screenW, screenH, "Blip");
    SetTargetFPS(60);
    Image img = {
        .data = fb.pixels,
        .width = screenW,
        .height = screenH,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    Texture2D tex = LoadTextureFromImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_POINT);


    while (!WindowShouldClose()) {
        blip_clear_fb(&fb, (ColorRGB) {
            35, 45, 35, 255
        });

        static const char *view_names[] = { "Fill", "Wire", "Fill+Wire" };
        static int view = 0;
        if (IsKeyPressed(KEY_SPACE)) view = (view + 1) % 3;
        blip_set_view((BlipView)view);

        float t = GetTime();

        {
            // float t = GetTime();
            // float s = sinf(t);
            // float c = cosf(t);

            Point o = {
                screenW / 2 - 100,
                screenH / 2 - 100
            };

            int w = 200;
            int h = 200;

            Point co = {
                o.x + w / 2,
                o.y + h / 2
            };

            FillParams fp = blip_dither_n(co, 200, 0.6f, BLIP_MAGENTA0, BLIP_YELLOW0);
            blip_fill_rect_ex(&fb, o, w, h, &fp);
        }
        {
            Point cc = { 100, 200};
            int outer = 40;
            float r = (float)outer + (sinf(t * 0.5f) * 0.5f);
            float core = 0.3f + (sinf(t * 2.0f) * 0.1f);

            // float pulse = 10.0f + (sinf(t * 4.0f) * 1.0f);

            // FillParams fp = blip_dither_n(cc, 80, 0.5f, BLIP_MAGENTA0, BLIP_RED0);
            FillParams glow = blip_dither_glow(
            cc, r, core, BLIP_BG1, BLIP_YELLOW0
            );
            blip_fill_circle_ex(&fb, cc, outer, &glow);
        }

        UpdateTexture(tex, fb.pixels);
        BeginDrawing();
        DrawTexture(tex, 0, 0, WHITE);
        DrawFPS(10, 10);
        DrawText(TextFormat("View: %s", view_names[view]), 100, 10, 20, WHITE);
        EndDrawing();
    }

    UnloadTexture(tex);
    CloseWindow();
    free(pixels);
    return 0;
}
