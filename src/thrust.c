// Thrust - C/raylib port of the JavaScript Thrust game
#include "raylib.h"
#include "rlgl.h"
#include "Draw.h"
#include "Levels.h"
#include "Collision.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// stop console window popping up in release
#if defined(WIN32) && !defined(_DEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
// ===================== CONSTANTS =====================
//#define VIEWPORT_W   1024/*(960)*/
//#define VIEWPORT_H   717/*(470)*/
#define VIEWPORT_W   960
#define VIEWPORT_H   470
#define HUD_H        51
#define SCREEN_H     (VIEWPORT_H + HUD_H)
#define GAME_FPS     60
#define BASE_FPS     20.0f  // original physics rate; all per-frame increments scaled by gTick = dt*BASE_FPS
#define ROD_LEN      73.0f
// MAX_LS, MAX_ENEMIES, MAX_TANKS, MAX_DOORS, MAX_DSTATES, MAX_DVERTS,
// MAX_KEYS, MAX_RESTART, NUM_LEVELS are defined in Levels.h
#define MAX_BULLETS  40
#define MAX_EXPL     25
#define MAX_PART     55
#define MAX_BH       5
#define MAX_STARS    55

// ===================== COLOR HELPERS =====================
static Color HexColor(unsigned r, unsigned g, unsigned b) {
    return (Color){r, g, b, 255};
}
#define C_YELLOW  HexColor(255,255,0)
#define C_GREEN   HexColor(0,255,0)
#define C_RED     HexColor(255,0,0)
#define C_BLUE    HexColor(0,0,255)
#define C_CYAN    HexColor(0,255,255)
#define C_MAGENTA HexColor(255,0,255)
#define C_WHITE   HexColor(255,255,255)
#define C_BLACK   CLITERAL(Color){0,0,0,255}
#define C_GRAY    HexColor(136,136,136)

#define COL_YELLOW   CLITERAL(Color){255,255,0,255}
#define COL_TEST   CLITERAL(Color){80,80,80,255}


static Color ParseHex(const char *h) {
    if (!h || h[0]!='#') return C_WHITE;
    unsigned r=0,g=0,b=0;
    sscanf(h+1,"%02x%02x%02x",&r,&g,&b);
    return HexColor(r,g,b);
}

// ===================== MATH UTILITIES =====================
static float DTR(float deg) { return (deg - 90.0f) * DEG2RAD; }  // JS DegreesToRadians
static float RTD(float rad) { return rad * RAD2DEG; }

typedef struct { float x, y; } V2;

static V2 RotObj(float lx, float ly, float ori, float px, float py) {
    float r = sqrtf(lx*lx + ly*ly);
    float a = atan2f(lx, ly) + ori;
    return (V2){ px + r*cosf(a), py + r*sinf(a) };
}

static V2 CalcPt(float angleDeg, float radius) {
    float a = DTR(angleDeg);
    return (V2){ cosf(a)*radius, sinf(a)*radius };
}

static float Dist(float ax, float ay, float bx, float by) {
    return sqrtf((ax-bx)*(ax-bx)+(ay-by)*(ay-by));
}

// CheckIntersect and CheckIntersectCircle are in Collision.h / Collision.c

// Transform local point via canvas-style rotate+translate
static V2 TPoint(float lx, float ly, float theta, float px, float py) {
    return (V2){ px + lx*cosf(theta) - ly*sinf(theta),
                 py + lx*sinf(theta) + ly*cosf(theta) };
}

// Draw arc (raylib has no single arc fn; start/end in radians, y-down coords)
static void DrawArcLines(float cx, float cy, float radius,
                          float startRad, float endRad, Color col) {
    int seg = 20;
    float step = (endRad - startRad) / seg;
    for (int i = 0; i < seg; i++) {
        float a1 = startRad + i*step, a2 = startRad + (i+1)*step;
        DrawLine((int)(cx+cosf(a1)*radius), (int)(cy+sinf(a1)*radius),
                 (int)(cx+cosf(a2)*radius), (int)(cy+sinf(a2)*radius), col);
    }
}

// Draw filled triangle fan (polygon fill)
static void FillPoly(V2 *pts, int n, Color col) {
    if (n < 3) return;
    for (int i = 1; i < n-1; i++) {
        // raylib DrawTriangle expects CCW in screen coords
        Vector2 a = {pts[0].x, pts[0].y};
        Vector2 b = {pts[i].x, pts[i].y};
        Vector2 c = {pts[i+1].x, pts[i+1].y};
        // Check winding and swap if needed
        float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
        if (cross > 0) { Vector2 tmp=b; b=c; c=tmp; } // swap CW→CCW (raylib needs CCW)
        DrawTriangle(a, b, c, col);
    }
}

// Draw triangle with automatic CCW winding correction (raylib requires CCW)
static void DrawTriCCW(V2 a, V2 b, V2 c, Color col) {
    float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    Vector2 va={a.x,a.y}, vb={b.x,b.y}, vc={c.x,c.y};
    if (cross < 0) DrawTriangle(va,vb,vc,col);
    else           DrawTriangle(va,vc,vb,col);
}

// Test if point p is strictly inside triangle abc
static bool PointInTri(V2 a, V2 b, V2 c, V2 p) {
    float d1 = (p.x-b.x)*(a.y-b.y) - (a.x-b.x)*(p.y-b.y);
    float d2 = (p.x-c.x)*(b.y-c.y) - (b.x-c.x)*(p.y-c.y);
    float d3 = (p.x-a.x)*(c.y-a.y) - (c.x-a.x)*(p.y-a.y);
    bool has_neg = (d1<0)||(d2<0)||(d3<0);
    bool has_pos = (d1>0)||(d2>0)||(d3>0);
    return !(has_neg && has_pos);
}

