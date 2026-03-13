#include "Input.h"
#include "Hiscore.h"
#include "raylib.h"
#include <math.h>

// ===================== EXTERN DEPENDENCIES =====================
// Defined in thrust.c
extern Game      gGame;
extern GameState gState;
void ShipDie(float sx, float sy, bool jumpLevel);

// ===================== KEY BINDINGS =====================
static int KEY_CCW    = KEY_Z;
static int KEY_CW     = KEY_X;
static int KEY_FIRE   = KEY_ENTER;
static int KEY_THRUST = KEY_RIGHT_SHIFT;
static int KEY_SHIELD = KEY_SPACE;
static int BIND_PAUSE = KEY_P;
static int KEY_QUIT   = KEY_ESCAPE;

// ===================== LOCAL MATH =====================
static V2 CalcPt(float angleDeg, float radius) {
    float a = (angleDeg - 90.0f) * DEG2RAD;
    return (V2){ cosf(a) * radius, sinf(a) * radius };
}

// ===================== INPUT HANDLING =====================
void HandleInput(void) {
    Ship *s = &gGame.ship;

    if (!s->active) return;

    if (IsKeyPressed(BIND_PAUSE)) {
        s->paused = !s->paused;
    } else if (s->paused && (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_X) ||
               IsKeyPressed(KEY_RIGHT_SHIFT) || IsKeyPressed(KEY_ENTER) ||
               IsKeyPressed(KEY_SPACE))) {
        s->paused = false;
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
