#include "Draw.h"
#include "rlgl.h"
#include <math.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Transform a local-space vertex by rotation + scale + translation.
static inline Vector2 TransformVert(const Vert2D *v,
                                    float cosA, float sinA,
                                    float scale,
                                    float px, float py)
{
    float rx = (v->x * cosA - v->y * sinA) * scale + px;
    float ry = (v->x * sinA + v->y * cosA) * scale + py;
    return (Vector2){ rx, ry };
}

// Ensure CCW winding; swap b<->c if CW.
static inline void DrawTriCCW(Vector2 a, Vector2 b, Vector2 c, Color col)
{
    float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross < 0) DrawTriangle(a, c, b, col);
    else           DrawTriangle(a, b, c, col);
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

    // Build transformed vertex buffer on the stack.
    Vector2 tv[MESH2D_MAX_VERTS];
    for (int i = 0; i < mesh->vertCount; i++)
        tv[i] = TransformVert(&mesh->verts[i], cosA, sinA, scale, pos.x, pos.y);

    // Fill triangles first (behind lines).
    for (int i = 0; i < mesh->triCount; i++) {
        const Tri2D *t = &mesh->tris[i];
        if (t->color.a == 0) continue;
        DrawTriCCW(tv[t->a], tv[t->b], tv[t->c], t->color);
    }

    // Draw line edges.
    if (lineCol.a > 0) {
        for (int i = 0; i < mesh->lineCount; i++) {
            const Line2D *l = &mesh->lines[i];
            DrawLine((int)tv[l->a].x, (int)tv[l->a].y,
                     (int)tv[l->b].x, (int)tv[l->b].y, lineCol);
        }
    }
}

// ---------------------------------------------------------------------------
// Mesh builder helpers (used only during static init)
// ---------------------------------------------------------------------------

static void MeshAddVert(Mesh2D *m, float x, float y) {
    m->verts[m->vertCount++] = (Vert2D){ x, y };
}
static void MeshAddLine(Mesh2D *m, int a, int b) {
    m->lines[m->lineCount++] = (Line2D){ a, b };
}
static void MeshAddLinePair(Mesh2D *m, float ax, float ay,
                                       float bx, float by) {
    int a = m->vertCount; MeshAddVert(m, ax, ay);
    int b = m->vertCount; MeshAddVert(m, bx, by);
    MeshAddLine(m, a, b);
}
// Add a closed polygon outline (loop of lines through consecutive verts).
// Vertices must already exist starting at startIdx.
static void MeshAddLoop(Mesh2D *m, int startIdx, int count) {
    for (int i = 0; i < count; i++)
        MeshAddLine(m, startIdx + i, startIdx + (i + 1) % count);
}
// Add a filled triangle (CCW – enforced at draw time anyway).
static void MeshAddTri(Mesh2D *m, int a, int b, int c, Color col) {
    m->tris[m->triCount++] = (Tri2D){ a, b, c, col };
}

// Arc approximation: appends arc vertices and connecting line segments.
// Returns index of first new vertex.
static int MeshAddArc(Mesh2D *m, float cx, float cy, float radius,
                      float startRad, float endRad, int segments)
{
    int first = m->vertCount;
    float step = (endRad - startRad) / (float)segments;
    for (int i = 0; i <= segments; i++) {
        float a = startRad + i * step;
        MeshAddVert(m, cx + cosf(a) * radius, cy + sinf(a) * radius);
    }
    for (int i = 0; i < segments; i++)
        MeshAddLine(m, first + i, first + i + 1);
    return first;
}

// Shorthand: degrees -> radians with the project's canvas convention.
// In thrust.c: DTR(deg) = (deg - 90) * DEG2RAD
// We replicate that here so mesh coords match the existing draw calls.
static inline float D2R(float deg) {
    return (deg - 90.0f) * (float)(3.14159265358979323846 / 180.0);
}
// Plain radians from degrees (no -90 offset) for arcs defined directly.
static inline float DEG(float deg) {
    return deg * (float)(3.14159265358979323846 / 180.0);
}

// ---------------------------------------------------------------------------
// SHIP MESH
// ---------------------------------------------------------------------------
//  9-vertex outline matching shipVerts in thrust.c.
//  Orientation convention: 0 deg = pointing right; this mesh stores the ship
//  pointing right (+X) to match the original code.

static Mesh2D sShipMesh;
static bool   sShipMeshReady = false;

