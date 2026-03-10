#include "Collision.h"
#include <math.h>

// Line-line intersection. Returns true + intersection point.
bool CheckIntersect(float a1x, float a1y, float a2x, float a2y,
                    float b1x, float b1y, float b2x, float b2y,
                    float *ox, float *oy) {
    float vA = (b2x-b1x)*(a1y-b1y) - (b2y-b1y)*(a1x-b1x);
    float vB = (a2x-a1x)*(a1y-b1y) - (a2y-a1y)*(a1x-b1x);
    float vC = (b2y-b1y)*(a2x-a1x) - (b2x-b1x)*(a2y-a1y);
    if (vC == 0.0f) return false;
    float iA = vA/vC, iB = vB/vC;
    if (iA>=0&&iA<=1&&iB>=0&&iB<=1) {
        if (ox) *ox = a1x + iA*(a2x-a1x);
        if (oy) *oy = a1y + iA*(a2y-a1y);
        return true;
    }
    return false;
}

// Circle-segment intersection.
bool CheckIntersectCircle(float cx, float cy, float r,
                           float a1x, float a1y, float a2x, float a2y,
                           float *ox, float *oy) {
    r += 1.0f;
    float vA = (a2x-a1x)*(a2x-a1x)+(a2y-a1y)*(a2y-a1y);
    float vB = 2*((a2x-a1x)*(a1x-cx)+(a2y-a1y)*(a1y-cy));
    float vC = cx*cx+cy*cy+a1x*a1x+a1y*a1y-2*(cx*a1x+cy*a1y)-r*r;
    float det = vB*vB - 4*vA*vC;
    if (det>0) {
        float e = sqrtf(det);
        float u1 = (-vB+e)/(2*vA);
        float u2 = (-vB-e)/(2*vA);
        if (u1>=0&&u1<=1) { if(ox)*ox=a1x+(a2x-a1x)*u1; if(oy)*oy=a1y+(a2y-a1y)*u1; return true; }
        if (u2>=0&&u2<=1) { if(ox)*ox=a1x+(a2x-a1x)*u2; if(oy)*oy=a1y+(a2y-a1y)*u2; return true; }
    }
    return false;
}
