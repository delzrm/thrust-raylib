#include "Hiscore.h"
#include "HUD.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

// ===================== EXTERN DEPENDENCIES =====================
// Defined in thrust.c
extern Game       gGame;
extern GameState  gState;
extern float      gZoom;
void SetPause(GameState target, int ms);

// ===================== HIGH SCORE DATA =====================
HScore gHighScores[8] = {
    {"HCG",200000},{"Delz",150000},{"Hayes",100000},
    {"CHS",50000},{"RDA",20000},{"Super",15000},{"Space",5000},{"Towers",1000}
};
static int  gHSNewIdx  = -1;
static char gHSNewName[16] = {0};

// ===================== FUNCTIONS =====================
void CheckHighScore(bool immediate) {
    bool cheating = gGame.cheatUnlimFuel || gGame.cheatUnlimLives;
    if (!cheating && gGame.score > gHighScores[7].score) {
        gHighScores[7].score = gGame.score;
        gHighScores[7].name[0] = '\0';
        for (int i = 7; i > 0 && gHighScores[i].score > gHighScores[i-1].score; i--) {
            HScore tmp = gHighScores[i];
            gHighScores[i] = gHighScores[i-1];
            gHighScores[i-1] = tmp;
        }
        gHSNewIdx = -1;
        for (int i = 0; i < 8; i++) {
            if (gHighScores[i].score == gGame.score && gHighScores[i].name[0] == '\0') {
                gHSNewIdx = i; break;
            }
        }
        gHSNewName[0] = '\0';
        gState = GS_HIGHSCORE_EDIT;
    } else {
        gHSNewIdx = -1;
        if (immediate) gState = GS_HIGHSCORE;
        else SetPause(GS_HIGHSCORE, 3000);
    }
}

void DoHighScoreTable(void) {
    char buf[2048];
    int off = 0;
    off += snprintf(buf+off, sizeof(buf)-off, "\n#00ff00hiscores (best of the best!)\n\n#ffff00");
    for (int i = 0; i < 8; i++) {
        off += snprintf(buf+off, sizeof(buf)-off, " %d. %8d  %s\n", i+1, gHighScores[i].score, gHighScores[i].name);
    }
    snprintf(buf+off, sizeof(buf)-off, "\n#ffffffpress space to start\n\n");
    DrawMessage(buf, gZoom);
}

void DoHighScoreEdit(void) {
    int nameLen = (int)strlen(gHSNewName);
    int key = GetCharPressed();
    while (key > 0) {
        char c = (char)key;
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (nameLen < 11 && (c == ' ' || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
            gHSNewName[nameLen++] = c;
            gHSNewName[nameLen] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && nameLen > 0) {
        gHSNewName[--nameLen] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (gHSNewIdx >= 0) {
            snprintf(gHighScores[gHSNewIdx].name, sizeof(gHighScores[gHSNewIdx].name), "%s", gHSNewName);
        }
        gHSNewIdx = -1;
        gState = GS_HIGHSCORE;
        return;
    }

    char buf[2048];
    int off = 0;
    off += snprintf(buf+off, sizeof(buf)-off, "#ff0000congratulations\n\n#ffff00");
    for (int i = 0; i < 8; i++) {
        if (i == gHSNewIdx) {
            off += snprintf(buf+off, sizeof(buf)-off, " %d. %8d  #00ff00%s", i+1, gHighScores[i].score, gHSNewName);
            if (nameLen < 11) {
                off += snprintf(buf+off, sizeof(buf)-off, "#ffff00_");
                for (int sp = nameLen + 1; sp < 11; sp++)
                    off += snprintf(buf+off, sizeof(buf)-off, " ");
            }
            off += snprintf(buf+off, sizeof(buf)-off, "#ffff00\n");
        } else {
            off += snprintf(buf+off, sizeof(buf)-off, " %d. %8d  %s\n", i+1, gHighScores[i].score, gHighScores[i].name);
        }
    }
    snprintf(buf+off, sizeof(buf)-off, "\n#00ff00please enter your name\n\n");
    DrawMessage(buf, gZoom);
}
