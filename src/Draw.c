#include "Draw.h"
#include "HUD.h"
#include "Levels.h"
#include "rlgl.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static inline Vector2 TransformVert(const Vert2D *v,
                                    float cosA, float sinA,
                                    float scale,
                                    float px, float py)
{
    float rx = (v->x * cosA - v->y * sinA) * scale + px;
    float ry = (v->x * sinA + v->y * cosA) * scale + py;
    return (Vector2){ rx, ry };
}

static inline void DrawTriCCW(Vector2 a, Vector2 b, Vector2 c, Color col)
{
    float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross < 0) DrawTriangle(a, b, c, col);
    else           DrawTriangle(a, c, b, col);
}

// ---------------------------------------------------------------------------
// DrawMesh2D
// ---------------------------------------------------------------------------

void DrawMesh2D(const Mesh2D *mesh, Vector2 pos, float rotation, float scale,
                Color lineCol)
{
    if (!mesh || mesh->vertCount == 0) return;

    float cosA = cosf(rotation);
    float sinA = sinf(rotation);

    Vector2 tv[MESH2D_MAX_VERTS];
    for (int i = 0; i < mesh->vertCount; i++)
        tv[i] = TransformVert(&mesh->verts[i], cosA, sinA, scale, pos.x, pos.y);

    for (int i = 0; i < mesh->triCount; i++) {
        const Tri2D *t = &mesh->tris[i];
        if (t->color.a == 0) continue;
        DrawTriCCW(tv[t->a], tv[t->b], tv[t->c], t->color);
    }

    if (lineCol.a > 0) {
        for (int i = 0; i < mesh->lineCount; i++) {
            const Line2D *l = &mesh->lines[i];
            DrawLine((int)tv[l->a].x, (int)tv[l->a].y,
                     (int)tv[l->b].x, (int)tv[l->b].y, lineCol);
        }
    }
}

// ===========================================================================
// SHIP MESH
// 9-vertex outline matching shipVerts[] in thrust.c.
// Mesh stored in standard (lx, ly) space: nose points in +X direction.
// ===========================================================================

static const Vert2D kShipVerts[9] = {
    {  18.0f,    0.0f },   // 0  nose
    {   0.0f,   10.0f },   // 1  mid-right
    {  -1.0f,   14.5f },   // 2  rear-right wing tip
    { -11.0f,    5.5f },   // 3  rear-right body
    {  -8.0f,    2.0f },   // 4  rear-right inner
    {  -8.0f,   -2.0f },   // 5  rear-left inner
    { -11.0f,   -5.5f },   // 6  rear-left body
    {  -1.0f,  -14.5f },   // 7  rear-left wing tip
    {   0.0f,  -10.0f },   // 8  mid-left
};
static const Line2D kShipLines[9] = {
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},{8,0}
};
static const Mesh2D sShipMesh = { kShipVerts, 9, kShipLines, 9, NULL, 0 };

const Mesh2D *GetShipMesh(void) { return &sShipMesh; }

static const Vert2D kShipThruster1Verts[3] = {
    {  -12.0f,    0.0f },   // 0  thrustertip
    {  -8.0f,    2.0f },   // 4  rear-right inner
    {  -8.0f,   -2.0f }   // 5  rear-left inner
};
static const Vert2D kShipThruster2Verts[3] = {
    {  -11.0f,    0.0f },   // 0  thrustertip
    {  -8.0f,    2.0f },   // 4  rear-right inner
    {  -8.0f,   -2.0f }   // 5  rear-left inner
};

static const Line2D kShipThrusterLines[2] = {
    {0,1},{0,2}
};
static const Mesh2D sShipThruster1Mesh = { kShipThruster1Verts, 3, kShipThrusterLines, 2, NULL, 0 };
static const Mesh2D sShipThruster2Mesh = { kShipThruster2Verts, 3, kShipThrusterLines, 2, NULL, 0 };

const Mesh2D *GetShipThrusterMesh(void) { 
    if (GetRandomValue(0,10)>6){
        return &sShipThruster1Mesh;
    }
    return &sShipThruster2Mesh;
}



// ===========================================================================
// POD MESH  (circle body, radius 9, 20 segments)
// Vertices pre-computed from: angle = k * 2*PI/20, r = 9
// ===========================================================================

