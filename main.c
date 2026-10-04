#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "blip.h"
#include "raylib.h"


int main(void)
{
    int screenW = 320;
    int screenH = 240;

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


    // +++++++++
    ColorRGB red = (ColorRGB) {
        200, 0, 0, 255
    };

    while(!WindowShouldClose()) {
        blip_clear_fb(&fb, (ColorRGB) {
            35, 45, 35, 255
        });

        float t = GetTime() * 5.0f;
        float s = sinf(t), c = cosf(t);
        int r = 100;

        Point center = { screenW / 2, screenH / 2 };
        Point tip = {
            center.x + (int)lroundf(s * r),
            center.y - (int)lroundf(c * r)
        };


        blip_draw_line(&fb, center, tip, red);

        UpdateTexture(tex, fb.pixels);
        BeginDrawing();
        DrawTexture(tex, 0, 0, WHITE);
        EndDrawing();
    }

    UnloadTexture(tex);
    CloseWindow();
    free(pixels);

    return 0;
}
