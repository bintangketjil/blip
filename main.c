#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "blip.h"
#include "raylib.h"



int main(void) {
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

    // +++++++++
    ColorRGB red = (ColorRGB) {
        200, 0, 0, 255
    };
    // ColorRGB green = (ColorRGB) {
    //     0, 200, 0, 255
    // };

    while (!WindowShouldClose()) {
        blip_clear_fb(&fb, (ColorRGB) {
            35, 45, 35, 255
        });

        static int view = 0;
        if (IsKeyPressed(KEY_SPACE)) view = (view + 1) % 3;
        blip_set_view((BlipView)view);


        float angle = GetTime();
        float s = sinf(angle), co = cosf(angle);

        {
            Point center = {320, 240};
            float hw = 150.0f, hh = 100.0f;
            float off[4][2] = {
                {-hw, -hh},
                {hw, -hh},
                {hw, hh},
                {-hw, hh},
            };
            Point q[4];

            for (int i = 0; i < 4; i++) {
                q[i].x = center.x + (int)lroundf(off[i][0] * co - off[i][1] * s);
                q[i].y = center.y + (int)lroundf(off[i][0] * s + off[i][1] * co);
            }

            blip_fill_quad(&fb, q[0], q[1], q[2], q[3], (ColorRGB){100, 190, 125, 255});
            blip_draw_poly(&fb, q, 4, red);
        }

        {
            Point center = {440, 240};
            float R = 70.0f;
            int count = 64;
            Point hex[count];

            for (int k = 0; k < count; k++) {
                float t = angle + k * (BLIP_TAU / (float)count);
                hex[k].x = center.x + (int)lroundf(R * sinf(t));
                hex[k].y = center.y - (int)lroundf(R * cosf(t));
            }

            blip_fill_poly_convex(&fb, hex, count, (ColorRGB){100, 100, 150, 255});
            blip_draw_poly(&fb, hex, count, red);
        }

        blip_fill_circle(&fb, (Point){200, 180}, 50, (ColorRGB){255, 180, 95, 255});


        UpdateTexture(tex, fb.pixels);
        BeginDrawing();
        DrawTexture(tex, 0, 0, WHITE);
        DrawFPS(10, 10);
        EndDrawing();
    }

    UnloadTexture(tex);
    CloseWindow();
    free(pixels);
    return 0;
}