static const Vert2D kPodVerts[20] = {
    {   9.0000f,   0.0000f },   //  0
    {   8.5595f,   2.7812f },   //  1
    {   7.2812f,   5.2901f },   //  2
    {   5.2901f,   7.2812f },   //  3
    {   2.7812f,   8.5595f },   //  4
    {   0.0000f,   9.0000f },   //  5
    {  -2.7812f,   8.5595f },   //  6
    {  -5.2901f,   7.2812f },   //  7
    {  -7.2812f,   5.2901f },   //  8
    {  -8.5595f,   2.7812f },   //  9
    {  -9.0000f,   0.0000f },   // 10
    {  -8.5595f,  -2.7812f },   // 11
    {  -7.2812f,  -5.2901f },   // 12
    {  -5.2901f,  -7.2812f },   // 13
    {  -2.7812f,  -8.5595f },   // 14
    {   0.0000f,  -9.0000f },   // 15
    {   2.7812f,  -8.5595f },   // 16
    {   5.2901f,  -7.2812f },   // 17
    {   7.2812f,  -5.2901f },   // 18
    {   8.5595f,  -2.7812f },   // 19
};
static const Line2D kPodLines[20] = {
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},{8,9},{9,10},
    {10,11},{11,12},{12,13},{13,14},{14,15},{15,16},{16,17},{17,18},{18,19},{19,0}
};
static const Mesh2D sPodMesh = { kPodVerts, 20, kPodLines, 20, NULL, 0 };

const Mesh2D *GetPodMesh(void) { return &sPodMesh; }

// ===========================================================================
// POD BASE MESH  (landing legs + foot plate, drawn when not connected)
//
// Angles use the project's DTR convention: D2R(deg) = (deg-90)*PI/180
//   D2R(130) = 0.6981 rad,  D2R(164) = 1.2915 rad
//   D2R(196) = 1.8500 rad,  D2R(230) = 2.4435 rad
//
// Arc radii: r3 = 12 (r+3), r8 = 17 (r+8)
//
// Vertex layout (26 verts):
//   v0-v8   arc1: r=12, D2R(130)->D2R(230), 8 segments
//   v9      connector end: r=17 at D2R(230)
//   v10-v13 arc2 interior: r=17, D2R(230)->D2R(196), 4 segments
//   v14     left leg foot start (-5, 27)
//   v15-v19 foot plate U-shape: (-8,27)(-8,31)(8,31)(8,27)(5,27)
//   v20     arc3 start: r=17 at D2R(164)
//   v21-v24 arc3 interior: r=17, D2R(164)->D2R(130), 4 segments
//   v25     right strut end (9, 9)
// ===========================================================================

static const Vert2D kPodBaseVerts[26] = {
    // arc1: r=12, D2R(130)->D2R(230), 8 segments
    {   9.1925f,   7.7135f },   //  0  D2R(130)
    {   7.3051f,   9.5202f },   //  1
    {   5.0714f,  10.8757f },   //  2
    {   2.5973f,  11.7156f },   //  3
    {   0.0000f,  12.0000f },   //  4  top (90 deg)
    {  -2.5973f,  11.7156f },   //  5
    {  -5.0714f,  10.8757f },   //  6
    {  -7.3051f,   9.5202f },   //  7
    {  -9.1925f,   7.7135f },   //  8  D2R(230) at r=12 (= connector start)
    // connector + arc2: r=17, D2R(230)->D2R(196), 4 segments
    { -13.0228f,  10.9274f },   //  9  D2R(230) at r=17 (= arc2 start)
    { -11.2645f,  12.7322f },   // 10
    {  -9.2589f,  14.2574f },   // 11
    {  -7.0498f,  15.4693f },   // 12
    {  -4.6858f,  16.3414f },   // 13  D2R(196) at r=17 (= left leg start)
    // left leg + foot plate
    {  -5.0000f,  27.0000f },   // 14  left leg foot start
    {  -8.0000f,  27.0000f },   // 15
    {  -8.0000f,  31.0000f },   // 16
    {   8.0000f,  31.0000f },   // 17
    {   8.0000f,  27.0000f },   // 18
    {   5.0000f,  27.0000f },   // 19  right leg foot start
    // arc3 start + interior: r=17, D2R(164)->D2R(130), 4 segments
    {   4.6858f,  16.3414f },   // 20  D2R(164) at r=17 (= arc3 start)
    {   7.0498f,  15.4693f },   // 21
    {   9.2589f,  14.2574f },   // 22
    {  11.2645f,  12.7322f },   // 23
    {  13.0228f,  10.9274f },   // 24  D2R(130) at r=17 (= right strut start)
    // right strut end
    {   9.0000f,   9.0000f },   // 25
};
static const Line2D kPodBaseLines[25] = {
    // arc1 (8 segs)
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},
    // connector
    {8,9},
    // arc2 (4 segs)
    {9,10},{10,11},{11,12},{12,13},
    // left leg
    {13,14},
    // foot plate
    {14,15},{15,16},{16,17},{17,18},{18,19},
    // right leg
    {19,20},
    // arc3 (4 segs)
    {20,21},{21,22},{22,23},{23,24},
    // right strut
    {24,25},
};
static const Mesh2D sPodBaseMesh = { kPodBaseVerts, 26, kPodBaseLines, 25, NULL, 0 };

