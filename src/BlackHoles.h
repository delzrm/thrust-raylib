#pragma once
// ===================== BLACK HOLES =====================
#include "raylib.h"
#include "GameTypes.h"
#include <stdbool.h>

typedef struct {
    float x,y;
    Color color;
    float blocks[5];
    float age;
    GameState callback; // state to set when animation completes
} BlackHole;

void       AddBlackHole(float wx, float wy, Color col, GameState cb);
bool       DrawBlackHole(BlackHole *bh, float vpOfsX, float vpOfsY, float tick);
int        GetBhCount(void);
BlackHole *GetBhAt(int i);
void       RemoveBhAt(int i);
void       ResetBlackHoles(void);
