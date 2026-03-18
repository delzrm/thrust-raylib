#pragma once
#include <stdbool.h>

// Toggle collision debug overlay (press F3 in-game)
extern bool gDebugCollision;

// Draw all collision areas in pink. Call inside BeginZoom/EndZoom.
void DrawDebugCollision(void);

// Save a screenshot to thrust_NNN.png (press F8 in-game).
void TakeDebugScreenshot(void);

// Render the full level landscape + starfield to levelN_map.png (press F4 in-game).
void SaveLevelMapTexture(void);
void SaveLineSpriteSheet(void);
void SaveShipSpriteSheet(void);