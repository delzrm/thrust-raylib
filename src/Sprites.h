#pragma once
void    LoadSprites(void);
void    FreeSprites(void);
void DrawSprite(float x,float y,int sprnum);
void DrawSpriteRot(float x,float y,int sprnum,float rot);
void DrawTetherSprite(float x1,float y1,float x2,float y2);
void DrawTetherSprite4040(float x1,float y1,float x2,float y2);
void DrawShipSprite3232(float x,float y,float rot);
void DrawBulletSprite(float x,float y,int sprnum);

void DrawTetherSpriteSheet();
void DrawShipSpriteSheet();

#define SPRDEF_BLANK        0
#define SPRDEF_PODWITHBASE  18
#define SPRDEF_POD          20//19
#define SPRDEF_FUELTANK     8
#define SPRDEF_REACTOR      2
#define SPRDEF_KEYHOLE      3
#define SPRDEF_SHIP         10
#define SPRDEF_SHIELD       12

#define SPRDEF_TURRET_NE    1
#define SPRDEF_TURRET_NW    9
#define SPRDEF_TURRET_SW    17
#define SPRDEF_TURRET_SE    25

#define SPRDEF_BULLETENEMY  0
#define SPRDEF_BULLETPLAYER 1