const Mesh2D *GetPodBaseMesh(void) { return &sPodBaseMesh; }

// ===========================================================================
// TANK MESH  (body arcs + connecting sides only)
//
// Angle convention (D2R = (deg-90)*PI/180):
//   Lower arc: center (0,+25), r=40, D2R(24) -> D2R(336)-2PI, 16 segments
//   Upper arc: center (0,-30), r=40, D2R(204) -> D2R(156),    16 segments
//   Side lines connect the arc endpoints at x ≈ ±16.27
//
// Vertex layout (34 verts):
//   v0-v16   lower arc  (17 verts)
//   v17-v33  upper arc  (17 verts)
//
// Line layout (34 lines):
//   [0-15]   lower arc segments
//   [16-31]  upper arc segments
//   [32]     right side: v33->v0
//   [33]     left  side: v16->v17
// ===========================================================================

static const Vert2D kTankVerts[34] = {
    // lower arc: center (0,+25), r=40, D2R(24)->D2R(336)-2PI, 16 segs
    {  16.2695f, -11.5418f },   //  0
    {  14.3347f, -12.3432f },   //  1
    {  12.3607f, -13.0423f },   //  2
    {  10.3528f, -13.6370f },   //  3
    {   8.3165f, -14.1259f },   //  4
    {   6.2574f, -14.5075f },   //  5
    {   4.1811f, -14.7809f },   //  6
    {   2.0934f, -14.9452f },   //  7
    {   0.0000f, -15.0000f },   //  8
    {  -2.0934f, -14.9452f },   //  9
    {  -4.1811f, -14.7809f },   // 10
    {  -6.2574f, -14.5075f },   // 11
    {  -8.3165f, -14.1259f },   // 12
    { -10.3528f, -13.6370f },   // 13
    { -12.3607f, -13.0423f },   // 14
    { -14.3347f, -12.3432f },   // 15
    { -16.2695f, -11.5418f },   // 16
    // upper arc: center (0,-30), r=40, D2R(204)->D2R(156), 16 segs
    { -16.2695f,   6.5418f },   // 17
    { -14.3347f,   7.3432f },   // 18
    { -12.3607f,   8.0423f },   // 19
    { -10.3528f,   8.6370f },   // 20
    {  -8.3165f,   9.1259f },   // 21
    {  -6.2574f,   9.5075f },   // 22
    {  -4.1811f,   9.7809f },   // 23
    {  -2.0934f,   9.9452f },   // 24
    {   0.0000f,  10.0000f },   // 25
    {   2.0934f,   9.9452f },   // 26
    {   4.1811f,   9.7809f },   // 27
    {   6.2574f,   9.5075f },   // 28
    {   8.3165f,   9.1259f },   // 29
    {  10.3528f,   8.6370f },   // 30
    {  12.3607f,   8.0423f },   // 31
    {  14.3347f,   7.3432f },   // 32
    {  16.2695f,   6.5418f },   // 33
};
static const Line2D kTankLines[34] = {
    // lower arc (16 segs)
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},
    {8,9},{9,10},{10,11},{11,12},{12,13},{13,14},{14,15},{15,16},
    // upper arc (16 segs)
    {17,18},{18,19},{19,20},{20,21},{21,22},{22,23},{23,24},{24,25},
    {25,26},{26,27},{27,28},{28,29},{29,30},{30,31},{31,32},{32,33},
    // side connectors
    {33, 0},    // right side: upper-arc end -> lower-arc start
    {16,17},    // left  side: lower-arc end -> upper-arc start
};
static const Mesh2D sTankMesh = { kTankVerts, 34, kTankLines, 34, NULL, 0 };

