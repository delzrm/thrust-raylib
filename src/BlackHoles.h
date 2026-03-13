#pragma once
// ===================== BLACK HOLES =====================
#include "raylib.h"
#include <stdbool.h>

typedef struct {
    float x,y;
    Color color;
    float blocks[5];
    float age;
    const char *callback; // state name to set when done
} BlackHole;

void       AddBlackHole(float wx, float wy, Color col, const char *cb);
bool       DrawBlackHole(BlackHole *bh, float vpOfsX, float vpOfsY, float tick);
int        GetBhCount(void);
BlackHole *GetBhAt(int i);
void       RemoveBhAt(int i);
void       ResetBlackHoles(void);
