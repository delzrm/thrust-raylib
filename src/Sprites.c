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
Texture2D ShipSprSheet;
Texture2D LineSprSheet;

void    LoadSprites(void)
{
    SprSheet = LoadTexture("sprsheet.png"); 
    LineSprSheet = LoadTexture("linesheet2.png"); 
    ShipSprSheet = LoadTexture("shipsheet2.png");
}
void    FreeSprites(void)
{
    UnloadTexture(SprSheet);       // Texture unloading
    UnloadTexture(LineSprSheet);       // Texture unloading
    UnloadTexture(ShipSprSheet);       // Texture unloading
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

    //if (rot>180.0f)
    //    rot-=180.0f;
    //if (rot<-180.0f)
    //    rot+=180.0f;
    

    float x = x1+((x2-x1)/2.0f)+2.0f;       // +2.0f fixes centre?
    float y = y1+((y2-y1)/2.0f)+2.0f;



    Vector2 org = {24.0f,24.0f};
    Rectangle frameRec = { frameX, frameY, 48.0f, 48.0f };
    Rectangle destRec = { x/2.0f,y/2.0f, 48.0f, 48.0f };
    DrawTexturePro(SprSheet,frameRec,destRec, org, rot,WHITE);
    rlPopMatrix();

}
float wrap180(float angle);
void DrawTetherSprite4040(float x1,float y1,float x2,float y2)
{
// we can get rid of 2.0 scale when everything is normalized correctly...
// vectors are currently rendered with 0.5f scale
    rlPushMatrix();
    rlScalef(2.0f, 2.0f, 1.0f);

    float frameX = 0; //(sprnum&7)*32.0f;
    float frameY = 0;   //(sprnum>>3)*32.0f;


    float dx = x2-x1;
    float dy = y2-y1;
    float rot = atan2f(dy,dx);
    rot *= (180.0f/PI); // DEGTORAD
    rot+=90.0f;

    //if (rot>180.0f)
    //    rot-=180.0f;
    //if (rot<-180.0f)
    //    rot+=180.0f;
    
    float x = x1+((x2-x1)/2.0f)+2.0f;
    float y = y1+((y2-y1)/2.0f)+2.0f;

    // test sprite frames...
    int def = ((int)wrap180(rot))/2;
    frameX = (def&7)*40.0f;
    frameY = (def>>3)*40.0f;
    rot = 0.0f;



    Vector2 org = {20.0f,20.0f};
    Rectangle frameRec = { frameX, frameY, 40.0f, 40.0f };
    Rectangle destRec = { x/2.0f,y/2.0f, 40.0f, 40.0f };
    DrawTexturePro(LineSprSheet,frameRec,destRec, org, rot,WHITE);
    rlPopMatrix();

}
void DrawShipSprite3232(float x,float y,float rot)
{
// we can get rid of 2.0 scale when everything is normalized correctly...
// vectors are currently rendered with 0.5f scale
    rlPushMatrix();
    rlScalef(2.0f, 2.0f, 1.0f);

    // test sprite frames...
    if (rot>=360.0f)
        rot-=360.0f;
    if (rot<0.0f)
        rot+=360.0f;
    
    //int def = ((int)wrap180(rot))/2;
    int def = rot/5;
    float frameX = (def%10)*32.0f;
    float frameY = (def/10)*32.0f;
    rot = 0.0f;

    Vector2 org = {16.0f,16.0f};
    Rectangle frameRec = { frameX, frameY, 32.0f, 32.0f };
    Rectangle destRec = { x/2.0f,y/2.0f, 32.0f, 32.0f };
    DrawTexturePro(ShipSprSheet,frameRec,destRec, org, 0.0f,WHITE);
    rlPopMatrix();

}

float wrap180(float angle)
{
    return angle - 180.0f * floorf(angle / 180.0f);
}

void DrawTetherSpriteSheet(){
    float x,y;
    float rot = 0.0f;

    rlPushMatrix();
    rlScalef(1.0f, 1.0f, 1.0f);

    for (int i=0;i<90;i++,rot+=2.0f){

        x = 20.0f+((i&7)*40.f);
        y = 20.0f + ((i>>3)*40.0f);

        float frameX = 0; //(sprnum&7)*32.0f;
        float frameY = 176;   //(sprnum>>3)*32.0f;

        Vector2 org = {20.0f,20.0f};
        Rectangle frameRec = { frameX, frameY, 40.0f, 40.0f };
        Rectangle destRec = { x,y, 40.0f, 40.0f };
        DrawTexturePro(SprSheet,frameRec,destRec, org, rot,WHITE);
    }
    rlPopMatrix();
}

static float DTR(float deg) { return (deg - 90.0f) * DEG2RAD; }  // JS DegreesToRadians

void DrawShipSpriteSheet(){
    float x,y;
    float rot = 0.0f;

    rlPushMatrix();
    rlScalef(1.0f, 1.0f, 1.0f);

    for (int i=0;i<36*2;i++,rot+=5.0f){

        x = 16.0f+((i%10)*32.f);
        y = 16.0f + ((i/10)*32.0f);
        //DrawSpriteRot(x,y,11,rot);

        rlPushMatrix();
        rlScalef(1.0f, 1.0f, 1.0f);
        DrawShipMesh(x*2.0f, y*2.0f, DTR(rot), C_YELLOW, false);
        //int sprnum=11;
        //float frameX = (sprnum&7)*32.0f;
        //float frameY = (sprnum>>3)*32.0f;
        //Vector2 org = {16.0f,16.0f};
        //Rectangle frameRec = { frameX, frameY, 32.0f, 32.0f };
        //Rectangle destRec = { x,y, 32.0f, 32.0f };
        //DrawTexturePro(SprSheet,frameRec,destRec, org, rot,WHITE);
        rlPopMatrix();        



    }
    rlPopMatrix();
}



void DrawBulletSprite(float x,float y,int sprnum)
{
// we can get rid of 2.0 scale when everything is normalized correctly...
// vectors are currently rendered with 0.5f scale
    rlPushMatrix();
    rlScalef(2.0f, 2.0f, 1.0f);

    float frameX = 128.0f+(sprnum<<3);
    float frameY = 0.0f;


    Vector2 position = { (x-2.0f)/2.0f,(y-2.0f)/2.0f };
    Rectangle frameRec = { frameX, frameY, 8.0f, 8.0f };
    DrawTextureRec(SprSheet, frameRec, position, WHITE);  // Draw part of the texture
    rlPopMatrix();

}