const Mesh2D *GetTankMesh(void) { return &sTankMesh; }

// ===========================================================================
// TANK LEG MESH  (two support legs, drawn in a separate colour)
// ===========================================================================

static const Vert2D kTankLegVerts[4] = {
    { -11.0f,  20.0f },   // 0  left leg top
    {  -8.0f,   9.0f },   // 1  left leg bottom
    {  11.0f,  20.0f },   // 2  right leg top
    {   8.0f,   9.0f },   // 3  right leg bottom
};
static const Line2D kTankLegLines[2] = {
    {0,1},  // left leg
    {2,3},  // right leg
};
static const Mesh2D sTankLegMesh = { kTankLegVerts, 4, kTankLegLines, 2, NULL, 0 };

const Mesh2D *GetTankLegMesh(void) { return &sTankLegMesh; }

// ===========================================================================
// TANK LABEL MESH  ("FUEL" pixel-art letters)
// Each stroke is a pair of unique vertices; 24 verts, 12 lines.
// ===========================================================================

static const Vert2D kTankLabelVerts[24] = {
    // F
    { -11.0f,  -7.0f }, { -11.0f,   2.0f },   //  0- 1  left stroke
    { -10.0f,  -7.0f }, {  -7.0f,  -7.0f },   //  2- 3  top bar
    { -10.0f,  -2.0f }, {  -8.0f,  -2.0f },   //  4- 5  mid bar
    // U
    {  -4.0f,  -7.0f }, {  -4.0f,   2.0f },   //  6- 7  left stroke
    {  -3.0f,   2.0f }, {  -1.0f,   2.0f },   //  8- 9  bottom bar
    {   0.0f,  -7.0f }, {   0.0f,   2.0f },   // 10-11  right stroke
    // E
    {   2.0f,  -7.0f }, {   2.0f,   2.0f },   // 12-13  left stroke
    {   3.0f,  -7.0f }, {   7.0f,  -7.0f },   // 14-15  top bar
    {   3.0f,  -2.0f }, {   6.0f,  -2.0f },   // 16-17  mid bar
    {   3.0f,   2.0f }, {   7.0f,   2.0f },   // 18-19  bottom bar
    // L
    {   9.0f,  -7.0f }, {   9.0f,   2.0f },   // 20-21  left stroke
    {  10.0f,   2.0f }, {  13.0f,   2.0f },   // 22-23  bottom bar
};
static const Line2D kTankLabelLines[12] = {
    {0,1},{2,3},{4,5},          // F
    {6,7},{8,9},{10,11},        // U
    {12,13},{14,15},{16,17},{18,19},  // E
    {20,21},{22,23},            // L
};
static const Mesh2D sTankLabelMesh = { kTankLabelVerts, 24, kTankLabelLines, 12, NULL, 0 };

const Mesh2D *GetTankLabelMesh(void) { return &sTankLabelMesh; }

// ===========================================================================
// ENEMY BODY MESH  (6-vertex hexagonal hull)
//
// RotObj(lx, ly, ori) is equivalent to a standard rotation applied to (ly,lx)
// (the arguments are swapped vs. the standard convention). Mesh vertices are
// therefore stored as (ly, lx) so that DrawMesh2D's standard matrix produces
// the same result as the original RotObj calls.
//
// Original RotObj (lx, ly) -> stored as Vert2D{ ly, lx }:
//   (18, -3) -> (-3, 18)    (28, 11) -> (11, 28)    (12,  4) -> ( 4, 12)
//   (-12, 4) -> ( 4,-12)    (-28,11) -> (11,-28)    (-18,-3) -> (-3,-18)
// ===========================================================================

static const Vert2D kEnemyBodyVerts[6] = {
    {  -3.0f,  18.0f },   // 0
    {  11.0f,  28.0f },   // 1
    {   4.0f,  12.0f },   // 2
    {   4.0f, -12.0f },   // 3
    {  11.0f, -28.0f },   // 4
    {  -3.0f, -18.0f },   // 5
};
static const Line2D kEnemyBodyLines[6] = {
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,0}
};
static const Mesh2D sEnemyBodyMesh = { kEnemyBodyVerts, 6, kEnemyBodyLines, 6, NULL, 0 };

