#pragma once
#include <stdbool.h>

// Toggle collision debug overlay (press F3 in-game)
extern bool gDebugCollision;

// Draw all collision areas in pink. Call inside BeginZoom/EndZoom.
void DrawDebugCollision(void);
