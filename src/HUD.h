#pragma once
// ===================== HUD & MESSAGE DRAWING =====================
#include "raylib.h"
#include <stdbool.h>

#define HUD_H  16   // HUD strip height in pixels

// Draw the HUD bar (background sprite + numbers).
// hudTexture: the 5760×51 sprite sheet (6 frames of 960px, one per base level).
void DrawHUD(int curLevel, float fuel, int lives, int score,
             bool countdownActive, int countdown);

// Draw a centered text message over the game area.
// Supports inline colour tags: #rrggbb changes the current draw colour.
// zoom: the current display scale factor (gZoom).
void DrawMessage(const char *msg, float zoom);
void DrawMessageTitle(Texture2D picTexture, const char *msg, float zoom);


