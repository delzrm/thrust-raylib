#include "BlackHoles.h"
#include "HUD.h"
#include "raylib.h"
#include <stdbool.h>

#define MAX_BH 5

static BlackHole sBhArr[MAX_BH];
static int sBhCount;

void AddBlackHole(float wx, float wy, Color col, GameState cb) {
    if (sBhCount >= MAX_BH) return;
    BlackHole *bh = &sBhArr[sBhCount++];
    bh->x = wx; bh->y = wy;
    bh->color = col;
    bh->callback = cb;
    bh->age = 0;
    for (int i = 0; i < 5; i++) bh->blocks[i] = 1.0f;
}

// Returns true when animation completes
bool DrawBlackHole(BlackHole *bh, float vpOfsX, float vpOfsY, float tick) {
    float sx = bh->x - vpOfsX, sy = bh->y - vpOfsY + HUD_H;
    for (int angle = 0; angle < 360; angle += 90) {
        float rotation = (float)(angle - 90);
        float dist = 16.0f;
        for (int i = 0; i < 5; i++) {
            if (bh->blocks[i] >= 1.0f) {
                float bw = bh->blocks[i];
                Rectangle rec = { sx, sy, bw, 18.0f };
                Vector2 origin = { -dist, 9.0f };
                DrawRectanglePro(rec, origin, rotation, bh->color);
            }
            dist += 21.0f;
        }
    }
    bh->age += tick;
    if (bh->age < 7)  bh->blocks[0] += 4.5f * tick;
    else              bh->blocks[0] -= 2.0f * tick;
    for (int i = 1; i < 5; i++) bh->blocks[i] = bh->blocks[i-1] / 1.6f;
    return bh->blocks[0] < 0;
}

int GetBhCount(void)       { return sBhCount; }
BlackHole *GetBhAt(int i)  { return &sBhArr[i]; }
void RemoveBhAt(int i)     { sBhArr[i] = sBhArr[--sBhCount]; }
void ResetBlackHoles(void) { sBhCount = 0; }
