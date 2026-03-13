#include "VectorFont.h"
#include "GameTypes.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

// ---- Internal polygon fill helpers (local copies; V2 from GameTypes.h) ----

// Max polygon vertices expected for any glyph.
#define VF_MAX_VERTS 16

static void DrawTriCCW(V2 a, V2 b, V2 c, Color col) {
    float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    Vector2 va={a.x,a.y}, vb={b.x,b.y}, vc={c.x,c.y};
    if (cross < 0) DrawTriangle(va,vb,vc,col);
    else           DrawTriangle(va,vc,vb,col);
}

static bool PointInTri(V2 a, V2 b, V2 c, V2 p) {
    float d1 = (p.x-b.x)*(a.y-b.y) - (a.x-b.x)*(p.y-b.y);
    float d2 = (p.x-c.x)*(b.y-c.y) - (b.x-c.x)*(p.y-c.y);
    float d3 = (p.x-a.x)*(c.y-a.y) - (c.x-a.x)*(p.y-a.y);
    bool has_neg = (d1<0)||(d2<0)||(d3<0);
    bool has_pos = (d1>0)||(d2>0)||(d3>0);
    return !(has_neg && has_pos);
}

// Ear-clipping triangulation for a simple polygon.
static void FillPolygon(V2 *pts, int n, Color col) {
    if (n < 3) return;
    float area = 0;
    for (int i = 0; i < n; i++) {
        int j = (i+1)%n;
        area += (pts[j].x - pts[i].x) * (pts[j].y + pts[i].y);
    }
    int idx[VF_MAX_VERTS];
    for (int i = 0; i < n; i++) idx[i] = i;
    int rem = n;
    while (rem > 3) {
        bool found = false;
        for (int i = 0; i < rem; i++) {
            int pi = (i-1+rem)%rem, ni = (i+1)%rem;
            V2 a = pts[idx[pi]], b = pts[idx[i]], c = pts[idx[ni]];
            float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
            if (fabsf(cross) < 0.01f) continue;
            if (area * cross >= 0.0f) continue;
            bool is_ear = true;
            for (int j = 0; j < rem && is_ear; j++) {
                if (j==pi || j==i || j==ni) continue;
                if (PointInTri(a, b, c, pts[idx[j]])) is_ear = false;
            }
            if (is_ear) {
                DrawTriCCW(a, b, c, col);
                for (int j = i; j < rem-1; j++) idx[j] = idx[j+1];
                rem--;
                found = true;
                break;
            }
        }
        if (!found) break;
    }
    if (rem == 3) DrawTriCCW(pts[idx[0]], pts[idx[1]], pts[idx[2]], col);
}

// ---- Vector font glyph data ----
// Matches classText.js: TEXT_SPACING=2.3, letters on a 7×5 grid.
#define VF_SCALE  2.3f
#define VF_W      (VF_SCALE * 7.0f)   // 16.1px per glyph
#define VF_ADV    (VF_W + VF_SCALE)   // 18.4px advance per character

static void DrawVectorGlyph(char ch, float ox, float oy, Color col) {
#define P(px,py) (V2){ox + (px)*VF_SCALE, oy + (py)*VF_SCALE}
    switch (ch) {
    case '0': {
        V2 a[]={P(0,0),P(7,0),P(7,5),P(5,5),P(5,1),P(2,1),P(2,5),P(0,5)};
        V2 b[]={P(0,5),P(7,5),P(7,3),P(3,3),P(3,4),P(0,4)};
        FillPolygon(a,8,col); FillPolygon(b,6,col);
    } break;
    case '1': {
        V2 a[]={P(0,0),P(4,0),P(4,3),P(7,3),P(7,5),P(0,5),P(0,3),P(2,3),P(2,1),P(0,1)};
        FillPolygon(a,10,col);
    } break;
    case '2': {
        V2 a[]={P(1,0),P(7,0),P(7,3),P(3,3),P(3,4),P(7,4),P(7,5),P(0,5),P(0,2),P(5,2),P(5,1),P(1,1)};
        FillPolygon(a,12,col);
    } break;
    case '3': {
        V2 a[]={P(1,0),P(5,0),P(5,2),P(7,2),P(7,5),P(0,5),P(0,4),P(3,4),P(3,3),P(1,3),P(1,2),P(3,2),P(3,1),P(1,1)};
        FillPolygon(a,14,col);
    } break;
    case '4': {
        V2 a[]={P(0,0),P(2,0),P(2,2),P(4,2),P(4,1),P(7,1),P(7,5),P(4,5),P(4,3),P(0,3)};
        FillPolygon(a,10,col);
    } break;
    case '5': {
        V2 a[]={P(0,0),P(5,0),P(5,1),P(2,1),P(2,2),P(7,2),P(7,5),P(0,5),P(0,4),P(4,4),P(4,3),P(0,3)};
        FillPolygon(a,12,col);
    } break;
    case '6': {
        V2 a[]={P(0,0),P(4,0),P(4,1),P(2,1),P(2,2),P(7,2),P(7,3),P(2,3),P(2,5),P(0,5)};
        V2 b[]={P(1,2),P(7,2),P(7,5),P(1,5),P(1,4),P(4,4),P(4,3),P(1,3)};
        FillPolygon(a,10,col); FillPolygon(b,8,col);
    } break;
    case '7': {
        V2 a[]={P(0,0),P(7,0),P(7,3),P(4,3),P(4,5),P(1,5),P(1,2),P(5,2),P(5,1),P(1,1)};
        FillPolygon(a,10,col);
    } break;
    case '8': {
        V2 a[]={P(1,0),P(6,0),P(6,1),P(3,1),P(3,2),P(5,2),P(5,3),P(3,3),P(3,4),P(7,4),P(7,5),P(0,5),P(0,2),P(1,2)};
        V2 b[]={P(1,0),P(6,0),P(6,2),P(7,2),P(7,5),P(4,5),P(4,1),P(1,1)};
        FillPolygon(a,14,col); FillPolygon(b,8,col);
    } break;
    case '9': {
        V2 a[]={P(0,0),P(7,0),P(7,1),P(2,1),P(2,2),P(4,2),P(4,3),P(0,3)};
        V2 b[]={P(0,0),P(7,0),P(7,5),P(3,5),P(3,2),P(5,2),P(5,1),P(0,1)};
        FillPolygon(a,8,col); FillPolygon(b,8,col);
    } break;
    default: break;
    }
#undef P
}

float VectorStrWidth(const char *s) {
    int n = (int)strlen(s);
    if (n == 0) return 0.0f;
    return n * VF_ADV - VF_SCALE;  // = n*VF_W + (n-1)*VF_SCALE
}

void DrawVectorStr(const char *s, float x, float y, Color col) {
    for (; *s; s++) {
        DrawVectorGlyph(*s, x, y, col);
        x += VF_ADV;
    }
}
