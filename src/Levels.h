#pragma once
// ===================== LEVEL DATA TYPES & DECLARATIONS =====================
#include "Colours.h"
#include <stdbool.h>

#define MAX_LS       55   // max landscape points per level
#define MAX_ENEMIES  15
#define MAX_TANKS    12
#define MAX_DOORS    3
#define MAX_DSTATES  13   // door animation states
#define MAX_DVERTS   6    // door polygon vertices per state
#define MAX_KEYS     3    // keyhole count per door
#define MAX_RESTART  8
#define NUM_LEVELS   6

typedef struct { float x, y; } LPt;
typedef struct { float x,y,ori,gunRange,gunOfs,aggression; } EnemyDef;
typedef struct { float x,y; } TankDef;
typedef struct { float x,y; } RestartPt;

typedef struct {
    float x,y;
    int stateCount;
    float verts[MAX_DSTATES][MAX_DVERTS][2];
    int vertCount[MAX_DSTATES];
    float keyholes[MAX_KEYS][2];
    int keyholeCount;
    Color doorColor;
    Color keyColor;
} DoorDef;

typedef struct {
    int arenaW, arenaH;
    float vpOfsX, vpOfsY;
    float gravity;
    Color landscapeColor;
    LPt landscape[MAX_LS]; int lsCount;
    int starCount; int starLowerLimit;
    Color starColorA, starColorB;
    float reactorX, reactorY;
    Color reactorColor, reactorChimney, reactorDoor, reactorExplosion;
    float shipX, shipY;
    Color shipBulletColor, refuelColor, shieldColor, shipExplosion;
    float podX, podY;
    Color podColor, podBaseColor, rodColor;
    EnemyDef enemies[MAX_ENEMIES]; int enemyCount;
    Color enemyColor, enemyBulletColor, enemyExplosion;
    TankDef tanks[MAX_TANKS]; int tankCount;
    Color tankColor, tankLegs, tankLabel, tankExplosion;
    DoorDef doors[MAX_DOORS]; int doorCount;
    const char *endColorTop, *endColorMid, *endColorBot;
    RestartPt restarts[MAX_RESTART]; int restartCount;
    float restartYPos[MAX_RESTART]; bool restartHasPod[MAX_RESTART];
    float restartShipX[MAX_RESTART], restartShipY[MAX_RESTART];
} LevelDef;

extern LevelDef gLevels[NUM_LEVELS];
void InitLevelColors(LevelDef *lv, int levelIdx);
