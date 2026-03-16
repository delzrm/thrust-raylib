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
    SprSheet = LoadTexture("sprsheet.png"); 
}
void    FreeSprites(void)
{
    UnloadTexture(SprSheet);       // Texture unloading
}


void DrawSprite(float x,float y,int sprnum)
{
// we can get rid of 2.0 scale when everything is normalized correctly...
// vectors are currently rendered with 0.5f scale
    rlPushMatrix();
    rlScalef(2.0f, 2.0f, 1.0f);

    float frameX = (sprnum&7)*32.0f;
    float frameY = (sprnum>>3)*32.0f;


    Vector2 position = { (x-32.0f)/2.0f,(y-32.0f)/2.0f };
    Rectangle frameRec = { frameX, frameY, 32.0f, 32.0f };
    DrawTextureRec(SprSheet, frameRec, position, WHITE);  // Draw part of the texture
    rlPopMatrix();

}

void DrawSpriteRot(float x,float y,int sprnum,float rot)
{
// we can get rid of 2.0 scale when everything is normalized correctly...
// vectors are currently rendered with 0.5f scale
    rlPushMatrix();
    rlScalef(2.0f, 2.0f, 1.0f);

    float frameX = (sprnum&7)*32.0f;
    float frameY = (sprnum>>3)*32.0f;


    Vector2 org = {16.0f,16.0f};
    Rectangle frameRec = { frameX, frameY, 32.0f, 32.0f };
    Rectangle destRec = { x/2.0f,y/2.0f, 32.0f, 32.0f };
    DrawTexturePro(SprSheet,frameRec,destRec, org, rot,WHITE);
    rlPopMatrix();

}


// 48x48
void DrawTetherSprite(float x1,float y1,float x2,float y2)
{
// we can get rid of 2.0 scale when everything is normalized correctly...
// vectors are currently rendered with 0.5f scale
    rlPushMatrix();
    rlScalef(2.0f, 2.0f, 1.0f);

    float frameX = 0; //(sprnum&7)*32.0f;
    float frameY = 128;   //(sprnum>>3)*32.0f;


    float dx = x2-x1;
    float dy = y2-y1;
    float rot = atan2f(dy,dx);
    rot *= (180.0f/PI); // DEGTORAD
    rot+=90.0f;

    float x = x1+((x2-x1)/2.0f)+2.0f;       // +2.0f fixes centre?
    float y = y1+((y2-y1)/2.0f)+2.0f;



    Vector2 org = {24.0f,24.0f};
    Rectangle frameRec = { frameX, frameY, 48.0f, 48.0f };
    Rectangle destRec = { x/2.0f,y/2.0f, 48.0f, 48.0f };
    DrawTexturePro(SprSheet,frameRec,destRec, org, rot,WHITE);
    rlPopMatrix();

}