static void BuildShipMesh(void) {
    Mesh2D *m = &sShipMesh;
    memset(m, 0, sizeof(*m));

    // Original vertices from thrust.c (local space, +X = nose direction).
    static const float sv[][2] = {
        { 18,   0   },  // 0 nose
        {  0,  10   },  // 1 mid-right
        { -1,  14.5f},  // 2 rear-right wing tip
        {-11,   5.5f},  // 3 rear-right body
        { -8,   2   },  // 4 rear-right inner
        { -8,  -2   },  // 5 rear-left inner
        {-11,  -5.5f},  // 6 rear-left body
        { -1, -14.5f},  // 7 rear-left wing tip
        {  0, -10   },  // 8 mid-left
    };
    for (int i = 0; i < 9; i++) MeshAddVert(m, sv[i][0], sv[i][1]);
    MeshAddLoop(m, 0, 9);

    sShipMeshReady = true;
}

const Mesh2D *GetShipMesh(void) {
    if (!sShipMeshReady) BuildShipMesh();
    return &sShipMesh;
}

// ---------------------------------------------------------------------------
// POD MESH  (circle body, radius 9)
// ---------------------------------------------------------------------------

static Mesh2D sPodMesh;
static bool   sPodMeshReady = false;

static void BuildPodMesh(void) {
    Mesh2D *m = &sPodMesh;
    memset(m, 0, sizeof(*m));
    // Approximate circle with 20 segments.
    MeshAddArc(m, 0, 0, 9.0f, 0.0f, 2.0f * 3.14159265f, 20);
    // Close the loop (last vertex -> first vertex).
    MeshAddLine(m, m->vertCount - 1, 0);
    sPodMeshReady = true;
}

const Mesh2D *GetPodMesh(void) {
    if (!sPodMeshReady) BuildPodMesh();
    return &sPodMesh;
}

// ---------------------------------------------------------------------------
// POD BASE MESH  (landing legs + foot plate, drawn when not connected)
// Coordinates match the DrawPod base drawing in thrust.c.
// r=9, so r+3=12, r+8=17; arcs at 130°/230°/196°/164° (plain degrees).
// ---------------------------------------------------------------------------

static Mesh2D sPodBaseMesh;
static bool   sPodBaseMeshReady = false;

static void BuildPodBaseMesh(void) {
    Mesh2D *m = &sPodBaseMesh;
    memset(m, 0, sizeof(*m));

    const float r  = 9.0f;
    const float r3 = r + 3.0f;   // 12
    const float r8 = r + 8.0f;   // 17

    // Must use D2R (= DTR in thrust.c: (deg-90)*DEG2RAD) not DEG (plain deg*DEG2RAD)
    const float a130 = D2R(130), a164 = D2R(164);
    const float a196 = D2R(196), a230 = D2R(230);

    // Arc1: radius 12, 130° -> 230° (8 segments)
    MeshAddArc(m, 0, 0, r3, a130, a230, 8);

    // Line: arc1 end (r3 at 230°) -> arc2 start (r8 at 230°)
    MeshAddLinePair(m, r3 * cosf(a230), r3 * sinf(a230),
                       r8 * cosf(a230), r8 * sinf(a230));

    // Arc2: radius 17, 230° -> 196°  (note: going backwards, so step is negative)
    MeshAddArc(m, 0, 0, r8, a230, a196, 4);

    // Left leg: arc2 end -> foot pad corner
    MeshAddLinePair(m, r8 * cosf(a196), r8 * sinf(a196), -5, 27);

    // Foot plate (5 line segments forming 3-sided U shape)
    // x: -8..+8, y: +27..+31
    MeshAddLinePair(m, -5, 27, -8, 27);
    MeshAddLinePair(m, -8, 27, -8, 31);
    MeshAddLinePair(m, -8, 31,  8, 31);
    MeshAddLinePair(m,  8, 31,  8, 27);
    MeshAddLinePair(m,  8, 27,  5, 27);

    // Right leg: foot pad -> arc3 start
    MeshAddLinePair(m, 5, 27, r8 * cosf(a164), r8 * sinf(a164));

    // Arc3: radius 17, 164° -> 130°
    MeshAddArc(m, 0, 0, r8, a164, a130, 4);

    // Right strut: arc3 end -> (9, 9)
    MeshAddLinePair(m, r8 * cosf(a130), r8 * sinf(a130), 9, 9);

    sPodBaseMeshReady = true;
}

const Mesh2D *GetPodBaseMesh(void) {
    if (!sPodBaseMeshReady) BuildPodBaseMesh();
    return &sPodBaseMesh;
}