// Ear-clipping triangulation for a simple polygon.
// Handles concave (non-convex) shapes correctly.
static void FillPolygon(V2 *pts, int n, Color col) {
    if (n < 3) return;
    // Compute signed area via trapezoid formula; < 0 means CW in screen (Y-down)
    float area = 0;
    for (int i = 0; i < n; i++) {
        int j = (i+1)%n;
        area += (pts[j].x - pts[i].x) * (pts[j].y + pts[i].y);
    }
    int idx[MAX_LS + 4];
    for (int i = 0; i < n; i++) idx[i] = i;
    int rem = n;
    while (rem > 3) {
        bool found = false;
        for (int i = 0; i < rem; i++) {
            int pi = (i-1+rem)%rem, ni = (i+1)%rem;
            V2 a = pts[idx[pi]], b = pts[idx[i]], c = pts[idx[ni]];
            float cross = (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
            // Ear must be convex for this polygon's winding:
            // CW polygon (area<0) → ear has cross>0; CCW (area>0) → ear has cross<0
            // Equivalently: area * cross < 0
            if (fabsf(cross) < 0.01f) continue; // collinear, skip
            if (area * cross >= 0.0f) continue;  // reflex vertex, not an ear
            // Check no other vertex is inside this triangle
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

// ===================== LEVEL DATA =====================
// Types and data are in Levels.h / Levels.c

// ===================== GAME STRUCTURES =====================
typedef struct {
    float x,y,prevX,prevY,vx,vy,avx,avy;
    float ori, applyRot, thrustPow, rotSpeed;
    bool active, thrust, shield, paused;
    bool podInitiate, podConnected, gunLoaded, refuelling;
    V2 cNose,cTail,cWingS,cWingP;
    V2 pNose,pTail,pWingS,pWingP;
} Ship;

typedef struct {
    float x,y,prevX,prevY,vx,vy;
    float rot, radius;
    bool active;
} Pod;

typedef struct {
    // body vertices (pre-rotated, in world space)
    V2 body[6];
    V2 gun, dome;
    float ori; // radians
    float gunAngleRange, gunAngleOfs, aggression;
} Enemy;

typedef struct {
    float x,y,fuelLoad;
    V2 corners[4]; // collision box
    struct { float l,r,t,b; } refuelZone;
} Tank;

typedef struct {
    float x,y;   // world pos
    float prevX, prevY;
    float vx,vy;
    bool enemyFire;
    float age; int maxAge;
} Bullet;

typedef struct {
    float x,y,vx,vy,gravity;
    Color color;
    float age; int maxAge;
} Particle;

typedef struct {
    Particle parts[MAX_PART];
    int count;
} Explosion;

typedef struct {
    float x,y;
    Color color;
    float blocks[5];
    float age;
    const char *callback; // state name to set when done
} BlackHole;

typedef struct {
    float x,y;
    int state, movement;
    float stateF;        // fractional state for sub-frame animation
    float beginCloseAge; // game age when close begins (-1 = inactive)
    bool inProgress;
    const DoorDef *def;
} Door;

typedef struct {
    // Stars
    V2 stars[MAX_STARS];
    Color starColors[MAX_STARS];
    // Arena geometry
    float vpOfsX, vpOfsY;
    float slideX, slideY;
    int arenaW, arenaH;
    const LevelDef *lvl;
} Arena;

typedef struct {
    float x,y,radius;
    int damage, maxDamage;
    int countdown; float timer;
    bool active, countdownStarted;
    float smokeY[2];
    float drawSmoke;
} Reactor;

typedef enum {
    GS_KEY_SELECT, GS_HIGHSCORE, GS_HIGHSCORE_SHOW, GS_START_GAME, GS_START_LIFE,
    GS_IN_FLIGHT, GS_BLACK_HOLE, GS_GAME_OVER,
    GS_MISSION_COMPLETE, GS_MISSION_COMPLETE_MSG,
    GS_MISSION_FAILED, GS_MISSION_FAILED_MSG,
    GS_MISSION_INCOMPLETE, GS_MISSION_INCOMPLETE_MSG,
    GS_CHECK_LEVEL, GS_DO_NOTHING, GS_HIGHSCORE_EDIT,
} GameState;

typedef struct {
    int visLevel, curLevel, prevLevel;
    float age; int lives, score; float fuel;
    bool invisTerrain;
    float lastDieY;
    bool lastDieHasPod;
    Arena arena;
    Reactor reactor;
    Ship ship;
    Pod pod;
    Enemy enemies[MAX_ENEMIES]; int enemyCount;
    Tank tanks[MAX_TANKS]; int tankCount;
    Bullet bullets[MAX_BULLETS]; int bulletCount;
    Explosion explosions[MAX_EXPL]; int explosionCount;
    BlackHole blackHoles[MAX_BH]; int bhCount;
    Door doors[MAX_DOORS]; int doorCount;
    bool cheatUnlimFuel, cheatUnlimLives;
    LevelDef level; // copy of current level data
} Game;

// ===================== HIGH SCORES =====================
typedef struct { char name[16]; int score; } HScore;
static HScore gHighScores[8] = {
    {"HCG",200000},{"Delz",150000},{"Hayes",100000},
    {"CHS",50000},{"RDA",20000},{"Super",15000},{"Space",5000},{"Towers",1000}
};
static int gHSNewIdx = -1;
static char gHSNewName[16] = {0};

// ===================== GLOBALS =====================
static Game gGame;
static GameState gState = GS_KEY_SELECT;
static Texture2D gHudTexture;   // bg.gif sprite sheet (5760×51, 6 frames of 960px)
static int gPauseTimer = 0;     // frame at which deferred state fires
static GameState gPauseTarget;
static bool gPauseActive = false;
static int gFrameCount = 0;     // total frames elapsed (always incremented)
static int gBonusScore = 0;
static float gPlanetExpPos = 0.0f;
static float gTick = 1.0f;  // dt * BASE_FPS, set each frame
static float gZoom = 1.0f;  // global zoom scale (1.0 = normal)

static Color gPlanetExpColors[] = {
    {255,255,0,255},{255,0,0,255},{0,255,0,255},{0,0,255,255},{255,0,255,255},
    {255,0,0,255},{0,255,0,255},{0,0,255,255},{255,0,255,255},{255,255,0,255}
};

// Key bindings (default)
static int KEY_CCW    = KEY_Z;
static int KEY_CW     = KEY_X;
static int KEY_FIRE   = KEY_ENTER;
static int KEY_THRUST = KEY_RIGHT_SHIFT;
static int KEY_SHIELD = KEY_SPACE;
static int BIND_PAUSE  = KEY_P;
static int KEY_QUIT   = KEY_ESCAPE;

static int gKeySelectPos = 0;
static int gKeySelectTimer = 0;

// ===================== DRAWING HELPERS =====================
// Convert world coords to screen coords
static float SXf(float wx) { return wx - gGame.arena.vpOfsX; }
static float SYf(float wy) { return wy - gGame.arena.vpOfsY + HUD_H; }
static int SX(float wx) { return (int)SXf(wx); }
static int SY(float wy) { return (int)SYf(wy); }

// Apply/remove a zoom scale transform centred on the game viewport.
// Source pivot: centre of the logical viewport (pre-zoom space).
// Destination pivot: centre of the actual screen game area, so the view
// stays centred regardless of window size.
static void BeginZoom(void) {
    if (gZoom == 1.0f) return;
    float dstX = GetScreenWidth()  * 0.5f;
    float dstY = HUD_H + (GetScreenHeight() - HUD_H) * 0.5f;
    rlPushMatrix();
    rlTranslatef(dstX, dstY, 0.0f);
    rlScalef(gZoom, gZoom, 1.0f);
    rlTranslatef(-VIEWPORT_W * 0.5f, -(HUD_H + VIEWPORT_H * 0.5f), 0.0f);
}
static void EndZoom(void) {
    if (gZoom == 1.0f) return;
    rlPopMatrix();
}

static void DrawPolyOutline(V2 *pts, int n, Color col) {
    for (int i = 0; i < n; i++) {
        int j = (i+1) % n;
        DrawLine((int)pts[i].x, (int)pts[i].y,
                 (int)pts[j].x, (int)pts[j].y, col);
    }
}

// ===================== LANDSCAPE DRAWING =====================
static void DrawLandscape(bool invisible) {
    LevelDef *lv = &gGame.level;
    Color col = invisible ? C_BLACK : ParseHex(lv->landscapeColor);
    // Use arena bottom in screen coords — matches JS fill-to-arenaH.
    // Avoids clamping terrain points to the screen edge, which causes self-intersecting
    // polygons when cave walls go leftward at the clamp boundary (level 2+).
    float polyBottom = SYf(lv->arenaH);

    for (int rep = -1; rep <= 1; rep++) {
        float offX = rep * lv->arenaW;
        V2 poly[MAX_LS + 4];
        int n = 0;

        // Bottom-left corner
        poly[n++] = (V2){SXf(lv->landscape[0].x + offX), polyBottom};

        // Terrain profile — no clamping: terrain Y <= arenaH always, so no self-intersection
        for (int i = 0; i < lv->lsCount; i++) {
            poly[n++] = (V2){SXf(lv->landscape[i].x + offX), SYf(lv->landscape[i].y)};
        }

        // Bottom-right corner
        poly[n++] = (V2){SXf(lv->landscape[lv->lsCount-1].x + offX), polyBottom};

        FillPolygon(poly, n, col);
    }
}

// ===================== STAR DRAWING =====================
static void InitStars(void) {
    LevelDef *lv = &gGame.level;
    Arena *ar = &gGame.arena;
    for (int i = 0; i < lv->starCount && i < MAX_STARS; i++) {
        float px = (float)(rand() % lv->arenaW);
        float py = (float)(rand() % lv->starLowerLimit);
        ar->stars[i] = (V2){px, py};
        ar->starColors[i] = (rand()%10 < 7) ? ParseHex(lv->starColorA) : ParseHex(lv->starColorB);
    }
}

static void DrawStars(void) {
    LevelDef *lv = &gGame.level;
    Arena *ar = &gGame.arena;
    if (ar->vpOfsY >= lv->starLowerLimit) return;
    // Randomly regenerate one star
    int ri = rand() % lv->starCount;
    float px = (float)(rand() % lv->arenaW);
    float py = (float)(rand() % lv->starLowerLimit);
    ar->stars[ri] = (V2){px, py};
    ar->starColors[ri] = (rand()%10 < 7) ? ParseHex(lv->starColorA) : ParseHex(lv->starColorB);

    for (int i = 0; i < lv->starCount && i < MAX_STARS; i++) {
        for (int rep = -1; rep <= 1; rep++) {
            int sx = SX(ar->stars[i].x + rep * lv->arenaW);
            int sy = SY(ar->stars[i].y);
            DrawRectangle(sx, sy, 2, 2, ar->starColors[i]);
        }
    }
}

// ===================== SHIP DRAWING =====================
static void CalcShipCollisionPoints(void) {
    Ship *s = &gGame.ship;
    float a = DTR(s->ori);
    s->cNose  = RotObj(0,  18, a, s->x, s->y);
    s->cTail  = RotObj(0, -11, a, s->x, s->y);
    s->cWingS = RotObj( 14.5f, -1, a, s->x, s->y);
    s->cWingP = RotObj(-14.5f, -1, a, s->x, s->y);
}

static void DrawShip(void) {
    Ship *s = &gGame.ship;
    if (!s->active) return;
    float theta = DTR(s->ori);
    float cx = SXf(s->x), cy = SYf(s->y);

    DrawShipMesh(cx, cy, theta, C_YELLOW);

    // Shield arcs — not part of the mesh
    if (s->shield && ((int)gGame.age % 2 == 0)) {
        Color sc = ParseHex(gGame.level.shieldColor);
        for (int seg = 0; seg < 3; seg++) {
            float a1 = theta + seg * 2.0f * PI / 3.0f;
            float a2 = theta + (seg+1) * 2.0f * PI / 3.0f;
            V2 oc = TPoint(3, 0, theta, cx, cy);
            DrawArcLines(oc.x, oc.y, 17, a1, a2, sc);
        }
    }

    if (s->refuelling) {
        Color rc = ParseHex(gGame.level.refuelColor);
        DrawLine((int)(cx+11),(int)(cy+16),(int)(cx+34),(int)(cy+80), rc);
        DrawLine((int)(cx-11),(int)(cy+16),(int)(cx-34),(int)(cy+80), rc);
    }
}

// ===================== POD DRAWING =====================
static void DrawPodRod(void) {
    Color rc = ParseHex(gGame.level.rodColor);
    DrawLine(SX(gGame.ship.x), SY(gGame.ship.y),
             SX(gGame.pod.x),  SY(gGame.pod.y), rc);
}

static void DrawPod(void) {
    Pod *p = &gGame.pod;
    if (!p->active) return;
    float px = SXf(p->x), py = SYf(p->y);
    Color pc = ParseHex(gGame.level.podColor);
    Color bc = ParseHex(gGame.level.podBaseColor);
    DrawPodMesh(px, py, !gGame.ship.podConnected, pc, bc);
}

// ===================== ENEMY DRAWING =====================
static void InitEnemy(Enemy *e, EnemyDef *d) {
    static const float bp[6][2] = {{18,-3},{28,11},{12,4},{-12,4},{-28,11},{-18,-3}};
    float ori = DTR(d->ori);
    e->ori = ori;
    e->gunAngleRange = d->gunRange;
    e->gunAngleOfs   = d->gunOfs;
    e->aggression    = d->aggression;
    for (int i = 0; i < 6; i++) {
        e->body[i] = RotObj(bp[i][0], bp[i][1], ori, d->x, d->y);
    }
    e->dome = RotObj(0, 19, ori, d->x, d->y);
    e->gun  = RotObj(0,-10, ori, d->x, d->y);
}

static void DrawEnemy(Enemy *e) {
    Color ec = ParseHex(gGame.level.enemyColor);
    // Recover world-space centre from dome: dome = (cx + 19*cos(ori), cy + 19*sin(ori))
    float cx = e->dome.x - 19.0f * cosf(e->ori);
    float cy = e->dome.y - 19.0f * sinf(e->ori);
    DrawEnemyMesh(SXf(cx), SYf(cy), e->ori, ec);
}

// ===================== FUEL TANK DRAWING =====================
static void InitTank(Tank *t, TankDef *d) {
    t->x = d->x; t->y = d->y;
    t->fuelLoad = 300;
    t->corners[0] = (V2){d->x-17, d->y+16};
    t->corners[1] = (V2){d->x-17, d->y-14};
    t->corners[2] = (V2){d->x+17, d->y-14};
    t->corners[3] = (V2){d->x+17, d->y+16};
    t->refuelZone.l = d->x-21;
    t->refuelZone.r = d->x+21;
    t->refuelZone.t = d->y-82;
    t->refuelZone.b = d->y-18;
}

static void DrawTank(Tank *t) {
    DrawTankMesh(SXf(t->x), SYf(t->y),
                 ParseHex(gGame.level.tankColor),
                 ParseHex(gGame.level.tankLegs),
                 ParseHex(gGame.level.tankLabel));
}

// ===================== REACTOR DRAWING =====================
static void DrawReactor(void) {
    Reactor *r = &gGame.reactor;
    if (!r->active) return;
    if (r->damage >= r->maxDamage && ((int)gGame.age % 6 < 3)) return;

    float rsx = SXf(r->x), rsy = SYf(r->y);
    if (r->damage > 0) r->damage -= 0.02f;
    bool drawSmoke = (gGame.age > r->drawSmoke);
    DrawReactorMesh(rsx, rsy,
                    ParseHex(gGame.level.reactorColor),
                    ParseHex(gGame.level.reactorChimney),
                    ParseHex(gGame.level.reactorDoor),
                    r->smokeY, drawSmoke,
                    r->damage, r->maxDamage);
    if (drawSmoke) {
        for (int i = 0; i < 2; i++) {
            r->smokeY[i] -= (1 + r->damage / 100.0f / 6.0f);
            if (r->smokeY[i] < -36) r->smokeY[i] += 20;
        }
    }
}

// ===================== DOOR DRAWING =====================
static void DrawDoor(Door *d, bool invisible) {
    const DoorDef *def = d->def;
    Color dc = invisible ? C_BLACK : ParseHex(def->doorColor);
    Color kc = ParseHex(def->keyColor);

    // Draw door polygon (current state)
    int s = d->state;
    int vc = def->vertCount[s];
    if (vc >= 3) {
        V2 pts[MAX_DVERTS];
        for (int i = 0; i < vc; i++) {
            pts[i].x = SXf(def->x + def->verts[s][i][0]);
            pts[i].y = SYf(def->y + def->verts[s][i][1]);
        }
        FillPoly(pts, vc, dc);
    }

    // Keyholes
    for (int k = 0; k < def->keyholeCount; k++) {
        float kx = SXf(def->keyholes[k][0]);
        float ky = SYf(def->keyholes[k][1]);
        DrawCircleLines((int)kx,(int)ky, 13, kc);
    }
}

// ===================== EXPLOSION DRAWING =====================
static void AddExplosion(float screenX, float screenY, bool big) {
    if (gGame.explosionCount >= MAX_EXPL) return;
    Explosion *e = &gGame.explosions[gGame.explosionCount++];
    e->count = big ? 50 : 7;
    float grav = gGame.level.gravity * 2;
    for (int i = 0; i < e->count; i++) {
        Particle *p = &e->parts[i];
        p->x = screenX - 1.5f; p->y = screenY - 1.5f;
        p->vx = ((float)rand()/RAND_MAX)*8 - 4;
        p->vy = ((float)rand()/RAND_MAX)*8 - 4;
        p->gravity = grav;
        p->age = 0;
        p->maxAge = (int)(((float)rand()/RAND_MAX)*20 + 5);
        p->color = C_WHITE; // set by caller after
    }
}

static void AddExplosionColored(float sx, float sy, bool big, Color col) {
    if (gGame.explosionCount >= MAX_EXPL) return;
    int idx = gGame.explosionCount;
    AddExplosion(sx, sy, big);
    // Set color on all new particles
    Explosion *e = &gGame.explosions[idx];
    for (int i = 0; i < e->count; i++) e->parts[i].color = col;
}

static void UpdateAndDrawExplosions(void) {
    for (int i = gGame.explosionCount-1; i >= 0; i--) {
        Explosion *e = &gGame.explosions[i];
        int alive = 0;
        for (int j = e->count-1; j >= 0; j--) {
            Particle *p = &e->parts[j];
            if (p->age > p->maxAge) continue;
            p->vy += p->gravity * gTick;
            p->x  += (p->vx - gGame.arena.slideX) * gTick;
            p->y  += (p->vy - gGame.arena.slideY) * gTick;
            DrawRectangle((int)p->x,(int)p->y,3,3,p->color);
            p->age += gTick;
            alive++;
        }
        if (alive == 0) {
            gGame.explosions[i] = gGame.explosions[--gGame.explosionCount];
        }
    }
}

// ===================== BLACK HOLE =====================
static void AddBlackHole(float wx, float wy, Color col, const char *cb) {
    if (gGame.bhCount >= MAX_BH) return;
    BlackHole *bh = &gGame.blackHoles[gGame.bhCount++];
    bh->x = wx; bh->y = wy;
    bh->color = col;
    bh->callback = cb;
    bh->age = 0;
    for (int i = 0; i < 5; i++) bh->blocks[i] = 1.0f;
}

// Returns true when animation completes
static bool DrawBlackHole(BlackHole *bh) {
    float sx = SXf(bh->x), sy = SYf(bh->y);
    for (int angle = 0; angle < 360; angle += 90) {
        // JS: rotate(DegreesToRadians(angle)) = (angle-90)*PI/180
        // For DrawRectanglePro (CW degrees): rotation = angle - 90
        // origin anchors the BH center (sx,sy) at local (-dist, 9) within each block rect,
        // so the block naturally sits at dist..dist+bw from center pointing right,
        // then the rotation swings each arm to the correct direction.
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
    bh->age += gTick;
    if (bh->age < 7)  bh->blocks[0] += 4.5f * gTick;
    else              bh->blocks[0] -= 2.0f * gTick;
    for (int i = 1; i < 5; i++) bh->blocks[i] = bh->blocks[i-1] / 1.6f;
    return bh->blocks[0] < 0;
}

// ===================== VECTOR FONT =====================
// Matches classText.js: TEXT_SPACING=2.3, letters on a 7×5 grid
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

static float VectorStrWidth(const char *s) {
    int n = (int)strlen(s);
    if (n == 0) return 0.0f;
    return n * VF_ADV - VF_SCALE;  // = n*VF_W + (n-1)*VF_SCALE
}

static void DrawVectorStr(const char *s, float x, float y, Color col) {
    for (; *s; s++) {
        DrawVectorGlyph(*s, x, y, col);
        x += VF_ADV;
    }
}

// ===================== HUD DRAWING =====================
// bg.gif is 5760×51: 6 frames of 960px, one per base level (1-6).
// JS positions canvases at top=30 within the 51px HUD div.
// Numbers drawn at y=30 (HUD_NUM_Y) to overlay on the bg.gif labels/borders.
#define HUD_NUM_Y  30.0f
static void DrawHUD(void) {
    // Scale everything to the actual window width (HUD was designed for 960px)
    float hs = (float)GetScreenWidth() / 960.0f;
    int xoff = 0;
    if (hs>1.0f){ hs=1.0f; xoff = (GetScreenWidth()-960)/2;}

    // Draw HUD background stretched to full window width
    int lvFrame = (gGame.curLevel - 1);
    if (lvFrame < 0) lvFrame = 0;
    if (lvFrame > 5) lvFrame = 5;


    Rectangle src = { lvFrame * 960.0f, 0.0f, 960.0f, 51.0f };
    Rectangle dst = { xoff, 0.0f, 960.0f*hs, (float)HUD_H };
    DrawTexturePro(gHudTexture, src, dst, (Vector2){0,0}, 0.0f, WHITE);

    // Scale all number positions and glyphs to match the stretched HUD
    rlPushMatrix();
    rlScalef(hs, 1.0f, 1.0f);  // stretch x only; y stays at original pixel rows

    // Black out the number display areas so old values don't bleed through
    DrawRectangle(xoff+94,  30, 150, 12, C_BLACK);
    DrawRectangle(xoff+276, 30,  40, 12, C_BLACK);
    DrawRectangle(xoff+426, 30,  90, 12, C_BLACK);
    DrawRectangle(xoff+626, 30,  40, 12, C_BLACK);
    DrawRectangle(xoff+667, 30, 200, 12, C_BLACK);

    char buf[64];

    // Fuel (left-aligned at x=95)
    snprintf(buf, sizeof(buf), "%d", (int)gGame.fuel);
    DrawVectorStr(buf, xoff+95.0f, HUD_NUM_Y, C_YELLOW);

    // Lives (centered at x=471)
    snprintf(buf, sizeof(buf), "%d", gGame.lives);
    DrawVectorStr(buf, xoff+471.0f - VectorStrWidth(buf)/2.0f, HUD_NUM_Y, C_YELLOW);

    // Score (right-aligned at x=867)
    snprintf(buf, sizeof(buf), "%d", gGame.score);
    DrawVectorStr(buf, xoff+867.0f - VectorStrWidth(buf), HUD_NUM_Y, C_YELLOW);

    // Countdown (centered at x=296 and x=646 when active)
    if (gGame.reactor.countdownStarted && gGame.reactor.countdown >= 0) {
        snprintf(buf, sizeof(buf), "%d", gGame.reactor.countdown);
        float cw = VectorStrWidth(buf);
        DrawVectorStr(buf, xoff+296.0f - cw/2.0f, HUD_NUM_Y, C_GREEN);
        DrawVectorStr(buf, xoff+646.0f - cw/2.0f, HUD_NUM_Y, C_GREEN);
    }

    rlPopMatrix();

    DrawFPS(0,51);

}

// ===================== MESSAGE DRAWING =====================
static void DrawMessage(const char *msg) {
    // Clear full game area
    DrawRectangle(0, HUD_H, GetScreenWidth(), GetScreenHeight() - HUD_H, C_BLACK);
    int cx       = GetScreenWidth() / 2;
//    int fontSize = (int)(12.0f * gZoom);  if (fontSize < 10) fontSize = 10;
//    int lineH    = (int)(18.0f * gZoom);
    int fontSize = (int)(18.0f * gZoom);  if (fontSize < 10) fontSize = 10;
    int lineH    = (int)(24.0f * gZoom);
    int y        = HUD_H + (int)(40.0f * gZoom);
    Color col = C_WHITE;
    const char *p = msg;
    char line[256];
    int li = 0;
    while (*p) {
        if (*p == '#' && *(p+1)) {
            char hex[8]; strncpy(hex, p, 7); hex[7]=0;
            col = ParseHex(hex);
            p += 7; continue;
        }
        if (*p == '\n') {
            line[li] = 0;
            int tw = MeasureText(line, fontSize);
            DrawText(line, cx - tw/2, y, fontSize, col);
            y += lineH; li = 0; p++; continue;
        }
        if (li < 255) line[li++] = *p;
        p++;
    }
    if (li > 0) {
        line[li] = 0;
        int tw = MeasureText(line, fontSize);
        DrawText(line, cx - tw/2, y, fontSize, col);
    }
}

// ===================== SHIP PHYSICS =====================
static void ShipCalcPosition(void) {
    Ship *s = &gGame.ship;
    Game *g = &gGame;

    s->prevX = s->x; s->prevY = s->y;
    s->pNose = s->cNose; s->pTail = s->cTail;
    s->pWingS = s->cWingS; s->pWingP = s->cWingP;

    s->vx *= (1.0f - 0.004f * gTick);  // friction: per-tick decay of 0.004 (= 1-0.996)
    s->vy += g->level.gravity * gTick;
    s->ori += s->applyRot * gTick;
    if (s->ori <   0) s->ori += 360;
    if (s->ori >= 360) s->ori -= 360;

    // Shield fuel drain
    if (g->fuel > 0 && s->shield) {
        if (!g->cheatUnlimFuel) g->fuel -= gTick;
    } else {
        s->shield = false;
    }

    // Thrust
    if (g->fuel > 0 && s->thrust) {
        V2 t = CalcPt(s->ori, s->thrustPow);
        s->vx += t.x * gTick; s->vy += t.y * gTick;
        if (!g->cheatUnlimFuel) g->fuel -= gTick;
    } else if (g->fuel <= 0) {
        s->shield = false;
    }

    s->x += s->vx * gTick; s->y += s->vy * gTick;
    s->avx = s->vx; s->avy = s->vy;

    // Pod physics
    if (s->podConnected) {
        Pod *pod = &g->pod;
        pod->prevX = pod->x; pod->prevY = pod->y;
        pod->x += pod->vx * gTick;
        pod->y += pod->vy * gTick + g->level.gravity * gTick;

        float dx = pod->x - s->x, dy = pod->y - s->y;
        float dlen = sqrtf(dx*dx + dy*dy);
        float diff = (dlen - ROD_LEN) / dlen;
        float mvx = dx * 0.5f * diff, mvy = dy * 0.5f * diff;

        s->x += mvx; s->y += mvy;
        s->vx += mvx / gTick; s->vy += mvy / gTick;  // convert position correction to logical-tick velocity
        s->avx = s->vx; s->avy = s->vy;
        pod->x -= mvx; pod->y -= mvy;
        pod->vx = (pod->x - pod->prevX) / gTick;  // logical-tick velocity
        pod->vy = (pod->y - pod->prevY) / gTick;
    }

    CalcShipCollisionPoints();

    // Check upper limit (exit) - JS: ship.y <= iUpperLimit (= viewport height = 470)
    Arena *ar = &g->arena;

    bool skipLevel = false;
    if (IsKeyPressed(KEY_EQUAL)){
        skipLevel = true;
    }

    if ((s->y <= (float)VIEWPORT_H || skipLevel) && s->active) {
        g->pod.active = false;
        if (s->podConnected || skipLevel) {
            AddBlackHole(s->x, s->y, C_YELLOW, "MissionComplete");
            AddBlackHole(g->pod.x, g->pod.y, ParseHex(g->level.podColor), "MissionComplete");
            gBonusScore = (g->reactor.countdown < 10) ?
                (3600 + g->curLevel*400) : (1600 + g->curLevel*400);
        } else {
            const char *cb = (g->reactor.countdown < 10) ? "MissionFailed" : "MissionIncomplete";
            AddBlackHole(s->x, s->y, C_YELLOW, cb);
        }
        gState = GS_BLACK_HOLE;
    }
}

static void ShipScrollViewport(void) {
    Ship *s = &gGame.ship;
    Arena *ar = &gGame.arena;
    Game *g = &gGame;

    if (s->active) {
        // For zoom > 1 (large window): divide by gZoom so the visual dead-zone stays
        // constant in screen pixels regardless of zoom level.
        // For zoom < 1 (small window): clamp to 1 so behaviour matches the gZoom=1 baseline —
        // inflating thresholds beyond the viewport at low zoom makes tracking fail entirely.
        float zoomScale   = fmaxf(gZoom, 1.0f);
        float velThresh   = 6.0f  / zoomScale;
        float slideSpeedX = 11.0f / zoomScale;
        float slideSpeedY = 10.0f / zoomScale;

        if (fabsf(s->avx) > velThresh) ar->slideX = s->avx;
        else if (s->avx != 0) {
            float cx   = VIEWPORT_W * 0.5f;
            float xOfs = fminf(VIEWPORT_W * (270.0f / 960.0f) / zoomScale, cx - 5.0f);
            if ((s->x - ar->vpOfsX) > cx + xOfs) ar->slideX = slideSpeedX;
            else if ((s->x - ar->vpOfsX) < cx - xOfs) ar->slideX = -slideSpeedX;
        }
        if (fabsf(s->avy) > velThresh) {
            ar->slideY = s->avy;
        } else {
            float cy  = VIEWPORT_H * 0.5f;
            float yDn = fminf(VIEWPORT_H * (120.0f / 470.0f) / zoomScale, cy - 5.0f);
            float yUp = fminf(VIEWPORT_H * (140.0f / 470.0f) / zoomScale, cy - 5.0f);
            if ((s->y - ar->vpOfsY) > cy + yDn) ar->slideY = slideSpeedY;
            else if ((s->y - ar->vpOfsY) < cy - yUp) ar->slideY = -slideSpeedY;
            // Pod position check only when ship isn't fast-tracking — at high gZoom the pod
            // threshold would otherwise override upward velocity tracking and push the ship off screen.
            if (s->podConnected) {
                float pDn = fminf(VIEWPORT_H * (150.0f / 470.0f) / zoomScale, cy - 5.0f);
                float pUp = fminf(VIEWPORT_H * (170.0f / 470.0f) / zoomScale, cy - 5.0f);
                if ((g->pod.y - ar->vpOfsY) > cy + pDn) ar->slideY = slideSpeedY;
                else if ((g->pod.y - ar->vpOfsY) < cy - pUp) ar->slideY = -slideSpeedY;
            }
        }
    } else {
        // Decelerate slide (exponential decay)
        if (ar->slideX > 0) { ar->slideX *= (1.0f - gTick/9.0f); if (ar->slideX < 0.1f) ar->slideX = 0; }
        else if (ar->slideX < 0) { ar->slideX *= (1.0f - gTick/9.0f); if (ar->slideX > -0.1f) ar->slideX = 0; }
        if (ar->slideY > 0) { ar->slideY *= (1.0f - gTick/7.0f); if (ar->slideY < 0.1f) ar->slideY = 0; }
        else if (ar->slideY < 0) { ar->slideY *= (1.0f - gTick/7.0f); if (ar->slideY > -0.1f) ar->slideY = 0; }
    }

    if (fabsf(ar->slideX) > 0) {
        ar->vpOfsX += ar->slideX * gTick;
        if (ar->slideX > 0) { ar->slideX -= 0.2f * gTick; if (ar->slideX < 0) ar->slideX = 0; }
        else { ar->slideX += 0.2f * gTick; if (ar->slideX > 0) ar->slideX = 0; }
    }
    if (fabsf(ar->slideY) > 0) {
        ar->vpOfsY += ar->slideY * gTick;
        if (ar->slideY > 0) { ar->slideY -= 0.2f * gTick; if (ar->slideY < 0) ar->slideY = 0; }
        else { ar->slideY += 0.2f * gTick; if (ar->slideY > 0) ar->slideY = 0; }
    }

    // Wrap-around
    if (s->x < 0) {
        s->x += g->level.arenaW;
        if (s->podConnected) g->pod.x += g->level.arenaW;
        ar->vpOfsX += g->level.arenaW;
    } else if (s->x > g->level.arenaW) {
        s->x -= g->level.arenaW;
        if (s->podConnected) g->pod.x -= g->level.arenaW;
        ar->vpOfsX -= g->level.arenaW;
    }
}

// ===================== SHIP DIE =====================
static void SetPause(GameState target, int ms) {
    gPauseActive = true;
    gPauseTarget = target;
    gPauseTimer  = gFrameCount + (int)(ms * GAME_FPS / 1000.0f); // convert ms to frames
}

static void ShipDie(float sx, float sy, bool jumpLevel) {
    Ship *s = &gGame.ship;
    s->active = false;
    s->vx = 0; s->vy = 0;
    s->thrust = false; s->shield = false;

    gGame.lastDieY = s->y;
    gGame.lastDieHasPod = s->podConnected;
    if (gGame.reactor.countdown < 10 && s->podConnected) {
        gGame.lastDieHasPod = false;
        if (gGame.reactor.countdown <= 0) gGame.lastDieY = 0;
    }

    Color shipExp = ParseHex(gGame.level.shipExplosion);
    AddExplosionColored(sx, sy, true, shipExp);
    if (s->podConnected) {
        gGame.pod.active = false;
        AddExplosionColored(SXf(gGame.pod.x), SYf(gGame.pod.y), true, shipExp);
    }

    if (!gGame.cheatUnlimLives) gGame.lives--;
    if (gGame.fuel < 1) gGame.lives = -1;

    if (gGame.lives > -1) {
        SetPause(jumpLevel ? GS_MISSION_FAILED : GS_START_LIFE, 3000);
    } else {
        SetPause(GS_GAME_OVER, 2800);
    }
}

// ===================== COLLISION DETECTION =====================
// Coll type and CheckIntersect/CheckIntersectCircle primitives are in Collision.h / Collision.c

static Coll ShipVsSegment(float b1x, float b1y, float b2x, float b2y) {
    Ship *s = &gGame.ship;
    float ox, oy;
    // nose-tail
    if (fabsf(s->cNose.x - s->cTail.x) < 600) {
        if (CheckIntersect(b1x,b1y,b2x,b2y, s->cTail.x,s->cTail.y,s->cNose.x,s->cNose.y,&ox,&oy))
            return (Coll){true,ox,oy};
        if (CheckIntersect(b1x,b1y,b2x,b2y, s->pTail.x,s->pTail.y,s->pNose.x,s->pNose.y,&ox,&oy))
            return (Coll){true,ox,oy};
    }
    // wing-wing
    if (fabsf(s->cWingP.x - s->cWingS.x) < 600) {
        if (CheckIntersect(b1x,b1y,b2x,b2y, s->cWingP.x,s->cWingP.y,s->cWingS.x,s->cWingS.y,&ox,&oy))
            return (Coll){true,ox,oy};
        if (CheckIntersect(b1x,b1y,b2x,b2y, s->pWingP.x,s->pWingP.y,s->pWingS.x,s->pWingS.y,&ox,&oy))
            return (Coll){true,ox,oy};
    }
    return (Coll){false};
}

static Coll ShipVsCircle(float cx, float cy, float r) {
    Ship *s = &gGame.ship;
    float ox, oy;
    if (CheckIntersectCircle(cx,cy,r, s->pNose.x,s->pNose.y,s->cNose.x,s->cNose.y,&ox,&oy)) return (Coll){true,ox,oy};
    if (CheckIntersectCircle(cx,cy,r, s->pTail.x,s->pTail.y,s->cTail.x,s->cTail.y,&ox,&oy)) return (Coll){true,ox,oy};
    if (CheckIntersectCircle(cx,cy,r, s->pWingS.x,s->pWingS.y,s->cWingS.x,s->cWingS.y,&ox,&oy)) return (Coll){true,ox,oy};
    if (CheckIntersectCircle(cx,cy,r, s->pWingP.x,s->pWingP.y,s->cWingP.x,s->cWingP.y,&ox,&oy)) return (Coll){true,ox,oy};
    return (Coll){false};
}

static Coll ShipVsBullet(Bullet *b) {
    Ship *s = &gGame.ship;
    float ox, oy;
    if (CheckIntersect(b->prevX,b->prevY,b->x,b->y, s->cNose.x,s->cNose.y,s->cTail.x,s->cTail.y,&ox,&oy)) return (Coll){true,ox,oy};
    if (CheckIntersect(b->prevX,b->prevY,b->x,b->y, s->cWingP.x,s->cWingP.y,s->cWingS.x,s->cWingS.y,&ox,&oy)) return (Coll){true,ox,oy};
    return (Coll){false};
}

// ===================== REACTOR COUNTDOWN =====================
static void ReactorCountdown(void) {
    Reactor *r = &gGame.reactor;
    r->timer = gGame.age + 21.0f;
    if (r->countdown > 0) {
        r->countdown--;
    } else {
        r->timer += 10000;
        gPlanetExpPos = 1;
        // Destroy everything
        Color se = ParseHex(gGame.level.shipExplosion);
        AddExplosionColored(SXf(gGame.ship.x), SYf(gGame.ship.y), true, se);
        ShipDie(SXf(gGame.ship.x), SYf(gGame.ship.y), true);
        gGame.pod.active = false;
        for (int i = 0; i < gGame.enemyCount; i++) {
            Color ee = ParseHex(gGame.level.enemyExplosion);
            AddExplosionColored(SXf(gGame.enemies[i].gun.x), SYf(gGame.enemies[i].gun.y), true, ee);
        }
        gGame.enemyCount = 0;
        for (int i = 0; i < gGame.tankCount; i++) {
            Color te = ParseHex(gGame.level.tankExplosion);
            AddExplosionColored(SXf(gGame.tanks[i].x), SYf(gGame.tanks[i].y), true, te);
        }
        gGame.tankCount = 0;
        Color re = ParseHex(gGame.level.reactorExplosion);
        AddExplosionColored(SXf(r->x), SYf(r->y), true, re);
        r->active = false;
    }
}

// ===================== REDRAW (INFLIGHT) =====================
static void ReDraw(void) {
    bool paused = gGame.ship.paused;
    if (!paused) {
    gGame.age += gTick;

    // Clear arena
    DrawRectangle(0, HUD_H, VIEWPORT_W, VIEWPORT_H, C_BLACK);

    // Reactor countdown
    if (gGame.reactor.countdownStarted && gGame.reactor.timer <= gGame.age)
        ReactorCountdown();

    // Planet explosion flash
    if (gPlanetExpPos > 0) {
        if (gPlanetExpPos < 10) {
            DrawRectangle(0, HUD_H, VIEWPORT_W, VIEWPORT_H, gPlanetExpColors[(int)gPlanetExpPos-1]);
        }
        gPlanetExpPos += gTick;
    }

    // Ship physics
    ShipCalcPosition();

    // Bullet physics
    for (int i = gGame.bulletCount-1; i >= 0; i--) {
        Bullet *b = &gGame.bullets[i];
        b->age += gTick;
        if (b->age > b->maxAge) {
            gGame.bullets[i] = gGame.bullets[--gGame.bulletCount];
            continue;
        }
        b->prevX = b->x; b->prevY = b->y;
        b->x += b->vx * gTick; b->y += b->vy * gTick;
    }

    // ---- COLLISION DETECTION ----
    if (gGame.ship.active) {
        // Ship vs terrain
        for (int i = 0; i < gGame.level.lsCount-1 && gGame.ship.active; i++) {
            Coll c = ShipVsSegment(gGame.level.landscape[i].x, gGame.level.landscape[i].y,
                                    gGame.level.landscape[i+1].x, gGame.level.landscape[i+1].y);
            if (c.hit) { ShipDie(SXf(c.x), SYf(c.y), false); break; }
        }
        // Ship vs enemies
        for (int i = gGame.enemyCount-1; i >= 0 && gGame.ship.active; i--) {
            Enemy *e = &gGame.enemies[i];
            Coll c = ShipVsSegment(e->body[5].x,e->body[5].y,e->body[0].x,e->body[0].y);
            if (c.hit) { ShipDie(SXf(c.x), SYf(c.y), false); break; }
        }
        // Ship vs reactor
        if (gGame.ship.active) {
            Reactor *r = &gGame.reactor;
            Coll c = ShipVsCircle(r->x, r->y, r->radius);
            if (c.hit) ShipDie(SXf(c.x), SYf(c.y), false);
        }
        // Ship vs fuel tanks
        for (int i = gGame.tankCount-1; i >= 0 && gGame.ship.active; i--) {
            Tank *t = &gGame.tanks[i];
            for (int j = 0; j < 3 && gGame.ship.active; j++) {
                Coll c = ShipVsSegment(t->corners[j].x,t->corners[j].y,t->corners[j+1].x,t->corners[j+1].y);
                if (c.hit) { ShipDie(SXf(c.x), SYf(c.y), false); break; }
            }
        }
        // Ship vs doors
        for (int di = 0; di < gGame.doorCount && gGame.ship.active; di++) {
            Door *d = &gGame.doors[di];
            const DoorDef *def = d->def;
            int st = d->state, vc = def->vertCount[st];
            for (int j = 0; j < vc-1 && gGame.ship.active; j++) {
                Coll c = ShipVsSegment(
                    def->x+def->verts[st][j][0],   def->y+def->verts[st][j][1],
                    def->x+def->verts[st][j+1][0], def->y+def->verts[st][j+1][1]);
                if (c.hit) { ShipDie(SXf(c.x), SYf(c.y), false); break; }
            }
            // Ship vs keyholes
            for (int k = 0; k < def->keyholeCount && gGame.ship.active; k++) {
                Coll c = ShipVsCircle(def->keyholes[k][0], def->keyholes[k][1], 13);
                if (c.hit) ShipDie(SXf(c.x), SYf(c.y), false);
            }
        }

        // Pod collision (if connected)
        if (gGame.ship.podConnected) {
            Pod *pod = &gGame.pod;
            float ox, oy;
            // Pod vs terrain
            for (int i = 0; i < gGame.level.lsCount-1 && gGame.ship.active; i++) {
                if (CheckIntersectCircle(pod->x,pod->y,pod->radius,
                    gGame.level.landscape[i].x,gGame.level.landscape[i].y,
                    gGame.level.landscape[i+1].x,gGame.level.landscape[i+1].y,&ox,&oy))
                { ShipDie(SXf(gGame.ship.x), SYf(gGame.ship.y), false); break; }
            }
            // Pod vs reactor
            if (gGame.ship.active) {
                Reactor *r = &gGame.reactor;
                if (Dist(pod->x,pod->y,r->x,r->y) <= pod->radius + r->radius)
                    ShipDie(SXf(gGame.ship.x), SYf(gGame.ship.y), false);
            }
            // Pod vs keyholes (doors)
            for (int di = 0; di < gGame.doorCount && gGame.ship.active; di++) {
                Door *d = &gGame.doors[di];
                const DoorDef *def = d->def;
                for (int k = 0; k < def->keyholeCount && gGame.ship.active; k++) {
                    if (Dist(pod->x,pod->y, def->keyholes[k][0], def->keyholes[k][1]) <= pod->radius+13)
                        ShipDie(SXf(gGame.ship.x), SYf(gGame.ship.y), false);
                }
            }
        } else {
            // Ship vs pod (when not connected)
            Coll c = ShipVsCircle(gGame.pod.x, gGame.pod.y, gGame.pod.radius);
            if (c.hit && gGame.ship.active) ShipDie(SXf(c.x), SYf(c.y), false);
        }

        // Bullet collisions
        for (int bi = gGame.bulletCount-1; bi >= 0; bi--) {
            Bullet *b = &gGame.bullets[bi];
            bool removed = false;
            float ox, oy;

            // All bullets vs terrain
            for (int i = 0; i < gGame.level.lsCount-1 && !removed; i++) {
                if (CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                    gGame.level.landscape[i].x,gGame.level.landscape[i].y,
                    gGame.level.landscape[i+1].x,gGame.level.landscape[i+1].y,&ox,&oy))
                {
                    gGame.bullets[bi] = gGame.bullets[--gGame.bulletCount]; removed=true;
                }
            }
            if (removed || bi >= gGame.bulletCount) continue;

            // Bullets vs doors
            for (int di = 0; di < gGame.doorCount && !removed; di++) {
                Door *d = &gGame.doors[di];
                const DoorDef *def = d->def;
                int st = d->state, vc = def->vertCount[st];
                for (int j = 0; j < vc-1 && !removed; j++) {
                    if (CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                        def->x+def->verts[st][j][0], def->y+def->verts[st][j][1],
                        def->x+def->verts[st][j+1][0], def->y+def->verts[st][j+1][1],&ox,&oy))
                    { gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true; }
                }
                // Bullets vs keyholes
                for (int k = 0; k < def->keyholeCount && !removed; k++) {
                    if (CheckIntersectCircle(def->keyholes[k][0],def->keyholes[k][1],14,
                        b->prevX,b->prevY,b->x,b->y,&ox,&oy))
                    {
                        if (!b->enemyFire && !d->inProgress) {
                            d->inProgress = true;
                            d->movement = 1;
                            AddExplosionColored(SXf(ox),SYf(oy),false,C_YELLOW);
                        }
                        gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true;
                    }
                }
            }
            if (removed || bi >= gGame.bulletCount) continue;

            // Bullets vs pod+rod (enemy bullets only)
            if (b->enemyFire && gGame.ship.podConnected) {
                // rod
                if (CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                    gGame.ship.x,gGame.ship.y,gGame.pod.x,gGame.pod.y,&ox,&oy))
                {
                    gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true;
                    ShipDie(SXf(ox),SYf(oy),false);
                }
            }
            if (removed || bi >= gGame.bulletCount) continue;

            // Bullets vs pod
            if (gGame.ship.podConnected) {
                Pod *pod = &gGame.pod;
                if (CheckIntersectCircle(pod->x,pod->y,pod->radius,
                    b->prevX,b->prevY,b->x,b->y,&ox,&oy))
                {
                    gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true;
                    ShipDie(SXf(gGame.ship.x),SYf(gGame.ship.y),false);
                }
            }
            if (removed || bi >= gGame.bulletCount) continue;

            if (b->enemyFire) {
                // Enemy bullet vs ship
                if (!gGame.ship.shield) {
                    Coll c = ShipVsBullet(b);
                    if (c.hit) {
                        ShipDie(SXf(c.x),SYf(c.y),false);
                        gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true;
                    }
                }
            } else {
                // Player bullet vs enemies
                for (int ei = gGame.enemyCount-1; ei >= 0 && !removed; ei--) {
                    Enemy *e = &gGame.enemies[ei];
                    // Check top plane and two sides
                    if (CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                        e->body[5].x,e->body[5].y,e->body[0].x,e->body[0].y,&ox,&oy) ||
                        CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                        e->body[0].x,e->body[0].y,e->body[1].x,e->body[1].y,&ox,&oy) ||
                        CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                        e->body[4].x,e->body[4].y,e->body[5].x,e->body[5].y,&ox,&oy))
                    {
                        Color ee = ParseHex(gGame.level.enemyExplosion);
                        AddExplosionColored(SXf(ox),SYf(oy),true,ee);
                        gGame.enemies[ei] = gGame.enemies[--gGame.enemyCount];
                        gGame.bullets[bi] = gGame.bullets[--gGame.bulletCount]; removed=true;
                        gGame.score += 750;
                    }
                }
                if (removed || bi >= gGame.bulletCount) continue;

                // Player bullet vs reactor
                Reactor *r = &gGame.reactor;
                if (CheckIntersectCircle(r->x,r->y,20, b->prevX,b->prevY,b->x,b->y,&ox,&oy)) {
                    gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true;
                    r->damage += 100;
                    if (r->drawSmoke < gGame.age+50.0f) r->drawSmoke = gGame.age+50.0f;
                    r->drawSmoke += 20.0f;
                    r->smokeY[0]=0; r->smokeY[1]=9;
                    if (!r->countdownStarted && r->damage >= r->maxDamage) {
                        r->countdownStarted = true;
                        ReactorCountdown();
                    }
                    Color re = ParseHex(gGame.level.reactorExplosion);
                    AddExplosionColored(SXf(ox),SYf(oy),false,re);
                }
                if (removed || bi >= gGame.bulletCount) continue;

                // Player bullet vs fuel tanks
                for (int ti = gGame.tankCount-1; ti >= 0 && !removed; ti--) {
                    Tank *t = &gGame.tanks[ti];
                    for (int j = 0; j < 3 && !removed; j++) {
                        if (CheckIntersect(b->prevX,b->prevY,b->x,b->y,
                            t->corners[j].x,t->corners[j].y,t->corners[j+1].x,t->corners[j+1].y,&ox,&oy))
                        {
                            Color te = ParseHex(gGame.level.tankExplosion);
                            AddExplosionColored(SXf(ox),SYf(oy),true,te);
                            gGame.tanks[ti]=gGame.tanks[--gGame.tankCount];
                            gGame.bullets[bi]=gGame.bullets[--gGame.bulletCount]; removed=true;
                        }
                    }
                }
            }
        }
    } // end if ship.active
    } // end if !paused

    // ---- DRAWING ----
    if (!paused) ShipScrollViewport();  // update viewport offset before any drawing
    BeginZoom();
    UpdateAndDrawExplosions();

    // Enemies: draw + fire
    for (int i = gGame.enemyCount-1; i >= 0; i--) {
        Enemy *e = &gGame.enemies[i];
        DrawEnemy(e);
        // Fire
        if (!paused && gGame.age > gGame.reactor.drawSmoke &&
            (float)rand()/RAND_MAX < e->aggression &&
            e->gun.y > gGame.arena.vpOfsY - 100 &&
            e->gun.y < VIEWPORT_H + gGame.arena.vpOfsY + 100)
        {
            if (gGame.bulletCount < MAX_BULLETS) {
                float bangle = RTD(e->ori) - ((float)rand()/RAND_MAX)*e->gunAngleRange - e->gunAngleOfs;
                V2 bv = CalcPt(bangle, 17);
                Bullet *nb = &gGame.bullets[gGame.bulletCount++];
                nb->x = e->gun.x; nb->y = e->gun.y;
                nb->prevX = nb->x; nb->prevY = nb->y;
                nb->vx = bv.x; nb->vy = bv.y;
                nb->enemyFire = true;
                nb->age = 0;
                nb->maxAge = (int)(sqrtf(VIEWPORT_W*VIEWPORT_W+VIEWPORT_H*VIEWPORT_H)/15);
            }
        }
    }

    // Fuel tanks: draw + refuel
    int activeTanks = 0;
    for (int i = gGame.tankCount-1; i >= 0; i--) {
        Tank *t = &gGame.tanks[i];
        if (t->fuelLoad > 0) {
            DrawTank(t);
            Ship *s = &gGame.ship;
            if (!paused && s->shield && !s->podConnected &&
                s->x >= t->refuelZone.l && s->x <= t->refuelZone.r &&
                s->y >= t->refuelZone.t && s->y <= t->refuelZone.b)
            {
                s->refuelling = true;
                t->fuelLoad -= 12.0f * gTick;
                if (!gGame.cheatUnlimFuel) gGame.fuel += 12.0f * gTick;
                activeTanks++;
            }
        } else {
            gGame.tanks[i] = gGame.tanks[--gGame.tankCount];
            gGame.score += 300;
            gGame.ship.refuelling = false;
        }
    }
    if (activeTanks == 0) gGame.ship.refuelling = false;

    // Reactor
    DrawReactor();

    // Ship + pod draw
    DrawPod();
    DrawShip();

    // Pod interaction (tractor beam)
    if (!paused && !gGame.ship.podConnected) {
        if (gGame.ship.shield) {
            if (Dist(gGame.ship.x,gGame.ship.y,gGame.pod.x,gGame.pod.y) < ROD_LEN-3)
                gGame.ship.podInitiate = true;
            if (gGame.ship.podInitiate) {
                DrawPodRod();
                if (Dist(gGame.ship.x,gGame.ship.y,gGame.pod.x,gGame.pod.y) >= ROD_LEN) {
                    gGame.ship.podConnected = true;
                    gGame.ship.podInitiate = false;
                    gGame.pod.vy = 3;
                }
            }
        } else {
            gGame.ship.podInitiate = false;
        }
    } else if (gGame.ship.podInitiate) {
        DrawPodRod();
    }
    if (gGame.ship.podConnected) DrawPodRod();

    // Draw bullets
    for (int i = 0; i < gGame.bulletCount; i++) {
        Bullet *b = &gGame.bullets[i];
        Color bc = b->enemyFire ? ParseHex(gGame.level.enemyBulletColor) : ParseHex(gGame.level.shipBulletColor);
        DrawRectangle(SX(b->x)-1, SY(b->y)-1, 3, 3, bc);
    }

    // Stars
    DrawStars();

    // Doors
    bool invis = gPlanetExpPos >= 9 ? true :
                 (gGame.invisTerrain && !gGame.ship.shield);
    for (int i = 0; i < gGame.doorCount; i++) {
        Door *d = &gGame.doors[i];
        // Update door state
        if (!paused && d->beginCloseAge >= 0.0f && gGame.age >= d->beginCloseAge) {
            d->movement = -1; d->beginCloseAge = -1.0f;
        }
        if (!paused && d->movement != 0) {
            d->stateF += (float)d->movement * gTick;
            if (d->movement < 0 && d->stateF <= 0.0f) {
                d->stateF = 0.0f; d->state = 0; d->movement = 0; d->inProgress = false;
            } else if (d->movement > 0 && d->stateF >= (float)(d->def->stateCount-1)) {
                d->stateF = (float)(d->def->stateCount-1);
                d->state = d->def->stateCount-1;
                d->movement = 0;
                d->beginCloseAge = gGame.age + 140.0f;
            } else {
                d->state = (int)d->stateF;
            }
        }
        DrawDoor(d, invis);
    }

    // Landscape
    DrawLandscape(invis);

    // Paused overlay — drawn inside BeginZoom so it scales and centres with the game view
    if (paused) {
        float pcx = VIEWPORT_W * 0.5f;
        float pcy = HUD_H + VIEWPORT_H * 0.5f;
        const char *ptxt = "PAUSED";
        int ptxtW = MeasureText(ptxt, 16);
        DrawRectangle((int)(pcx - ptxtW/2 - 6), (int)(pcy - 12), ptxtW + 12, 22, C_BLACK);
        DrawText(ptxt, (int)(pcx - ptxtW/2), (int)(pcy - 8), 16, C_YELLOW);
    }

    EndZoom();
}

