#pragma once
// Colours.h - compile-time Color constants for ThrustC.
// Uses CLITERAL(Color){r,g,b,255} which is (Color){r,g,b,255} in C11.
#include "raylib.h"

#define C_YELLOW   CLITERAL(Color){255,255,  0,255}
#define C_GREEN    CLITERAL(Color){  0,255,  0,255}
#define C_RED      CLITERAL(Color){255,  0,  0,255}
#define C_BLUE     CLITERAL(Color){  0,  0,255,255}
#define C_CYAN     CLITERAL(Color){  0,255,255,255}
#define C_MAGENTA  CLITERAL(Color){255,  0,255,255}
#define C_WHITE    CLITERAL(Color){255,255,255,255}
#define C_BLACK    CLITERAL(Color){  0,  0,  0,255}
#define C_GRAY     CLITERAL(Color){136,136,136,255}
#define INVISCOLOR CLITERAL(Color){  0,  0,  0,255}  // fully black - invisible door tint

// Level landscape/door colors (door colors match landscape so they blend in)
#define LEVEL1COLOR  CLITERAL(Color){136,  0,  0,255}  // #880000 - levels 1, 5
#define LEVEL2COLOR  CLITERAL(Color){  0,136,  0,255}  // #008800 - levels 2, 4
#define LEVEL3COLOR  CLITERAL(Color){  0,136,136,255}  // #008888 - level 3
#define LEVEL4COLOR  LEVEL2COLOR                        // #008800 - same as level 2
#define LEVEL5COLOR  LEVEL1COLOR                        // #880000 - same as level 1
#define LEVEL6COLOR  CLITERAL(Color){136,  0,136,255}  // #880088 - level 6