// ---------------------------------------------------------------------------
// TANK MESH  (body arcs + legs)
// ---------------------------------------------------------------------------

static Mesh2D sTankMesh;
static bool   sTankMeshReady = false;

static void BuildTankMesh(void) {
    Mesh2D *m = &sTankMesh;
    memset(m, 0, sizeof(*m));

    // Lower arc: center (0,+25), radius 40, from 24° to 336° going CW
    // In original: DrawArcLines(sx, sy+25, 40, DTR(24), DTR(336)-2PI, tc)
    // DTR(24)  = (24-90)*PI/180  = -66*PI/180
    // DTR(336) = (336-90)*PI/180 = 246*PI/180; minus 2PI = 246*PI/180 - 2PI
    float lo_start = D2R(24.0f);
    float lo_end   = D2R(336.0f) - 2.0f * 3.14159265f;
    MeshAddArc(m, 0, 25, 40, lo_start, lo_end, 16);

    // Upper arc: center (0,-30), radius 40, from 204° to 156°
    float hi_start = D2R(204.0f);
    float hi_end   = D2R(156.0f);
    MeshAddArc(m, 0, -30, 40, hi_start, hi_end, 16);

    // Connecting lines between arc endpoints (JS canvas draws these implicitly).
    // Right: upper-arc end (DTR(156) from y=-30) → lower-arc start (DTR(24) from y=+25)
    // Left:  upper-arc start (DTR(204))           → lower-arc end   (DTR(336)-2π)
    {
        float ex = 40.0f * cosf(lo_start);              // ≈ +16.27
        float by =  25.0f + 40.0f * sinf(lo_start);    // ≈ -11.54
        float ty = -30.0f + 40.0f * sinf(hi_end);      // ≈  +6.54
        MeshAddLinePair(m,  ex, ty,  ex, by);           // right side
        MeshAddLinePair(m, -ex, ty, -ex, by);           // left side
    }

    // Left leg: (-11,+20) -> (-8,+9)   — must remain last 2 lines for DrawTankMesh leg colouring
    MeshAddLinePair(m, -11, 20, -8, 9);
    // Right leg: (+11,+20) -> (+8,+9)
    MeshAddLinePair(m,  11, 20,  8, 9);

    sTankMeshReady = true;
}

const Mesh2D *GetTankMesh(void) {
    if (!sTankMeshReady) BuildTankMesh();
    return &sTankMesh;
}

// ---------------------------------------------------------------------------
// TANK LABEL MESH  ("FUEL" pixel-art letters)
// ---------------------------------------------------------------------------

static Mesh2D sTankLabelMesh;
static bool   sTankLabelMeshReady = false;

static void BuildTankLabelMesh(void) {
    Mesh2D *m = &sTankLabelMesh;
    memset(m, 0, sizeof(*m));

    // F  (x: -11..-7, y: -7..+2)
    MeshAddLinePair(m, -11, -7, -11,  2);   // vertical stroke
    MeshAddLinePair(m, -10, -7,  -7, -7);   // top bar
    MeshAddLinePair(m, -10, -2,  -8, -2);   // mid bar

    // U  (x: -4..0, y: -7..+2)
    MeshAddLinePair(m,  -4, -7,  -4,  2);   // left stroke
    MeshAddLinePair(m,  -3,  2,  -1,  2);   // bottom bar
    MeshAddLinePair(m,   0, -7,   0,  2);   // right stroke

    // E  (x: +2..+7, y: -7..+2)
    MeshAddLinePair(m,   2, -7,   2,  2);   // vertical stroke
    MeshAddLinePair(m,   3, -7,   7, -7);   // top bar
    MeshAddLinePair(m,   3, -2,   6, -2);   // mid bar
    MeshAddLinePair(m,   3,  2,   7,  2);   // bottom bar

    // L  (x: +9..+13, y: -7..+2)
    MeshAddLinePair(m,   9, -7,   9,  2);   // vertical stroke
    MeshAddLinePair(m,  10,  2,  13,  2);   // bottom bar

    sTankLabelMeshReady = true;
}

const Mesh2D *GetTankLabelMesh(void) {
    if (!sTankLabelMeshReady) BuildTankLabelMesh();
    return &sTankLabelMesh;
}

// ---------------------------------------------------------------------------
// ENEMY BODY MESH  (6-vertex hexagonal hull)
// ---------------------------------------------------------------------------