static char gMsgBuf[2048];  // last displayed message (redrawn each frame in GS_DO_NOTHING)

// ===================== LEVEL LOAD / GAME INIT =====================
static void LoadLevel(int lvl) {
    bool reverseGravity = false;
    gPlanetExpPos = 0;

    if (lvl > 1) gGame.prevLevel = gGame.curLevel;
    gGame.visLevel = lvl;

    if (lvl > 24) { gState = GS_DO_NOTHING; return; }

    if      (lvl > 18) { reverseGravity=true; gGame.invisTerrain=true; lvl -= 18; }
    else if (lvl > 12) { gGame.invisTerrain=true; lvl -= 12; }
    else if (lvl > 6)  { reverseGravity=true; gGame.invisTerrain=false; lvl -= 6; }
    else               { gGame.invisTerrain=false; }

    gGame.curLevel = lvl;
    gGame.level = gLevels[lvl-1];
    if (reverseGravity) gGame.level.gravity = -gGame.level.gravity;
}

static void SetNewPosition(bool hasPod, float shipX, float shipY) {
    Ship *s = &gGame.ship;
    s->x = shipX; s->y = shipY;
    s->prevX = shipX; s->prevY = shipY;
    Arena *ar = &gGame.arena;
    ar->vpOfsX = shipX - VIEWPORT_W/2.0f;
    ar->vpOfsY = shipY - VIEWPORT_H/2.5f;
    if (hasPod) {
        s->podConnected = true;
        gGame.pod.x = shipX - 17;
        gGame.pod.y = shipY + 77;
    }
}

