#include "Explosions.h"
#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>

#define MAX_EXPL 25
#define MAX_PART 55

typedef struct {
    float x,y,vx,vy,gravity;
    Color color;
    float age; int maxAge;
} Particle;

typedef struct {
    Particle parts[MAX_PART];
    int count;
} Explosion;

static Explosion sExplosions[MAX_EXPL];
static int sExplosionCount;

static void AddExplosion(float sx, float sy, bool big, float gravity) {
    if (sExplosionCount >= MAX_EXPL) return;
    Explosion *e = &sExplosions[sExplosionCount++];
    e->count = big ? 50 : 7;
    float grav = gravity * 2;
    for (int i = 0; i < e->count; i++) {
        Particle *p = &e->parts[i];
        p->x = sx - 1.5f; p->y = sy - 1.5f;
        p->vx = ((float)rand()/RAND_MAX)*8 - 4;
        p->vy = ((float)rand()/RAND_MAX)*8 - 4;
        p->gravity = grav;
        p->age = 0;
        p->maxAge = (int)(((float)rand()/RAND_MAX)*20 + 5);
        p->color = WHITE;
    }
}

void AddExplosionColored(float sx, float sy, bool big, Color col, float gravity) {
    if (sExplosionCount >= MAX_EXPL) return;
    int idx = sExplosionCount;
    AddExplosion(sx, sy, big, gravity);
    Explosion *e = &sExplosions[idx];
    for (int i = 0; i < e->count; i++) e->parts[i].color = col;
}

void UpdateAndDrawExplosions(float tick, float slideX, float slideY) {
    for (int i = sExplosionCount-1; i >= 0; i--) {
        Explosion *e = &sExplosions[i];
        int alive = 0;
        for (int j = e->count-1; j >= 0; j--) {
            Particle *p = &e->parts[j];
            if (p->age > p->maxAge) continue;
            p->vy += p->gravity * tick;
            p->x  += (p->vx - slideX) * tick;
            p->y  += (p->vy - slideY) * tick;
            DrawRectangle((int)p->x,(int)p->y,3,3,p->color);
            p->age += tick;
            alive++;
        }
        if (alive == 0) {
            sExplosions[i] = sExplosions[--sExplosionCount];
        }
    }
}

void ResetExplosions(void) {
    sExplosionCount = 0;
}