const Mesh2D *GetEnemyBodyMesh(void) { return &sEnemyBodyMesh; }

// ===========================================================================
// ENEMY DOME MESH
//
// Dome is at RotObj local (lx=0, ly=19) -> mesh space (ly=19, lx=0) = (19,0).
// Arc angles 2.58..3.70 are in world space at ori=0; DrawMesh2D adds ori at
// draw time, reproducing the original 2.58+ori..3.70+ori.
//
// Vertices pre-computed: center=(19,0), r=26, 2.58->3.70 rad, 8 segments
// ===========================================================================

static const Vert2D kEnemyDomeVerts[9] = {
    {  -3.0066f,  13.8459f },   // 0  angle=2.58
    {  -4.7234f,  10.6396f },   // 1
    {  -5.9760f,   7.2250f },   // 2
    {  -6.7398f,   3.6691f },   // 3
    {  -7.0000f,   0.0414f },   // 4  angle≈PI
    {  -6.7514f,  -3.5871f },   // 5
    {  -5.9989f,  -7.1454f },   // 6
    {  -4.7572f, -10.5639f },   // 7
    {  -3.0506f, -13.7757f },   // 8  angle=3.70
};
static const Line2D kEnemyDomeLines[8] = {
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8}
};
static const Mesh2D sEnemyDomeMesh = { kEnemyDomeVerts, 9, kEnemyDomeLines, 8, NULL, 0 };

const Mesh2D *GetEnemyDomeMesh(void) { return &sEnemyDomeMesh; }

// ===========================================================================
// REACTOR BODY MESH  (building outline, 8 vertices)
//
// Corners: (-20,+22)(-20,+12)(+12,+12)(+12,-20)(+17,-20)(+17,+12)(+20,+12)(+20,+22)
// Open polyline 0->7 then bottom edge 0<->7.
// Black fill is done via DrawRectangle in DrawReactorMesh (not triangles) to
// avoid Raylib batch-ordering issues.
// ===========================================================================

static const Vert2D kReactorBodyVerts[8] = {
    { -20.0f,  22.0f },   // 0
    { -20.0f,  12.0f },   // 1
    {  12.0f,  12.0f },   // 2
    {  12.0f, -20.0f },   // 3
    {  17.0f, -20.0f },   // 4
    {  17.0f,  12.0f },   // 5
    {  20.0f,  12.0f },   // 6
    {  20.0f,  22.0f },   // 7
};
static const Line2D kReactorBodyLines[8] = {
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},  // open polyline
    {0,7}                                        // bottom edge
};
static const Mesh2D sReactorBodyMesh = { kReactorBodyVerts, 8, kReactorBodyLines, 8, NULL, 0 };

const Mesh2D *GetReactorBodyMesh(void) { return &sReactorBodyMesh; }

// ===========================================================================
// REACTOR CIRCLE MESH  (unit circle, 10 segments, scaled at draw time)
//
// Vertices pre-computed: center=(0,0), r=1, 0->2PI, 10 segments
// ===========================================================================

static const Vert2D kReactorCircleVerts[10] = {
    {   1.0000f,   0.0000f },   // 0
    {   0.8090f,   0.5878f },   // 1
    {   0.3090f,   0.9511f },   // 2
    {  -0.3090f,   0.9511f },   // 3
    {  -0.8090f,   0.5878f },   // 4
    {  -1.0000f,   0.0000f },   // 5
    {  -0.8090f,  -0.5878f },   // 6
    {  -0.3090f,  -0.9511f },   // 7
    {   0.3090f,  -0.9511f },   // 8
    {   0.8090f,  -0.5878f },   // 9
};
static const Line2D kReactorCircleLines[10] = {
    {0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},{8,9},{9,0}
};
static const Mesh2D sReactorCircleMesh = { kReactorCircleVerts, 10, kReactorCircleLines, 10, NULL, 0 };

const Mesh2D *GetReactorCircleMesh(void) { return &sReactorCircleMesh; }

// ===========================================================================
// High-level draw helpers
// ===========================================================================

void DrawShipMesh(float sx, float sy, float oriRad, Color col, bool thrusting)
{
    DrawMesh2D(&sShipMesh, (Vector2){ sx, sy }, oriRad, 1.0f, col);
    if (thrusting){
        col.g = 120+GetRandomValue(0,20);
     DrawMesh2D(GetShipThrusterMesh(), (Vector2){ sx, sy }, oriRad, 1.0f, col);
    }
}