static void StartNewLife(void) {
    LevelDef *lv = &gGame.level;
    Arena *ar = &gGame.arena;
    ar->vpOfsX = lv->shipX - VIEWPORT_W / 2.0f;
    ar->vpOfsY = lv->shipY - VIEWPORT_H / 2.5f;
    ar->slideX = ar->slideY = 0;
    ar->arenaW = lv->arenaW;
    ar->arenaH = lv->arenaH;
    ar->lvl = lv;

    gGame.bulletCount = 0;
    gGame.explosionCount = 0;
    gGame.bhCount = 0;

    // Reactor
    Reactor *r = &gGame.reactor;
    r->x = lv->reactorX; r->y = lv->reactorY;
    r->radius = 20; r->damage = 0; r->maxDamage = 1400;
    r->countdown = 10; r->timer = 0;
    r->active = true; r->countdownStarted = false;
    r->smokeY[0] = -25; r->smokeY[1] = -16;
    r->drawSmoke = -1;

    // Pod
    Pod *pod = &gGame.pod;
    pod->x = lv->podX; pod->y = lv->podY;
    pod->prevX = pod->x; pod->prevY = pod->y;
    pod->vx = 0; pod->vy = 0;
    pod->radius = 9; pod->active = true;

    // Fuel tanks
    gGame.tankCount = 0;
    for (int i = 0; i < lv->tankCount; i++)
        InitTank(&gGame.tanks[gGame.tankCount++], &lv->tanks[i]);

    // Enemies
    gGame.enemyCount = 0;
    for (int i = 0; i < lv->enemyCount; i++)
        InitEnemy(&gGame.enemies[gGame.enemyCount++], &lv->enemies[i]);

    // Doors
    gGame.doorCount = 0;
    for (int i = 0; i < lv->doorCount; i++) {
        Door *d = &gGame.doors[gGame.doorCount++];
        d->def = &gLevels[gGame.curLevel-1].doors[i];
        d->state = 0; d->movement = 0; d->stateF = 0.0f;
        d->inProgress = false; d->beginCloseAge = -1.0f;
        d->x = lv->doors[i].x; d->y = lv->doors[i].y;
    }

    // Ship
    Ship *s = &gGame.ship;
    memset(s, 0, sizeof(*s));
    s->x = lv->shipX; s->y = lv->shipY;
    s->prevX = s->x; s->prevY = s->y;
    s->thrustPow = 0.9f; s->rotSpeed = 14;
    s->active = true; s->gunLoaded = true;

    // Check restart positions
    for (int i = 0; i < lv->restartCount; i++) {
        if (!lv->restartHasPod[i] && gGame.lastDieY > lv->restartYPos[i])
            SetNewPosition(false, lv->restartShipX[i], lv->restartShipY[i]);
        if (gGame.lastDieHasPod && lv->restartHasPod[i])
            if (gGame.lastDieY < lv->restartYPos[i])
                SetNewPosition(true, lv->restartShipX[i], lv->restartShipY[i]);
    }
    gGame.lastDieY = 0;
    gGame.lastDieHasPod = false;

    InitStars();

    // Add black hole intro
    AddBlackHole(s->x, s->y, C_YELLOW, "InFlight");
    gState = GS_BLACK_HOLE;
}

