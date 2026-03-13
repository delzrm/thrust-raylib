// debug.c — Renders collision geometry in pink for debugging.
// Toggle with F3. Drawn inside BeginZoom/EndZoom so it scales with the view.
#include "debug.h"
#include "raylib.h"
#include "GameTypes.h"

bool gDebugCollision = false;

extern Game gGame;

// World-to-screen helpers (mirrors thrust.c's static SXf/SYf)
static inline int dbSX(float wx) { return (int)(wx - gGame.arena.vpOfsX); }
static inline int dbSY(float wy) { return (int)(wy - gGame.arena.vpOfsY + HUD_H); }

void DrawDebugCollision(void) {
    if (!gDebugCollision) return;

    const Color C = PINK;

    // --- Ship: two collision segments (nose-tail, wingP-wingS) ---
    Ship *s = &gGame.ship;
    if (s->active) {
        DrawLine(dbSX(s->cNose.x),  dbSY(s->cNose.y),
                 dbSX(s->cTail.x),  dbSY(s->cTail.y),  C);
        DrawLine(dbSX(s->cWingP.x), dbSY(s->cWingP.y),
                 dbSX(s->cWingS.x), dbSY(s->cWingS.y), C);
    }

    // --- Pod: circle ---
    Pod *pod = &gGame.pod;
    if (pod->active) {
        DrawCircleLines(dbSX(pod->x), dbSY(pod->y), pod->radius, C);
    }

    // --- Reactor: circle ---
    Reactor *r = &gGame.reactor;
    if (r->active) {
        DrawCircleLines(dbSX(r->x), dbSY(r->y), r->radius, C);
    }

    // --- Enemies: 3 collision edges (top, right side, left side) ---
    for (int i = 0; i < gGame.enemyCount; i++) {
        Enemy *e = &gGame.enemies[i];
        DrawLine(dbSX(e->body[5].x), dbSY(e->body[5].y),
                 dbSX(e->body[0].x), dbSY(e->body[0].y), C); // top
        DrawLine(dbSX(e->body[0].x), dbSY(e->body[0].y),
                 dbSX(e->body[1].x), dbSY(e->body[1].y), C); // right
        DrawLine(dbSX(e->body[4].x), dbSY(e->body[4].y),
                 dbSX(e->body[5].x), dbSY(e->body[5].y), C); // left
    }

    // --- Tanks: 3 collision edges (corners 0-1, 1-2, 2-3) ---
    for (int i = 0; i < gGame.tankCount; i++) {
        Tank *t = &gGame.tanks[i];
        for (int j = 0; j < 3; j++) {
            DrawLine(dbSX(t->corners[j].x),   dbSY(t->corners[j].y),
                     dbSX(t->corners[j+1].x), dbSY(t->corners[j+1].y), C);
        }
    }

    // --- Bullets: segment from previous to current position ---
    for (int i = 0; i < gGame.bulletCount; i++) {
        Bullet *b = &gGame.bullets[i];
        DrawLine(dbSX(b->prevX), dbSY(b->prevY),
                 dbSX(b->x),     dbSY(b->y),     C);
    }

    // --- Doors: current-state polygon edges + keyhole circles ---
    for (int i = 0; i < gGame.doorCount; i++) {
        Door *d = &gGame.doors[i];
        const DoorDef *def = d->def;
        int st = d->state, vc = def->vertCount[st];
        for (int j = 0; j < vc - 1; j++) {
            DrawLine(dbSX(def->x + def->verts[st][j][0]),
                     dbSY(def->y + def->verts[st][j][1]),
                     dbSX(def->x + def->verts[st][j+1][0]),
                     dbSY(def->y + def->verts[st][j+1][1]), C);
        }
        for (int k = 0; k < def->keyholeCount; k++) {
            DrawCircleLines(dbSX(def->keyholes[k][0]),
                            dbSY(def->keyholes[k][1]), 13, C);
        }
    }

    
    // --- Landscape: all terrain segments ---
    LevelDef *lv = &gGame.level;
    for (int i = 0; i < lv->lsCount - 1; i++) {
        DrawLine(dbSX(lv->landscape[i].x),   dbSY(lv->landscape[i].y),
                 dbSX(lv->landscape[i+1].x), dbSY(lv->landscape[i+1].y), C);
    }
    
}
