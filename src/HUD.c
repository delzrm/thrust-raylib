#include "HUD.h"
#include "VectorFont.h"
#include "rlgl.h"
#include <stdio.h>
#include <string.h>

// ---- Local colour helpers ----
static Color HexColor(unsigned r, unsigned g, unsigned b) { return (Color){r, g, b, 255}; }
#define C_YELLOW  HexColor(255,255,0)
#define C_GREEN   HexColor(0,255,0)
#define C_WHITE   HexColor(255,255,255)
#define C_BLACK   CLITERAL(Color){0,0,0,255}

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
             bool countdownActive, int countdown, Texture2D hudTexture) {
    // Scale to actual window width (HUD was designed for 960px).
    float hs = (float)GetScreenWidth() / 960.0f;
    int xoff = 0;
    if (hs > 1.0f) { hs = 1.0f; xoff = (GetScreenWidth() - 960) / 2; }

    // Draw HUD background stretched to full window width.
    int lvFrame = curLevel - 1;
    if (lvFrame < 0) lvFrame = 0;
    if (lvFrame > 5) lvFrame = 5;
    Rectangle src = { lvFrame * 960.0f, 0.0f, 960.0f, 51.0f };
    Rectangle dst = { (float)xoff, 0.0f, 960.0f * hs, (float)HUD_H };
    DrawTexturePro(hudTexture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);

    // Stretch number positions to match the HUD scale.
    rlPushMatrix();
    rlScalef(hs, 1.0f, 1.0f);

    // Black out number display areas so old values don't bleed through.
    DrawRectangle(xoff+94,  30, 150, 12, C_BLACK);
    DrawRectangle(xoff+276, 30,  40, 12, C_BLACK);
    DrawRectangle(xoff+426, 30,  90, 12, C_BLACK);
    DrawRectangle(xoff+626, 30,  40, 12, C_BLACK);
    DrawRectangle(xoff+667, 30, 200, 12, C_BLACK);

    char buf[64];

    snprintf(buf, sizeof(buf), "%d", (int)fuel);
    DrawVectorStr(buf, xoff+95.0f, HUD_NUM_Y, C_YELLOW);

    snprintf(buf, sizeof(buf), "%d", lives);
    DrawVectorStr(buf, xoff+471.0f - VectorStrWidth(buf)/2.0f, HUD_NUM_Y, C_YELLOW);

    snprintf(buf, sizeof(buf), "%d", score);
    DrawVectorStr(buf, xoff+867.0f - VectorStrWidth(buf), HUD_NUM_Y, C_YELLOW);

    if (countdownActive && countdown >= 0) {
        snprintf(buf, sizeof(buf), "%d", countdown);
        float cw = VectorStrWidth(buf);
        DrawVectorStr(buf, xoff+296.0f - cw/2.0f, HUD_NUM_Y, C_GREEN);
        DrawVectorStr(buf, xoff+646.0f - cw/2.0f, HUD_NUM_Y, C_GREEN);
    }

    rlPopMatrix();

    DrawFPS(0, 51);
}

// ---- DrawMessage ----
void xDrawMessage(const char *msg, float zoom) {
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
void DrawMessage(const char *msg, float zoom) {
    DrawRectangle(0, HUD_H, GetScreenWidth(), GetScreenHeight() - HUD_H, C_BLACK);
    xDrawMessage(msg,zoom);
}
void DrawMessageTitle(Texture2D picTexture, const char *msg, float zoom) {
    Color col = C_BLACK;
    col.r = 40;
    DrawRectangle(0, HUD_H, GetScreenWidth(), GetScreenHeight() - HUD_H, col);


    Rectangle src = { 0.0f, 0.0f, 320.0f, 200.0f };
    Rectangle dst = { 0.0f, HUD_H, GetScreenWidth(), GetScreenHeight() - HUD_H };

    
    DrawTexturePro(picTexture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);


    xDrawMessage(msg,zoom);
}