static void CreateGame(void) {
    memset(&gGame, 0, sizeof(gGame));
    gGame.lives = 3;
    gGame.fuel  = 1000;
    gGame.age   = 0;
    gMsgBuf[0]  = '\0';
    LoadLevel(1);
    //LoadLevel(7);
    gPauseActive = false;
}

static void CheckHighScore(bool immediate);  // forward declaration

// ===================== INPUT HANDLING =====================
static void HandleInput(void) {
    Ship *s = &gGame.ship;

    if (!s->active) return;

    if (IsKeyPressed(BIND_PAUSE)) {
        s->paused = !s->paused;
    }
    if (IsKeyPressed(KEY_QUIT)) {
        gGame.lives = -1;
        ShipDie(-5000,-5000,false);
        CheckHighScore(true);
        return;
    }

    s->thrust = IsKeyDown(KEY_THRUST);
    s->shield = IsKeyDown(KEY_SHIELD);

    // Rotation
    if (IsKeyDown(KEY_CCW)) s->applyRot = -s->rotSpeed;
    else if (IsKeyDown(KEY_CW)) s->applyRot = s->rotSpeed;
    else s->applyRot = 0;

    // Fire
    if (IsKeyReleased(KEY_FIRE)) s->gunLoaded = true;
    if (IsKeyPressed(KEY_FIRE) && s->gunLoaded && !s->shield) {
        s->gunLoaded = false;
        if (gGame.bulletCount < MAX_BULLETS) {
            V2 bofs = CalcPt(s->ori, 16);
            V2 bv   = CalcPt(s->ori, 17);
            Bullet *nb = &gGame.bullets[gGame.bulletCount++];
            nb->x = s->x + bofs.x; nb->y = s->y + bofs.y;
            nb->prevX = nb->x; nb->prevY = nb->y;
            nb->vx = bv.x + s->avx; nb->vy = bv.y + s->avy;
            nb->enemyFire = false;
            nb->age = 0;
            nb->maxAge = (int)(sqrtf(VIEWPORT_W*VIEWPORT_W+VIEWPORT_H*VIEWPORT_H)/15);
        }
    }
}

