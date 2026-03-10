#pragma once
// ===================== COLLISION PRIMITIVES =====================
#include <stdbool.h>

typedef struct { bool hit; float x, y; } Coll;

// Line-line intersection. Returns true + sets (*ox,*oy) to the intersection point.
bool CheckIntersect(float a1x, float a1y, float a2x, float a2y,
                    float b1x, float b1y, float b2x, float b2y,
                    float *ox, float *oy);

// Circle-segment intersection. Returns true + sets (*ox,*oy) to the hit point.
bool CheckIntersectCircle(float cx, float cy, float r,
                           float a1x, float a1y, float a2x, float a2y,
                           float *ox, float *oy);
