// debug.c — Renders collision geometry in pink for debugging.
// Toggle with F3. Drawn inside BeginZoom/EndZoom so it scales with the view.
// Press F8 to save a screenshot (thrust_NNN.png in the working directory).
// Press F4 to save a full-level map (landscape + stars) to level_map.png.
#include "debug.h"
#include "raylib.h"
#include "GameTypes.h"
#include "Draw.h"
#include <stdio.h>

bool gDebugCollision = false;

void TakeDebugScreenshot(void) {
    static int counter = 0;
    char path[64];
    snprintf(path, sizeof(path), "thrust_%03d.png", counter++);
    TakeScreenshot(path);
    TraceLog(LOG_INFO, "Screenshot saved: %s", path);
}

extern Game gGame;

// World-to-screen helpers (mirrors thrust.c's static SXf/SYf)
static inline int dbSX(float wx) { return (int)(wx - gGame.arena.vpOfsX); }
static inline int dbSY(float wy) { return (int)(wy - gGame.arena.vpOfsY + HUD_H); }

void SaveLevelMapTexture(void) {
    LevelDef *lv = &gGame.level;
    Arena    *ar = &gGame.arena;

    int w = lv->arenaW;
    int h = lv->arenaH;

    RenderTexture2D rt = LoadRenderTexture(w, h);
    BeginTextureMode(rt);
    ClearBackground(BLACK);

    // Stars: draw at raw world coordinates (no viewport offset).
    for (int i = 0; i < lv->starCount && i < MAX_STARS; i++) {
        int sx = (int)ar->stars[i].x;
        int sy = (int)ar->stars[i].y;
        DrawRectangle(sx, sy, 2, 2, ar->starColors[i]);
    }

    // Landscape: pass vpOfsY=HUD_H so the internal +HUD_H cancels out,
    // placing terrain triangles at raw world Y coordinates.
    DrawLandscapeMesh(0.0f, (float)HUD_H, w, false);

    EndTextureMode();

    // Raylib render textures are Y-flipped; correct before saving.
    Image img = LoadImageFromTexture(rt.texture);
    ImageFlipVertical(&img);
    UnloadRenderTexture(rt);

    char path[64];
    snprintf(path, sizeof(path), "level%d_map.png", gGame.curLevel);
    ExportImage(img, path);
    TraceLog(LOG_INFO, "Level map saved: %s (%dx%d)", path, w, h);

    // Zoomed version: scale down by ZOOM using nearest-neighbour to stay sharp.
    int zw = (int)(w * ZOOM);
    int zh = (int)(h * ZOOM);
    Image imgZoom = ImageCopy(img);
    ImageResizeNN(&imgZoom, zw, zh);
    snprintf(path, sizeof(path), "level%d_map_zoom.png", gGame.curLevel);
    ExportImage(imgZoom, path);
    TraceLog(LOG_INFO, "Level map (zoom) saved: %s (%dx%d)", path, zw, zh);

    UnloadImage(imgZoom);
    UnloadImage(img);
}

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

    // --- Camera dead zones (position-based scroll triggers) ---
    // These are drawn in pre-zoom screen space (same coordinate system as BeginZoom).
    // Cyan  = ship dead zone: ship must leave this box before the camera scrolls.
    // Yellow = pod dead zone:  wider Y thresholds used when pod is connected.
    if (s->active) {
        float zoomScale = fmaxf(ZOOM, 1.0f);

        float cx   = VIEWPORT_W * 0.5f;
        float xOfs = fminf(90.0f / ZOOM / zoomScale, cx - 5.0f);

        float cy   = VIEWPORT_H * 0.5f;
        float yDn  = fminf(VIEWPORT_H * (120.0f / 470.0f) / zoomScale, cy - 5.0f);
        float yUp  = fminf(VIEWPORT_H * (140.0f / 470.0f) / zoomScale, cy - 5.0f);

        int szX = (int)(cx - xOfs);
        int szY = (int)(HUD_H + cy - yUp);
        int szW = (int)(2.0f * xOfs);
        int szH = (int)(yUp + yDn);
        DrawRectangle(szX, szY, szW, szH, (Color){0, 200, 255, 30});
        DrawRectangleLines(szX, szY, szW, szH, (Color){0, 200, 255, 140});

        if (s->podConnected) {
            float pDn = fminf(VIEWPORT_H * (150.0f / 470.0f) / zoomScale, cy - 5.0f);
            float pUp = fminf(VIEWPORT_H * (170.0f / 470.0f) / zoomScale, cy - 5.0f);
            int pzX = (int)(cx - xOfs);
            int pzY = (int)(HUD_H + cy - pUp);
            int pzW = (int)(2.0f * xOfs);
            int pzH = (int)(pUp + pDn);
            DrawRectangle(pzX, pzY, pzW, pzH, (Color){255, 220, 0, 20});
            DrawRectangleLines(pzX, pzY, pzW, pzH, (Color){255, 220, 0, 100});
        }
    }
}
