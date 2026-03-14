#include "raylib.h"
#include "rlgl.h"
#include "GameTypes.h"
#include "Draw.h"
#include "Collision.h"
#include "VectorFont.h"
#include "HUD.h"
#include "Colours.h"
#include "Explosions.h"
#include "BlackHoles.h"
#include "Hiscore.h"
#include "Input.h"
#include "debug.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

Texture2D SprSheet;

void    LoadSprites(void)
{
    SprSheet = LoadTexture("sprsheet1.png"); 
}
void    FreeSprites(void)
{
    UnloadTexture(SprSheet);       // Texture unloading
}


void DrawSprite(int x,int y,int sprnum)
{
    rlPushMatrix();
//    rlTranslatef(dstX, dstY, 0.0f);
    rlScalef(2.0f, 2.0f, 1.0f);

    Vector2 position = { (x-24)/2.0f,(y-24)/2.0f };
    Rectangle frameRec = { 0.0f, 0.0f, 24.0f, 24.0f };
    DrawTextureRec(SprSheet, frameRec, position, WHITE);  // Draw part of the texture
    rlPopMatrix();

}