void DrawPodMesh(float sx, float sy, bool withBase, Color bodyCol, Color baseCol)
{
    DrawMesh2D(&sPodMesh,     (Vector2){ sx, sy }, 0.0f, 1.0f, bodyCol);
    if (withBase)
        DrawMesh2D(&sPodBaseMesh, (Vector2){ sx, sy }, 0.0f, 1.0f, baseCol);
}

void DrawPodRodMesh(float x0, float y0, float x1, float y1, Color col)
{
    DrawLine((int)x0, (int)y0, (int)x1, (int)y1, col);
}

void DrawTankMesh(float sx, float sy, Color bodyCol, Color legCol, Color labelCol)
{
    DrawMesh2D(&sTankMesh,      (Vector2){ sx, sy }, 0.0f, 1.0f, bodyCol);
    DrawMesh2D(&sTankLegMesh,   (Vector2){ sx, sy }, 0.0f, 1.0f, legCol);
    DrawMesh2D(&sTankLabelMesh, (Vector2){ sx, sy }, 0.0f, 1.0f, labelCol);
}

void DrawEnemyMesh(float sx, float sy, float oriRad, Color col)
{
    DrawMesh2D(&sEnemyBodyMesh, (Vector2){ sx, sy }, oriRad, 1.0f, col);
    DrawMesh2D(&sEnemyDomeMesh, (Vector2){ sx, sy }, oriRad, 1.0f, col);
}

void DrawReactorMesh(float sx, float sy,
                     Color bodyCol, Color chimneyCol, Color doorCol,
                     float smokeY[2], bool drawSmoke,
                     float damage, float maxDamage)
{
    (void)damage; (void)maxDamage;  // reserved for future tint/effects

    // Circle drawn first.
    DrawMesh2D(&sReactorCircleMesh, (Vector2){ sx, sy }, 0.0f, 20.0f, bodyCol);

    // Black fill rectangles overdraw the circle where the building sits,
    // matching the original DrawRectangle calls exactly.
    DrawRectangle((int)(sx - 20), (int)(sy + 12), 41, 11, (Color){ 0, 0, 0, 255 });  // body
    DrawRectangle((int)(sx + 12), (int)(sy - 20),  6, 33, (Color){ 0, 0, 0, 255 });  // chimney

    // Building outline on top of the fill.
    DrawMesh2D(&sReactorBodyMesh, (Vector2){ sx, sy }, 0.0f, 1.0f, chimneyCol);

    // Door: small filled rect at local (-17,+14), size 4x6.
    DrawRectangle((int)(sx - 17), (int)(sy + 14), 4, 6, doorCol);

    // Smoke particles.
    if (drawSmoke) {
        for (int i = 0; i < 2; i++)
            DrawRectangle((int)(sx + 13), (int)(sy + smokeY[i]), 3, 3, chimneyCol);
    }
}

// ===========================================================================
// LANDSCAPE MESH  (ear-clipped once at level load, cached for fast drawing)
// ===========================================================================

#define MAX_LAND_TRIS (MAX_LS + 2)   // ear-clipping produces at most n-2 triangles

static bool LandPointInTri(Vector2 a, Vector2 b, Vector2 c, Vector2 p) {
    float d1 = (p.x-b.x)*(a.y-b.y) - (a.x-b.x)*(p.y-b.y);
    float d2 = (p.x-c.x)*(b.y-c.y) - (b.x-c.x)*(p.y-c.y);
    float d3 = (p.x-a.x)*(c.y-a.y) - (c.x-a.x)*(p.y-a.y);
    bool has_neg = (d1<0)||(d2<0)||(d3<0);
    bool has_pos = (d1>0)||(d2>0)||(d3>0);
    return !(has_neg && has_pos);
}