// ===================== MAIN GAME STATE MACHINE =====================

static void ShowMessage(const char *msg) {
    DrawMessage(msg);
}

static void DoKeySelect(void) {
    // JS advances scroll every 110ms = 2.2 logical ticks (50ms/tick).
    // After JS iKeySelectPos > 168, auto-transitions to HighScoreTable.
    static const char scrollTxt[] =
        "                                                    "
        "with thanks to chris carline  lee johnson  alex cranstone  "
        "anne peattie  ammon torrence                                                    ";
    static float scrollAcc = 0.0f;
    static int   scrollPos = 0;
    static int   scrollStep = 0;   // counts advances; transition to highscore after 168

    int slen = (int)strlen(scrollTxt);
    char scroll[56] = {0};
    for (int i = 0; i < 52; i++) scroll[i] = scrollTxt[(scrollPos+i) % slen];

    snprintf(gMsgBuf, sizeof(gMsgBuf),
        "\n"
        "#ff0000rotate left:  #ffff00Z\n"
        "#ff0000rotate right: #ffff00X\n"
        "#ff0000fire:         #ffff00ENTER\n"
        "#ff0000thrust:       #ffff00SHIFT\n"
        "#ff0000shield:       #ffff00SPACE\n"
        "#ff0000pause:        #ffff00P\n"
        "#ff0000quit:         #ffff00ESCAPE\n\n"
        //"#ff00fforiginal game copyright jeremy c smith 1986\n"
        //"#00ff00recreated in javascript by jon combe\n\n"
        "\n\n#20f020Thrust recreated in raylib by HCG 2026\n\n"

        "#ffffffpress space to start\n"
        "#888888press escape to quit\n\n"
        //"#00ffff%.52s", scroll
        );
    DrawMessage(gMsgBuf);

    // Advance scroll at JS rate: 1 step per 110ms = 2.2 logical ticks
    scrollAcc += gTick;
    if (scrollAcc >= 2.2f) {
        scrollAcc -= 2.2f;
        scrollPos = (scrollPos + 1) % slen;
        scrollStep++;
        if (scrollStep > 168) {
            // JS auto-transitions to HighScoreTable after scrolling through credits
            scrollPos = 0; scrollStep = 0; scrollAcc = 0.0f;
            gState = GS_HIGHSCORE;
        }
    }

    if (IsKeyPressed(KEY_SPACE)) { gState = GS_START_GAME; }
}

