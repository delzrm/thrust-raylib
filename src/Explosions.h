#pragma once
// ===================== EXPLOSIONS =====================
#include "raylib.h"
#include <stdbool.h>

void AddExplosionColored(float sx, float sy, bool big, Color col, float gravity);
void UpdateAndDrawExplosions(float tick, float slideX, float slideY);
void ResetExplosions(void);
