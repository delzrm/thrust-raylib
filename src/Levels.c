#include "Levels.h"
#include "Colours.h"
#include "Sprites.h"
#include <stdbool.h>

#define TWOK 1920   //2000

LevelDef gLevels[NUM_LEVELS] = {
// ======= LEVEL 1 =======
{
    .arenaW=TWOK,.arenaH=2500,.gravity=0.10f,
    .landscape={{0,1500},{514,1500},{674,1575},{874,1575},{1004,1635},
                {1255,1635},{1255,1545},{1395,1545},{1495,1500},{TWOK,1500}},
    .lsCount=10,
    .starCount=50,.starLowerLimit=1300,
    .reactorX=1302,.reactorY=1525.5f,
    .shipX=805,.shipY=1350,
    .podX=1128,.podY=1603,
    .enemies={{946,1594,205,180,0,0.02f,SPRDEF_TURRET_NE}}, .enemyCount=1,
    .tanks={{805,1558}}, .tankCount=1,
    .doorCount=0,
    .endColorTop="#ff0000",.endColorMid="#00ff00",.endColorBot="#ffff00",
    .restartCount=0,
},
// ======= LEVEL 2 =======
{
    .arenaW=TWOK,.arenaH=2500,.gravity=0.11f,
    .landscape={{0,1500},{574,1500},{678,1552},{878,1552},{1054,1640},{1054,1860},
                {870,1952},{870,2033},{998,2097},{1143,2097},{1343,1996},{1343,1912},
                {1207,1844},{1207,1609},{1425,1500},{TWOK,1500}},
    .lsCount=16,
    .starCount=50,.starLowerLimit=1300,
    .reactorX=820,.reactorY=1532.5f,
    .shipX=910,.shipY=1360,
    .podX=1024,.podY=2064.5f,
    .enemies={{1266,1888,26.5f,200,-18,0.02f,SPRDEF_TURRET_SW},{960,1920,333,190,0,0.02f,SPRDEF_TURRET_SE}}, .enemyCount=2,
    .tanks={{1118,2077}}, .tankCount=1,
    .doorCount=0,
    .endColorTop="#00ff00",.endColorMid="#ff0000",.endColorBot="#ffff00",
    .restartCount=0,
},
// ======= LEVEL 3 =======
{
    .arenaW=TWOK,.arenaH=3000,.gravity=0.13f,
    .landscape={{0,1500},{880,1500},{880,1821},{800,1860},{800,2061},
                {566,2061},{480,2104},{480,2225},{366,2225},{280,2268},
                {280,2609},{369,2652},{529,2652},{529,2432},{601,2396},
                {761,2396},{761,2192},{1000,2192},{1000,1948},{1155,1948},
                {1241,1905},{1241,1821},{1049,1821},{1049,1580},{1233,1580},
                {1233,1500},{TWOK,1500}},
    .lsCount=27,
    .starCount=50,.starLowerLimit=1300,
    .reactorX=1140,.reactorY=1560.5f,
    .shipX=720,.shipY=1360,
    .podX=460,.podY=2620.5f,
    .enemies={
        {846,1852,333.5f,180,10,0.025f,SPRDEF_TURRET_SE},{1193,1915,153,100,50,0.025f,SPRDEF_TURRET_NW},
        {527,2093,333.5f,180,10,0.025f,SPRDEF_TURRET_SE},{327,2258,333.5f,180,10,0.025f,SPRDEF_TURRET_SE},
        {557,2405,153,100,50,0.025f,SPRDEF_TURRET_NW}}, .enemyCount=5,
    .tanks={{790,1480},{1025,1930},{1075,1930},{1125,1930},{820,2174},{635,2376}},
    .tankCount=6,
    .doorCount=0,
    .endColorTop="#00ffff",.endColorMid="#00ff00",.endColorBot="#ffff00",
    .restartCount=3,
    .restartYPos={1950,2240,1950},
    .restartShipX={905,410,905},.restartShipY={1950,2410,1950},
    .restartHasPod={false,false,true},
},
// ======= LEVEL 4 =======
{
    .arenaW=TWOK,.arenaH=3200,.gravity=0.14f,
    .landscape={{0,1500},{519,1500},{680,1580},{808,1580},{808,1656},
                {504,1816},{504,1896},{584,1935},{584,1961},{424,2040},
                {424,2180},{624,2180},{792,2264},{1049,2264},{1049,2417},
                {824,2528},{824,2672},{904,2712},{904,2796},{1001,2796},
                {1001,2712},{1193,2616},{1193,2085},{873,2085},{681,1989},
                {681,1916},{921,1916},{921,1500},{TWOK,1500}},
    .lsCount=29,
    .starCount=50,.starLowerLimit=1300,
    .reactorX=546,.reactorY=2160.5f,
    .shipX=720,.shipY=1360,
    .podX=955,.podY=2763.5f,
    .enemies={
        {738,1706,331.5f,180,10,0.025f,SPRDEF_TURRET_SE},{543,1902,206,180,10,0.025f,SPRDEF_TURRET_NE},
        {544,1993,333.5f,180,10,0.025f,SPRDEF_TURRET_SE},{693,2200,206,180,10,0.025f,SPRDEF_TURRET_NE},
        {777,2051,26,180,10,0.025f,SPRDEF_TURRET_SW},{930,2490,333.5f,180,10,0.025f,SPRDEF_TURRET_SE},
        {1111,2643,153.5f,180,10,0.025f,SPRDEF_TURRET_NW}}, .enemyCount=7,
    .tanks={0}, .tankCount=0,
    .doors={{
        .x=1043,.y=2308,
        .stateCount=11,
        .verts={{{0,0},{164,0},{164,50},{0,50}},
                {{0,0},{149,0},{149,50},{0,50}},
                {{0,0},{134,0},{134,50},{0,50}},
                {{0,0},{119,0},{119,50},{0,50}},
                {{0,0},{104,0},{104,50},{0,50}},
                {{0,0},{89,0},{89,50},{0,50}},
                {{0,0},{74,0},{74,50},{0,50}},
                {{0,0},{59,0},{59,50},{0,50}},
                {{0,0},{44,0},{44,50},{0,50}},
                {{0,0},{29,0},{29,50},{0,50}},
                {{0,0},{24,0},{24,50},{0,50}}},
        .vertCount={4,4,4,4,4,4,4,4,4,4,4},
        .keyholes={{1192,2227},{1192,2443}}, .keyholeCount=2,
    }},
    .doorCount=1,
    .endColorTop="#00ff00",.endColorMid="#ff00ff",.endColorBot="#ffff00",
    .restartCount=3,
    .restartYPos={1770,2200,1770},
    .restartShipX={815,1115,815},.restartShipY={1770,2160,1770},
    .restartHasPod={false,false,true},
},
// ======= LEVEL 5 =======
{
    .arenaW=TWOK,.arenaH=3900,.gravity=0.15f,
    .landscape={{0,1500},{431,1500},{599,1584},{599,1676},{783,1676},
                {783,2001},{710,2001},{607,2052},{607,2168},{687,2168},
                {687,2329},{527,2409},{367,2409},{367,2757},{527,2836},
                {527,2896},{447,2896},{447,3009},{550,3059},{687,3059},
                {687,3182},{789,3227},{943,3227},{943,3561},{1016,3596},
                {1080,3596},{1184,3546},{1184,3424},{1056,3361},{1056,3109},
                {928,3109},{928,3020},{842,2977},{768,2977},{768,2800},
                {512,2673},{512,2536},{778,2536},{864,2492},{864,2329},
                {800,2329},{800,2168},{1087,2168},{1087,2044},{1001,2001},
                {896,2001},{896,1500},{TWOK,1500}},
    .lsCount=48,
    .starCount=50,.starLowerLimit=1300,
    .reactorX=891,.reactorY=2148.5f,
    .shipX=625,.shipY=1402,
    .podX=1043,.podY=3563.5f,
    .enemies={
        {664,2037,333.5f,180,10,0.025f,SPRDEF_TURRET_SE},{1039,2034,26.5f,180,10,0.025f,SPRDEF_TURRET_SW},
        {816,2503,153.5f,180,10,0.025f,SPRDEF_TURRET_NW},{494,3018,205.5f,180,10,0.025f,SPRDEF_TURRET_NE},
        {878,3009,26,180,10,0.025f,SPRDEF_TURRET_SW},{736,3190,203.5f,180,10,0.025f,SPRDEF_TURRET_NE},
        {1119,3406,26,180,10,0.025f,SPRDEF_TURRET_SW}}, .enemyCount=7,
    .tanks={{735,1657},{976,2149},{1027,2149},{576,2517},{583,3040},
            {635,3040},{839,3207},{890,3207}}, .tankCount=8,
    .doors={{
        .x=930,.y=3346,
        .stateCount=10,
        .verts={{{0,0},{140,0},{140,-82},{0,-82}},
                {{0,0},{140,0},{140,-72},{0,-72}},
                {{0,0},{140,0},{140,-62},{0,-62}},
                {{0,0},{140,0},{140,-52},{0,-52}},
                {{0,0},{140,0},{140,-42},{0,-42}},
                {{0,0},{140,0},{140,-32},{0,-32}},
                {{0,0},{140,0},{140,-22},{0,-22}},
                {{0,0},{140,0},{140,-12},{0,-12}},
                {{0,0},{140,0},{140,-2},{0,-2}},
                {{0,0},{1,0},{1,-2},{0,-2}}},
        .vertCount={4,4,4,4,4,4,4,4,4,4},
        .keyholes={{1056,3156},{943,3474}}, .keyholeCount=2,
    }},
    .doorCount=1,
    .endColorTop="#ff0000",.endColorMid="#ff00ff",.endColorBot="#ffff00",
    .restartCount=5,
    .restartYPos={2370,2840,3095,2840,2370},
    .restartShipX={740,625,795,625,740},.restartShipY={2370,2840,3095,2840,2370},
    .restartHasPod={false,false,false,true,true},
},
// ======= LEVEL 6 =======
{
    .arenaW=TWOK,.arenaH=4300,.gravity=0.16f,
    .landscape={{0,1500},{511,1500},{511,1752},{695,1752},{1334,2072},
                {1334,2232},{1181,2232},{1094,2276},{1094,2925},{958,2925},
                {958,3145},{854,3196},{854,3277},{1286,3492},{1286,3549},
                {1182,3600},{1182,3725},{1102,3764},{1102,4000},{1190,4000},
                {1190,4123},{1311,4123},{1311,4029},{1207,4029},{1207,3884},
                {1431,3773},{1431,3549},{1489,3520},{1431,3492},{1431,3372},
                {1079,3179},{1079,3040},{1247,3040},{1247,2836},{1327,2797},
                {1327,2716},{1183,2645},{1183,2508},{1407,2396},{1607,2396},
                {1607,2312},{1447,2233},{1447,1972},{1008,1753},{1008,1672},
                {1352,1500},{TWOK,1500}},
    .lsCount=47,
    .starCount=50,.starLowerLimit=1300,
    .reactorX=1264,.reactorY=4103.5f,
    .shipX=792,.shipY=1555,
    .podX=1146,.podY=3967.5f,
    .enemies={
        {1136,1831,26.5f,180,0,0.025f,SPRDEF_TURRET_SW},{1525,2285,26.5f,180,0,0.025f,SPRDEF_TURRET_SW},
        {1286,2443,153.5f,180,0,0.025f,SPRDEF_TURRET_NW},{1141,2265,333.5f,180,0,0.025f,SPRDEF_TURRET_SE},
        {1285,2709,26.5f,180,0,0.025f,SPRDEF_TURRET_SW},{1285,2804,153.5f,180,0,0.025f,SPRDEF_TURRET_NW},
        {906,3184,333.5f,180,0,0.025f,SPRDEF_TURRET_SE},{1153,3234,28,180,0,0.025f,SPRDEF_TURRET_SW},
        {1224,3593,333.5f,180,0,0.025f,SPRDEF_TURRET_SE},{1152,3753,333.5f,180,0,0.025f,SPRDEF_TURRET_SE},
        {1310,3820,153.5f,180,0,0.025f,SPRDEF_TURRET_NW}}, .enemyCount=11,
    .tanks={{1454,2376},{1143,3021}}, .tankCount=2,
    .doors={{
        .x=1280,.y=3492,
        .stateCount=12,
        .verts={{{0,0},{171,0},{225,28},{171,57},{0,57}},
                {{0,0},{156,0},{210,28},{156,57},{0,57}},
                {{0,0},{141,0},{195,28},{141,57},{0,57}},
                {{0,0},{126,0},{180,28},{126,57},{0,57}},
                {{0,0},{111,0},{165,28},{111,57},{0,57}},
                {{0,0},{96,0},{150,28},{96,57},{0,57}},
                {{0,0},{81,0},{135,28},{81,57},{0,57}},
                {{0,0},{66,0},{120,28},{66,57},{0,57}},
                {{0,0},{51,0},{105,28},{51,57},{0,57}},
                {{0,0},{36,0},{90,28},{36,57},{0,57}},
                {{0,0},{20,0},{75,28},{20,57},{0,57}},
                {{0,0},{6,0},{60,28},{6,57},{0,57}}},
        .vertCount={5,5,5,5,5,5,5,5,5,5,5,5},
        .keyholes={{1430,3432},{1183,3667}}, .keyholeCount=2,
    }},
    .doorCount=1,
    .endColorTop="#ff0000",.endColorMid="#00ffff",.endColorBot="#ffff00",
    .restartCount=7,
    .restartYPos={2300,2850,3175,3630,3175,2850,2300},
    .restartShipX={1230,1160,1010,1320,1010,1160,1230},
    .restartShipY={2300,2850,3175,3630,3175,2850,2300},
    .restartHasPod={false,false,false,false,true,true,true},
},
};

