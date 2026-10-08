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
