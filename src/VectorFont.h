#pragma once
// ===================== VECTOR FONT =====================
// Hand-crafted filled polygon glyphs (digits 0-9) on a 7×5 grid,
// matching the classText.js TEXT_SPACING=2.3 convention.

#include "raylib.h"

// Width of a rendered string in pixels (no trailing advance).
float VectorStrWidth(const char *s);

// Draw a string of digits at (x, y) in the given colour.
void DrawVectorStr(const char *s, float x, float y, Color col);