static void DoHighScoreTable(void) {
    char buf[2048];
    int off = 0;
    //off += snprintf(buf+off, sizeof(buf)-off, "\n#00ff00top eight thrusters\n\n#ffff00");
    off += snprintf(buf+off, sizeof(buf)-off, "\n#00ff00hiscores (best of the best!)\n\n#ffff00");
    for (int i = 0; i < 8; i++) {
        off += snprintf(buf+off, sizeof(buf)-off, " %d. %8d  %s\n", i+1, gHighScores[i].score, gHighScores[i].name);
    }
    snprintf(buf+off, sizeof(buf)-off, "\n#ffffffpress space to start\n\n");
    DrawMessage(buf);
}

static void CheckHighScore(bool immediate) {
    bool cheating = gGame.cheatUnlimFuel || gGame.cheatUnlimLives;
    if (!cheating && gGame.score > gHighScores[7].score) {
        // Overwrite last slot, bubble up to correct sorted position
        gHighScores[7].score = gGame.score;
        gHighScores[7].name[0] = '\0';
        for (int i = 7; i > 0 && gHighScores[i].score > gHighScores[i-1].score; i--) {
            HScore tmp = gHighScores[i];
            gHighScores[i] = gHighScores[i-1];
            gHighScores[i-1] = tmp;
        }
        // Track which slot is the new entry (first empty-named slot matching score)
        gHSNewIdx = -1;
        for (int i = 0; i < 8; i++) {
            if (gHighScores[i].score == gGame.score && gHighScores[i].name[0] == '\0') {
                gHSNewIdx = i; break;
            }
        }
        gHSNewName[0] = '\0';
        gState = GS_HIGHSCORE_EDIT;
    } else {
        gHSNewIdx = -1;
        if (immediate) gState = GS_HIGHSCORE;
        else SetPause(GS_HIGHSCORE, 3000);
    }
}