void InitLevelColors(LevelDef *lv, int levelIdx) {
    switch (levelIdx) {
    case 0: // Level 1
        lv->landscapeColor=LEVEL1COLOR; lv->starColorA=C_YELLOW; lv->starColorB=C_GREEN;
        lv->reactorColor=C_YELLOW; lv->reactorChimney=C_GREEN; lv->reactorDoor=C_RED; lv->reactorExplosion=C_YELLOW;
        lv->shipBulletColor=C_GREEN; lv->refuelColor=C_YELLOW; lv->shieldColor=C_GREEN; lv->shipExplosion=C_GREEN;
        lv->podColor=C_GREEN; lv->podBaseColor=C_YELLOW; lv->rodColor=C_RED;
        lv->enemyColor=C_GREEN; lv->enemyBulletColor=C_RED; lv->enemyExplosion=C_YELLOW;
        lv->tankColor=C_YELLOW; lv->tankLabel=C_RED; lv->tankExplosion=C_GREEN;
        break;
    case 1: // Level 2
        lv->landscapeColor=LEVEL2COLOR; lv->starColorA=C_YELLOW; lv->starColorB=C_RED;
        lv->reactorColor=C_YELLOW; lv->reactorChimney=C_RED; lv->reactorDoor=C_GREEN; lv->reactorExplosion=C_YELLOW;
        lv->shipBulletColor=C_RED; lv->refuelColor=C_YELLOW; lv->shieldColor=C_RED; lv->shipExplosion=C_RED;
        lv->podColor=C_RED; lv->podBaseColor=C_YELLOW; lv->rodColor=C_GREEN;
        lv->enemyColor=C_RED; lv->enemyBulletColor=C_GREEN; lv->enemyExplosion=C_YELLOW;
        lv->tankColor=C_YELLOW; lv->tankLabel=C_GREEN; lv->tankExplosion=C_RED;
        break;
    case 2: // Level 3
        lv->landscapeColor=LEVEL3COLOR; lv->starColorA=C_YELLOW; lv->starColorB=C_RED;
        lv->reactorColor=C_YELLOW; lv->reactorChimney=C_GREEN; lv->reactorDoor=C_CYAN; lv->reactorExplosion=C_GREEN;
        lv->shipBulletColor=C_GREEN; lv->refuelColor=C_YELLOW; lv->shieldColor=C_GREEN; lv->shipExplosion=C_CYAN;
        lv->podColor=C_GREEN; lv->podBaseColor=C_YELLOW; lv->rodColor=C_CYAN;
        lv->enemyColor=C_GREEN; lv->enemyBulletColor=C_CYAN; lv->enemyExplosion=C_YELLOW;
        lv->tankColor=C_YELLOW; lv->tankLabel=C_CYAN; lv->tankExplosion=C_RED;
        break;
    case 3: // Level 4
        lv->landscapeColor=LEVEL4COLOR; lv->starColorA=C_YELLOW; lv->starColorB=C_RED;
        lv->reactorColor=C_YELLOW; lv->reactorChimney=C_MAGENTA; lv->reactorDoor=C_GREEN; lv->reactorExplosion=C_YELLOW;
        lv->shipBulletColor=C_MAGENTA; lv->refuelColor=C_YELLOW; lv->shieldColor=C_MAGENTA; lv->shipExplosion=C_RED;
        lv->podColor=C_MAGENTA; lv->podBaseColor=C_YELLOW; lv->rodColor=C_GREEN;
        lv->enemyColor=C_MAGENTA; lv->enemyBulletColor=C_GREEN; lv->enemyExplosion=C_YELLOW;
        lv->tankColor=C_YELLOW; lv->tankLabel=C_GREEN; lv->tankExplosion=C_RED;
        lv->doors[0].doorColor=LEVEL4COLOR; lv->doors[0].keyColor=C_YELLOW;
        break;
    case 4: // Level 5
        lv->landscapeColor=LEVEL5COLOR; lv->starColorA=C_YELLOW; lv->starColorB=C_RED;
        lv->reactorColor=C_YELLOW; lv->reactorChimney=C_MAGENTA; lv->reactorDoor=C_RED; lv->reactorExplosion=C_YELLOW;
        lv->shipBulletColor=C_MAGENTA; lv->refuelColor=C_YELLOW; lv->shieldColor=C_MAGENTA; lv->shipExplosion=C_MAGENTA;
        lv->podColor=C_MAGENTA; lv->podBaseColor=C_YELLOW; lv->rodColor=C_RED;
        lv->enemyColor=C_MAGENTA; lv->enemyBulletColor=C_RED; lv->enemyExplosion=C_GREEN;
        lv->tankColor=C_YELLOW; lv->tankLabel=C_RED; lv->tankExplosion=C_RED;
        lv->doors[0].doorColor=LEVEL5COLOR; lv->doors[0].keyColor=C_YELLOW;
        break;
    case 5: // Level 6
        lv->landscapeColor=LEVEL6COLOR; lv->starColorA=C_YELLOW; lv->starColorB=C_RED;
        lv->reactorColor=C_YELLOW; lv->reactorChimney=C_CYAN; lv->reactorDoor=C_MAGENTA; lv->reactorExplosion=C_YELLOW;
        lv->shipBulletColor=C_CYAN; lv->refuelColor=C_YELLOW; lv->shieldColor=C_CYAN; lv->shipExplosion=C_CYAN;
        lv->podColor=C_CYAN; lv->podBaseColor=C_YELLOW; lv->rodColor=C_MAGENTA;
        lv->enemyColor=C_CYAN; lv->enemyBulletColor=C_MAGENTA; lv->enemyExplosion=C_YELLOW;
        lv->tankColor=C_YELLOW; lv->tankLabel=C_MAGENTA; lv->tankExplosion=C_RED;
        lv->doors[0].doorColor=LEVEL6COLOR; lv->doors[0].keyColor=C_YELLOW;
        break;
    }
}