static Mesh2D sEnemyBodyMesh;
static bool   sEnemyBodyMeshReady = false;

static void BuildEnemyBodyMesh(void) {
    Mesh2D *m = &sEnemyBodyMesh;
    memset(m, 0, sizeof(*m));

    // RotObj uses (lx,ly) with angle = atan2(lx,ly)+ori, which is equivalent to
    // a standard rotation matrix applied to (ly, lx) — coords swapped.
    // So mesh verts are (ly_original, lx_original) for DrawMesh2D's standard matrix.
    static const float bp[6][2] = {   // original RotObj (lx, ly)
        { 18, -3 },
        { 28, 11 },
        { 12,  4 },
        {-12,  4 },
        {-28, 11 },
        {-18, -3 },
    };
    for (int i = 0; i < 6; i++) MeshAddVert(m, bp[i][1], bp[i][0]);  // store as (ly, lx)
    MeshAddLoop(m, 0, 6);

    sEnemyBodyMeshReady = true;
}

const Mesh2D *GetEnemyBodyMesh(void) {
    if (!sEnemyBodyMeshReady) BuildEnemyBodyMesh();
    return &sEnemyBodyMesh;
}

// ---------------------------------------------------------------------------
// ENEMY DOME MESH  (arc at local offset (0, +19), radius 26)
// In thrust.c: domeRad = 3.7 + ori, domeEnd = 2.58 + ori
// So in local space the arc spans [2.58, 3.7] radians (no orientation applied).
// ---------------------------------------------------------------------------

static Mesh2D sEnemyDomeMesh;
static bool   sEnemyDomeMeshReady = false;

static void BuildEnemyDomeMesh(void) {
    Mesh2D *m = &sEnemyDomeMesh;
    memset(m, 0, sizeof(*m));
    // Dome is at RotObj local (lx=0, ly=19) → mesh space (ly, lx) = (19, 0).
    // Arc angles 2.58..3.70 are world-space at ori=0; DrawMesh2D rotation adds ori,
    // producing the original 2.58+ori..3.70+ori after transform.
    MeshAddArc(m, 19, 0, 26, 2.58f, 3.70f, 8);
    sEnemyDomeMeshReady = true;
}

const Mesh2D *GetEnemyDomeMesh(void) {
    if (!sEnemyDomeMeshReady) BuildEnemyDomeMesh();
    return &sEnemyDomeMesh;
}

// ---------------------------------------------------------------------------
// REACTOR MESHES
// ---------------------------------------------------------------------------

// Body outline (8-line building shape) + filled black rectangle (erase circle
// overlap) baked as triangles so the mesh can drive both in one call.

static Mesh2D sReactorBodyMesh;
static bool   sReactorBodyMeshReady = false;

static void BuildReactorBodyMesh(void) {
    Mesh2D *m = &sReactorBodyMesh;
    memset(m, 0, sizeof(*m));

    // Building outline vertices (8 corners, from thrust.c DrawReactor lines):
    // (-20,+22)(-20,+12)(+12,+12)(+12,-20)(+17,-20)(+17,+12)(+20,+12)(+20,+22)
    // closed by bottom line (-20,+22)->(+20,+22)
    static const float rv[][2] = {
        {-20, 22},  // 0
        {-20, 12},  // 1
        { 12, 12},  // 2
        { 12,-20},  // 3
        { 17,-20},  // 4
        { 17, 12},  // 5
        { 20, 12},  // 6
        { 20, 22},  // 7
    };
    for (int i = 0; i < 8; i++) MeshAddVert(m, rv[i][0], rv[i][1]);
    // Open polyline (0->1->2->3->4->5->6->7) then close bottom (0->7).
    for (int i = 0; i < 7; i++) MeshAddLine(m, i, i + 1);
    MeshAddLine(m, 0, 7);  // bottom
    // Black fill is handled by DrawRectangle in DrawReactorMesh, not via triangles,
    // to avoid batch-ordering issues with Raylib's renderer.

    sReactorBodyMeshReady = true;
}

const Mesh2D *GetReactorBodyMesh(void) {
    if (!sReactorBodyMeshReady) BuildReactorBodyMesh();
    return &sReactorBodyMesh;
}

// Reactor circle mesh (approximated with 16 segments).
static Mesh2D sReactorCircleMesh;
static bool   sReactorCircleMeshReady = false;

