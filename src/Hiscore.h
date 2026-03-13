#pragma once
#include "GameTypes.h"

// ===================== HIGH SCORES =====================
typedef struct { char name[16]; int score; } HScore;

extern HScore gHighScores[8];

// Check if gGame.score qualifies for the high score table.
// immediate=true transitions to GS_HIGHSCORE immediately (no delay).
void CheckHighScore(bool immediate);

// Draw the high score table (called every frame while in GS_HIGHSCORE_SHOW).
void DoHighScoreTable(void);

// Handle name-entry for a new high score (called every frame while in GS_HIGHSCORE_EDIT).
void DoHighScoreEdit(void);