static void DoHighScoreEdit(void) {
    // Keyboard: append chars, backspace, enter to confirm
    int nameLen = (int)strlen(gHSNewName);
    int key = GetCharPressed();
    while (key > 0) {
        char c = (char)key;
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (nameLen < 11 && (c == ' ' || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            gHSNewName[nameLen++] = c;
            gHSNewName[nameLen] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && nameLen > 0) {
        gHSNewName[--nameLen] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (gHSNewIdx >= 0) {
            strncpy(gHighScores[gHSNewIdx].name, gHSNewName, 15);
            gHighScores[gHSNewIdx].name[15] = '\0';
        }
        gHSNewIdx = -1;
        gState = GS_HIGHSCORE;
        return;
    }

    // Draw congratulations + table with cursor on new entry
    char buf[2048];
    int off = 0;
    off += snprintf(buf+off, sizeof(buf)-off, "#ff0000congratulations\n\n#ffff00");
    for (int i = 0; i < 8; i++) {
        if (i == gHSNewIdx) {
            off += snprintf(buf+off, sizeof(buf)-off, " %d. %8d  #00ff00%s", i+1, gHighScores[i].score, gHSNewName);
            if (nameLen < 11) {
                off += snprintf(buf+off, sizeof(buf)-off, "#ffff00_");
                for (int sp = nameLen + 1; sp < 11; sp++)
                    off += snprintf(buf+off, sizeof(buf)-off, " ");
            }
            off += snprintf(buf+off, sizeof(buf)-off, "#ffff00\n");
        } else {
            off += snprintf(buf+off, sizeof(buf)-off, " %d. %8d  %s\n", i+1, gHighScores[i].score, gHighScores[i].name);
        }
    }
    snprintf(buf+off, sizeof(buf)-off, "\n#00ff00please enter your name\n\n");
    DrawMessage(buf);
}

static void Thrust(void) {
    gFrameCount++;

    if (gState == GS_IN_FLIGHT) {
        HandleInput();
        ReDraw();
        // Check deferred (e.g. quit)
        if (gPauseActive && gFrameCount >= gPauseTimer) {
            gPauseActive = false;
            gState = gPauseTarget;
        }
        return;
    }



    // Check deferred state
    if (gPauseActive && gFrameCount >= gPauseTimer) {
        gPauseActive = false;
        gState = gPauseTarget;
    }

    switch (gState) {
    case GS_BLACK_HOLE:
        DrawRectangle(0, HUD_H, VIEWPORT_W, VIEWPORT_H, C_BLACK);
        BeginZoom();
        // Draw game objects behind black hole
        for (int i = 0; i < gGame.enemyCount; i++) DrawEnemy(&gGame.enemies[i]);
        for (int i = 0; i < gGame.tankCount; i++) if(gGame.tanks[i].fuelLoad>0) DrawTank(&gGame.tanks[i]);
        for (int i = 0; i < gGame.doorCount; i++) DrawDoor(&gGame.doors[i], false);
        DrawReactor();
        DrawLandscape(gGame.invisTerrain);
        DrawPod();
        for (int i = gGame.bhCount-1; i >= 0; i--) {
            if (DrawBlackHole(&gGame.blackHoles[i])) {
                const char *cb = gGame.blackHoles[i].callback;
                if (strcmp(cb,"InFlight")==0) {
                    DrawShip();
                    gState = GS_IN_FLIGHT;
                } else if (strcmp(cb,"MissionComplete")==0) gState = GS_MISSION_COMPLETE;
                else if (strcmp(cb,"MissionFailed")==0)     gState = GS_MISSION_FAILED;
                else if (strcmp(cb,"MissionIncomplete")==0) gState = GS_MISSION_INCOMPLETE;
                gGame.blackHoles[i] = gGame.blackHoles[--gGame.bhCount];
            }
        }
        EndZoom();
        gGame.age += gTick;
        break;

    case GS_START_GAME:
        CreateGame();
        gState = GS_START_LIFE;
        break;

    case GS_START_LIFE:
        StartNewLife();
        break;

    case GS_GAME_OVER:
        gState = GS_DO_NOTHING;
        snprintf(gMsgBuf, sizeof(gMsgBuf), "%sgame over\n\n",
            gGame.fuel < 1 ? "#ff0000out of fuel\n\n#00ff00" : "#00ff00");
        DrawMessage(gMsgBuf);
        CheckHighScore(false);
        break;

    case GS_MISSION_COMPLETE:
        gState = GS_DO_NOTHING;
        gMsgBuf[0] = '\0';
        LoadLevel(gGame.visLevel + 1);
        SetPause(GS_MISSION_COMPLETE_MSG, 1200);
        break;

    case GS_MISSION_COMPLETE_MSG: {
        gState = GS_DO_NOTHING;
        SetPause(GS_CHECK_LEVEL, 3000);
        const char *t = gLevels[gGame.prevLevel > 0 ? gGame.prevLevel-1 : 0].endColorTop;
        const char *m = gLevels[gGame.prevLevel > 0 ? gGame.prevLevel-1 : 0].endColorMid;
        const char *b = gLevels[gGame.prevLevel > 0 ? gGame.prevLevel-1 : 0].endColorBot;
        if (gGame.reactor.countdown < 10) {
            snprintf(gMsgBuf,sizeof(gMsgBuf),"%splanet destroyed\n\n%s mission %s%d%s complete",
                t, m, b, gGame.visLevel-1, m);
        } else {
            snprintf(gMsgBuf,sizeof(gMsgBuf),"%s mission %s%d%s complete",
                m, b, gGame.visLevel-1, m);
        }
        if (gBonusScore > 0) {
            char tmp[64]; snprintf(tmp,sizeof(tmp),"%s\n\n%sbonus %d\n\n",gMsgBuf,b,gBonusScore);
            strcpy(gMsgBuf, tmp);
            gGame.score += gBonusScore;
            gBonusScore = 0;
        }
        DrawMessage(gMsgBuf);
        break;
    }

    case GS_MISSION_FAILED:
        gState = GS_DO_NOTHING;
        gMsgBuf[0] = '\0';
        LoadLevel(gGame.visLevel);
        SetPause(GS_MISSION_FAILED_MSG, 1200);
        break;

    case GS_MISSION_FAILED_MSG: {
        gState = GS_DO_NOTHING;
        SetPause(GS_CHECK_LEVEL, 3000);
        const char *t = gLevels[gGame.prevLevel > 0 ? gGame.prevLevel-1 : 0].endColorTop;
        const char *m = gLevels[gGame.prevLevel > 0 ? gGame.prevLevel-1 : 0].endColorMid;
        const char *b = gLevels[gGame.prevLevel > 0 ? gGame.prevLevel-1 : 0].endColorBot;
        snprintf(gMsgBuf,sizeof(gMsgBuf),"%splanet destroyed\n\n%smission %s%d%s failed\n\n%sno bonus",
            t, m, b, gGame.visLevel-1, m, b);
        DrawMessage(gMsgBuf);
        break;
    }

    case GS_MISSION_INCOMPLETE:
        gState = GS_DO_NOTHING;
        gMsgBuf[0] = '\0';
        SetPause(GS_MISSION_INCOMPLETE_MSG, 1200);
        break;

    case GS_MISSION_INCOMPLETE_MSG:
        SetPause(GS_START_LIFE, 3000);
        gState = GS_DO_NOTHING;
        snprintf(gMsgBuf, sizeof(gMsgBuf), "#00ff00mission incomplete\n\n");
        DrawMessage(gMsgBuf);
        break;

    case GS_CHECK_LEVEL:
        switch (gGame.visLevel) {
            case 7:  gState=GS_DO_NOTHING; snprintf(gMsgBuf,sizeof(gMsgBuf),"#00ff00reverse gravity"); DrawMessage(gMsgBuf); SetPause(GS_START_LIFE,3000); break;
            case 13: gState=GS_DO_NOTHING; snprintf(gMsgBuf,sizeof(gMsgBuf),"#00ff00normal gravity\n\ninvisible planet"); DrawMessage(gMsgBuf); SetPause(GS_START_LIFE,3000); break;
            case 19: gState=GS_DO_NOTHING; snprintf(gMsgBuf,sizeof(gMsgBuf),"#00ff00reverse gravity\n\ninvisible planet"); DrawMessage(gMsgBuf); SetPause(GS_START_LIFE,3000); break;
            default: gState = GS_START_LIFE; break;
        }
        break;

    case GS_KEY_SELECT:
        // DoKeySelect stays in this state for scroll animation; input handled inside
        DoKeySelect();
        break;

    case GS_HIGHSCORE_EDIT:
        gPauseActive = false;  // cancel any pending pause timer so it can't skip name entry
        DoHighScoreEdit();
        break;

    case GS_HIGHSCORE:
        // Entry point: schedule return to KeySelect, then display persistently
        gState = GS_HIGHSCORE_SHOW;
        SetPause(GS_KEY_SELECT, 12000);
        // fall through to draw first frame immediately
    case GS_HIGHSCORE_SHOW:
        // Redrawn every frame (raylib clears each frame)
        DoHighScoreTable();
        if (IsKeyPressed(KEY_SPACE)) { gPauseActive = false; gState = GS_START_GAME; }
        break;

    case GS_DO_NOTHING:
        // raylib clears each frame; redraw the last message so it stays visible
        DrawHUD();
        if (gMsgBuf[0]) ShowMessage(gMsgBuf);
        // Only allow space-to-restart when there is no pending auto-transition (gPauseActive).
        // Every mission-complete/failed/incomplete path through DO_NOTHING has an active timer,
        // so this prevents accidentally restarting from level 1 while the transition plays out.
        if (!gPauseActive && IsKeyPressed(KEY_SPACE)) { gState = GS_START_GAME; }
        break;

    default: break;
    }
}

// ===================== MAIN =====================
int main(void) {
    srand((unsigned)time(NULL));
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowSizeX = 1024;
    int windowSizeY = 768;
    InitWindow(windowSizeX, windowSizeY, "ThrustHCG");
    //ToggleFullscreen();

    //InitWindow(VIEWPORT_W, SCREEN_H, "Thrust");
    SetTargetFPS(GAME_FPS);
    gHudTexture = LoadTexture("bg.gif");

    // Initialize game state
    CreateGame();
    gState = GS_KEY_SELECT;

    while (!WindowShouldClose()) {
        gTick = GetFrameTime() * BASE_FPS;
        if (gTick > 3.0f) gTick = 3.0f;  // cap at 3 logical ticks (handles minimise/pause)
        gZoom = (float)(GetScreenHeight() - HUD_H) / VIEWPORT_H;

        if (IsKeyPressed(KEY_F1)){
            ToggleFullscreen();
        }

        //gZoom = 1.0f/(470.0f/VIEWPORT_H);

        BeginDrawing();
        ClearBackground(C_BLACK);
        Thrust();
        DrawHUD();
        EndDrawing();
    }

    UnloadTexture(gHudTexture);
    CloseWindow();
    return 0;
}
