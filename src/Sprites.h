#pragma once
void    LoadSprites(void);
void    FreeSprites(void);
void DrawSprite(float x,float y,int sprnum);
void DrawSpriteRot(float x,float y,int sprnum,float rot);
void DrawTetherSprite(float x1,float y1,float x2,float y2);


#define SPRDEF_BLANK        0
#define SPRDEF_PODWITHBASE  18
#define SPRDEF_POD          19
#define SPRDEF_FUELTANK     8
#define SPRDEF_REACTOR      2
#define SPRDEF_SHIP         10

#define SPRDEF_TURRET_NE    1
#define SPRDEF_TURRET_NW    9
#define SPRDEF_TURRET_SW    17
#define SPRDEF_TURRET_SE    25