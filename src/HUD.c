#include "HUD.h"
#include "Colours.h"
#include "VectorFont.h"
#include "rlgl.h"
#include <stdio.h>
#include <string.h>

// ---- Local colour helpers (HexColor/ParseHex used by DrawMessage tag parser) ----
static Color HexColor(unsigned r, unsigned g, unsigned b) { return (Color){r, g, b, 255}; }

static Color ParseHex(const char *h) {
    if (!h || h[0] != '#') return C_WHITE;
    unsigned r = 0, g = 0, b = 0;
    for (int i = 1; i <= 6 && h[i]; i++) {
        char c = h[i];
        unsigned v = (c>='0'&&c<='9') ? (unsigned)(c-'0') :
                     (c>='a'&&c<='f') ? (unsigned)(c-'a'+10) :
                     (c>='A'&&c<='F') ? (unsigned)(c-'A'+10) : 0u;
        if      (i <= 2) r = (r << 4) | v;
        else if (i <= 4) g = (g << 4) | v;
        else             b = (b << 4) | v;
    }
    return HexColor(r, g, b);
}

// ---- DrawHUD ----
// bg.gif is 5760×51: 6 frames of 960px, one per base level (1-6).
// Numbers are drawn at HUD_NUM_Y (y=30) to overlay on the bg.gif labels/borders.
#define HUD_NUM_Y  30.0f

void DrawHUD(int curLevel, float fuel, int lives, int score,
             bool countdownActive, int countdown) {

    DrawRectangle(0,  0, GetScreenWidth(), HUD_H, (Color){32,32,  96,255});

    rlPushMatrix();
    rlScalef(1.0f, 1.0f, 1.0f);

    int textY = 0; 
    int fps = GetFPS();
//    DrawFPS(0, textY);

    DrawText(TextFormat("FPS: %2i FUEL: %4i LIVES: %2i SCORE: %8i", fps, (int)fuel, lives, score), 0, textY, 8, C_WHITE);



    rlPopMatrix();
}

// ---- DrawMessage ----
void DrawMessage(const char *msg, float zoom) {
    int cx       = GetScreenWidth() / 2;
    int fontSize = (int)(18.0f * zoom); if (fontSize < 10) fontSize = 10;
    int lineH    = (int)(24.0f * zoom);
    int y        = HUD_H + (int)(40.0f * zoom);
    Color col    = C_WHITE;
    const char *p = msg;
    char line[256];
    int li = 0;
    while (*p) {
        if (*p == '#' && *(p+1)) {
            char hex[8]; memcpy(hex, p, 7); hex[7] = 0;
            col = ParseHex(hex);
            p += 7; continue;
        }
        if (*p == '\n') {
            line[li] = 0;
            int tw = MeasureText(line, fontSize);
            DrawText(line, cx - tw/2, y, fontSize, col);
            y += lineH; li = 0; p++; continue;
        }
        if (li < 255) line[li++] = *p;
        p++;
    }
    if (li > 0) {
        line[li] = 0;
        int tw = MeasureText(line, fontSize);
        DrawText(line, cx - tw/2, y, fontSize, col);
    }
}
void DrawMessageTitle(Texture2D picTexture, const char *msg, float zoom) {
    Rectangle src = { 0.0f, 0.0f, 320.0f, 200.0f };
    Rectangle dst = { 0.0f, HUD_H, GetScreenWidth(), GetScreenHeight() - HUD_H };
    DrawTexturePro(picTexture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    DrawMessage(msg,zoom);
}

