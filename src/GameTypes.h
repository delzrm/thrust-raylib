#pragma once
// Shared game type definitions: constants, structs, enums.
// Included by any module that needs game state types.
#include "Levels.h"
#include "HUD.h"        // for HUD_H
#include <stdbool.h>
#include <math.h>

typedef struct { float x, y; } V2;

// ===================== CONSTANTS =====================
#define FIXEDSCREENWIDTH  320
#define FIXEDSCREENHEIGHT 224

#define VIEWPORT_H      448 //470
#define ZOOM            ((float)FIXEDSCREENHEIGHT / VIEWPORT_H)
#define VIEWPORT_W      ((float)FIXEDSCREENWIDTH  / ZOOM)   // ~671
#define GAME_FPS        60
#define BASE_FPS        20.0f
#define ROD_LEN         73.0f
#define MAX_BULLETS     40
#define MAX_STARS       55
#define BULLET_MAX_AGE  54   // (int)(sqrt(VIEWPORT_W^2+VIEWPORT_H^2)/15)

// ===================== MATH UTILITIES =====================
// Angle (game degrees, 0=up) to offset vector. Used by thrust.c and Input.c.
static inline V2 CalcPt(float angleDeg, float radius) {
    float a = (angleDeg - 90.0f) * DEG2RAD;
    return (V2){ cosf(a) * radius, sinf(a) * radius };
}

// ===================== STRUCTS =====================
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
    V2 body[6];
    V2 gun, dome;
    float cx, cy;   // world-space centre (set once in InitEnemy)
    float ori;
    float gunAngleRange, gunAngleOfs, aggression;
} Enemy;

typedef struct {
    float x,y,fuelLoad;
    V2 corners[4];
    struct { float l,r,t,b; } refuelZone;
} Tank;

typedef struct {
    float x,y;
    float prevX, prevY;
    float vx,vy;
    bool enemyFire;
    float age; int maxAge;
} Bullet;

typedef struct {
    int state, movement;
    float stateF;
    float beginCloseAge;
    bool inProgress;
    const DoorDef *def;
} Door;

typedef struct {
    V2 stars[MAX_STARS];
    Color starColors[MAX_STARS];
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
    GS_RESPAWN,
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
    Door doors[MAX_DOORS]; int doorCount;
    bool cheatUnlimFuel, cheatUnlimLives;
    LevelDef level;
} Game;