static int LandTriangulate(Vector2 *pts, int n, int tris_out[][3]) {
    if (n < 3) return 0;
    float area = 0;
    for (int i = 0; i < n; i++) {
        int j = (i+1)%n;
        area += (pts[j].x - pts[i].x) * (pts[j].y + pts[i].y);
    }
    int idx[MAX_LS + 4];
    for (int i = 0; i < n; i++) idx[i] = i;
    int rem = n, count = 0;
    while (rem > 3) {
        bool found = false;
        for (int i = 0; i < rem; i++) {
            int pi = (i-1+rem)%rem, ni = (i+1)%rem;
            Vector2 a = pts[idx[pi]], b = pts[idx[i]], c = pts[idx[ni]];
            float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
            if (fabsf(cross) < 0.01f) continue;
            if (area * cross >= 0.0f) continue;
            bool is_ear = true;
            for (int j = 0; j < rem && is_ear; j++) {
                if (j==pi || j==i || j==ni) continue;
                if (LandPointInTri(a, b, c, pts[idx[j]])) is_ear = false;
            }
            if (is_ear) {
                tris_out[count][0] = idx[pi];
                tris_out[count][1] = idx[i];
                tris_out[count][2] = idx[ni];
                count++;
                for (int j = i; j < rem-1; j++) idx[j] = idx[j+1];
                rem--;
                found = true;
                break;
            }
        }
        if (!found) break;
    }
    if (rem == 3) {
        tris_out[count][0] = idx[0];
        tris_out[count][1] = idx[1];
        tris_out[count][2] = idx[2];
        count++;
    }
    return count;
}

static Vector2 sLandPoly[MAX_LS + 4];
static int     sLandPolyN;
static int     sLandTris[MAX_LAND_TRIS][3];
static int     sLandTriCount;
static Color   sLandCol;

void CreateLandscapeMesh(const Vert2D *pts, int count, float arenaH, Color col) {
    sLandCol   = col;
    sLandPolyN = 0;
    sLandPoly[sLandPolyN++] = (Vector2){ pts[0].x, arenaH };
    for (int i = 0; i < count; i++)
        sLandPoly[sLandPolyN++] = (Vector2){ pts[i].x, pts[i].y };
    sLandPoly[sLandPolyN++] = (Vector2){ pts[count-1].x, arenaH };
    sLandTriCount = LandTriangulate(sLandPoly, sLandPolyN, sLandTris);
}

void DrawLandscapeMesh(float vpOfsX, float vpOfsY, int arenaW, bool invisible) {
    Color col = invisible ? (Color){ 0, 0, 0, 255 } : sLandCol;
    for (int rep = -1; rep <= 1; rep++) {
        float offX = (float)(rep * arenaW);
        for (int t = 0; t < sLandTriCount; t++) {
            Vector2 wa = sLandPoly[sLandTris[t][0]];
            Vector2 wb = sLandPoly[sLandTris[t][1]];
            Vector2 wc = sLandPoly[sLandTris[t][2]];
            float ax = wa.x + offX - vpOfsX, ay = wa.y - vpOfsY + HUD_H;
            float bx = wb.x + offX - vpOfsX, by = wb.y - vpOfsY + HUD_H;
            float cx = wc.x + offX - vpOfsX, cy = wc.y - vpOfsY + HUD_H;
            DrawTriCCW((Vector2){ ax, ay }, (Vector2){ bx, by }, (Vector2){ cx, cy }, col);
        }
    }
}

// ===========================================================================
// DOOR MESH  (triangle-fan fill for convex door polygons + keyhole circles)
// ===========================================================================

void DrawDoorMesh(const DoorDef *def, int state,
                  float vpOfsX, float vpOfsY,
                  Color doorCol, Color keyCol)
{
    int vc = def->vertCount[state];
    if (vc >= 3) {
        // Build screen-space polygon
        Vector2 pts[MAX_DVERTS];
        for (int i = 0; i < vc; i++) {
            pts[i].x = def->x + def->verts[state][i][0] - vpOfsX;
            pts[i].y = def->y + def->verts[state][i][1] - vpOfsY + HUD_H;
        }
        // Triangle fan from pts[0] — works for convex polygons
        for (int i = 1; i < vc - 1; i++) {
            Vector2 a = pts[0], b = pts[i], c = pts[i+1];
            float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
            if (cross > 0) DrawTriangle(a, c, b, doorCol);
            else           DrawTriangle(a, b, c, doorCol);
        }
    }

    // Keyholes
    for (int k = 0; k < def->keyholeCount; k++) {
        int kx = (int)(def->keyholes[k][0] - vpOfsX);
        int ky = (int)(def->keyholes[k][1] - vpOfsY + HUD_H);
        DrawCircleLines(kx, ky, 13, keyCol);
    }
}
