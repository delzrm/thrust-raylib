#pragma once
#include "raylib.h"
#include <stdbool.h>

// ---------------------------------------------------------------------------
// Mesh2D – simple 2-D mesh stored as pointers to static const data arrays.
// All coordinates are in local (object) space; the origin is the object's
// logical centre.
//
// Because verts/lines/tris are plain pointers the struct can be initialised
// entirely at compile time from static const arrays, making the mesh data
// easy to inspect, serialise, and edit with a future tool.
// ---------------------------------------------------------------------------

#define MESH2D_MAX_VERTS  200   // stack-buffer size inside DrawMesh2D

typedef struct {
    float x, y;
} Vert2D;

typedef struct {
    int a, b;           // vertex indices
} Line2D;

typedef struct {
    int a, b, c;        // vertex indices (CCW winding for fill)
    Color color;        // per-triangle fill color (alpha 0 = skip fill)
} Tri2D;

typedef struct {
    const Vert2D *verts;
    int           vertCount;

    const Line2D *lines;
    int           lineCount;

    const Tri2D  *tris;
    int           triCount;
} Mesh2D;

// ---------------------------------------------------------------------------
// Core renderer
// ---------------------------------------------------------------------------

// Draw a Mesh2D.
//   pos      – world/screen position of the mesh origin
//   rotation – angle in radians (0 = right / +X axis, CCW positive)
//   scale    – uniform scale factor
//   lineCol  – color for all line edges (if alpha==0 line edges are skipped)
void DrawMesh2D(const Mesh2D *mesh, Vector2 pos, float rotation, float scale,
                Color lineCol);

// ---------------------------------------------------------------------------
// Pre-built mesh accessors (const singletons, never modified at runtime)
// ---------------------------------------------------------------------------

const Mesh2D *GetShipMesh(void);
const Mesh2D *GetPodMesh(void);        // pod circle only
const Mesh2D *GetPodBaseMesh(void);    // landing legs / foot-plate
const Mesh2D *GetTankMesh(void);       // fuel tank body arcs + sides
const Mesh2D *GetTankLegMesh(void);    // fuel tank legs (separate colour)
const Mesh2D *GetTankLabelMesh(void);  // "FUEL" pixel-art label
const Mesh2D *GetEnemyBodyMesh(void);
const Mesh2D *GetEnemyDomeMesh(void);  // arc portion only
const Mesh2D *GetReactorBodyMesh(void);// building outline
const Mesh2D *GetReactorCircleMesh(void);

// ---------------------------------------------------------------------------
// High-level draw helpers
// ---------------------------------------------------------------------------

// pos / ori are in world coordinates – call SXf/SYf before passing here.
typedef struct { float x, y; } V2Scr;  // screen-space point

void DrawShipMesh  (float sx, float sy, float oriRad, Color col);
void DrawPodMesh   (float sx, float sy, bool withBase, Color bodyCol, Color baseCol);
void DrawPodRodMesh(float x0, float y0, float x1, float y1, Color col);
void DrawTankMesh  (float sx, float sy, Color bodyCol, Color legCol, Color labelCol);

// Enemy: body + dome arc.  oriRad is the enemy's stored orientation (radians).
void DrawEnemyMesh (float sx, float sy, float oriRad, Color col);

// Reactor: body outline, door, and (separately) animated smoke.
// smokeY[2] are the current Y offsets for the two smoke particles.
// damage in [0, maxDamage]; used to tint and to modulate smoke speed.
void DrawReactorMesh(float sx, float sy,
                     Color bodyCol, Color chimneyCol, Color doorCol,
                     float smokeY[2], bool drawSmoke,
                     float damage, float maxDamage);