static void BuildReactorCircleMesh(void) {
    Mesh2D *m = &sReactorCircleMesh;
    memset(m, 0, sizeof(*m));
    // Radius 1 – caller scales via the scale parameter.
    MeshAddArc(m, 0, 0, 1.0f, 0.0f, 2.0f * 3.14159265f, 10);
    MeshAddLine(m, m->vertCount - 1, 0);
    sReactorCircleMeshReady = true;
}

const Mesh2D *GetReactorCircleMesh(void) {
    if (!sReactorCircleMeshReady) BuildReactorCircleMesh();
    return &sReactorCircleMesh;
}

// ---------------------------------------------------------------------------
// High-level draw helpers
// ---------------------------------------------------------------------------

void DrawShipMesh(float sx, float sy, float oriRad, Color col)
{
    DrawMesh2D(GetShipMesh(), (Vector2){ sx, sy }, oriRad, 1.0f, col);
}

void DrawPodMesh(float sx, float sy, bool withBase, Color bodyCol, Color baseCol)
{
    DrawMesh2D(GetPodMesh(),     (Vector2){ sx, sy }, 0.0f, 1.0f, bodyCol);
    if (withBase)
        DrawMesh2D(GetPodBaseMesh(), (Vector2){ sx, sy }, 0.0f, 1.0f, baseCol);
}

void DrawPodRodMesh(float x0, float y0, float x1, float y1, Color col)
{
    DrawLine((int)x0, (int)y0, (int)x1, (int)y1, col);
}

void DrawTankMesh(float sx, float sy, Color bodyCol, Color legCol, Color labelCol)
{
    // Body arcs (no rotation for tanks – they're always upright).
    DrawMesh2D(GetTankMesh(),      (Vector2){ sx, sy }, 0.0f, 1.0f, bodyCol);
    // Legs share the same mesh – draw again with leg color for leg lines only.
    // Actually legs are baked into TankMesh; we want different colors per part.
    // Override: draw leg lines manually (they are the last 2 line pairs added).
    // Simpler approach: leg lines are indices lineCount-2 and lineCount-1.
    {
        const Mesh2D *tm = GetTankMesh();
        // Body arcs: first 34 lines (16+1 + 16+1 segments)
        // Leg lines: last 2
        // Re-draw leg lines in leg color.
        int legStart = tm->lineCount - 2;
        for (int i = legStart; i < tm->lineCount; i++) {
            const Line2D *l = &tm->lines[i];
            Vector2 a = { tm->verts[l->a].x + sx, tm->verts[l->a].y + sy };
            Vector2 b = { tm->verts[l->b].x + sx, tm->verts[l->b].y + sy };
            DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, legCol);
        }
    }
    DrawMesh2D(GetTankLabelMesh(), (Vector2){ sx, sy }, 0.0f, 1.0f, labelCol);
}

void DrawEnemyMesh(float sx, float sy, float oriRad, Color col)
{
    DrawMesh2D(GetEnemyBodyMesh(), (Vector2){ sx, sy }, oriRad, 1.0f, col);
    DrawMesh2D(GetEnemyDomeMesh(), (Vector2){ sx, sy }, oriRad, 1.0f, col);
}

void DrawReactorMesh(float sx, float sy,
                     Color bodyCol, Color chimneyCol, Color doorCol,
                     float smokeY[2], bool drawSmoke,
                     float damage, float maxDamage)
{
    (void)damage; (void)maxDamage;  // reserved for future tint/effects

    // Circle drawn first.
    DrawMesh2D(GetReactorCircleMesh(), (Vector2){ sx, sy }, 0.0f, 20.0f, bodyCol);

    // Black fill rectangles overdraw the circle where the building sits,
    // matching the original DrawRectangle calls exactly.
    DrawRectangle((int)(sx - 20), (int)(sy + 12), 41, 11, (Color){ 0, 0, 0, 255 });  // body
    DrawRectangle((int)(sx + 12), (int)(sy - 20),  6, 33, (Color){ 0, 0, 0, 255 });  // chimney

    // Building outline on top of the fill.
    DrawMesh2D(GetReactorBodyMesh(), (Vector2){ sx, sy }, 0.0f, 1.0f, chimneyCol);

    // Door: small filled rect at local (-17,+14), size 4×6.
    DrawRectangle((int)(sx - 17), (int)(sy + 14), 4, 6, doorCol);

    // Smoke particles.
    if (drawSmoke) {
        for (int i = 0; i < 2; i++)
            DrawRectangle((int)(sx + 13), (int)(sy + smokeY[i]), 3, 3, chimneyCol);
    }
}
