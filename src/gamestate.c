#include "gamestate.h"
#include <ace/managers/memory.h>

tGameStats* g_pGameStats = NULL;

tTileMap g_pTownMap;
tPlayer *g_pMainPlayer = NULL;
tWanderer *g_pWanderers[2] = {NULL, NULL};
tGhost *g_pGhosts[4] = {NULL, NULL, NULL, NULL};

tGhost *g_pMarshGhosts[4] = {NULL, NULL, NULL, NULL};
tGhost *g_pDriveGhost = NULL;
tGhost *g_pCaptureGhost = NULL;

tMarshMan *g_pMarshMellowMan = NULL;

tView *g_pView = NULL;
tVPort *g_pVpMain = NULL;
tVPort *g_pVpHUD = NULL;
tTileBufferManager *g_pMainBuffer = NULL;
tSimpleBufferManager *g_pHudBuffer = NULL;
UWORD s_pPaletteRef[32];

UBYTE g_bBuildings[40] = {
						BT_1S, BT_2S, BT_3S, BT_1S, BT_2S, BT_SP, BT_2S, BT_1S, 
						BT_3S, BT_1S, BT_2S, BT_3S, BT_1S, BT_2S, BT_3S, BT_1S,
						BT_1S, BT_3S, BT_3S, BT_1S, BT_ZL, BT_3S, BT_2S, BT_1S,
						BT_2S, BT_2S, BT_1S, BT_2S, BT_3S, BT_1S, BT_3S, BT_2S,
						BT_1S, BT_2S, BT_HQ, BT_1S, BT_2S, BT_3S, BT_2S, BT_1S};

void ResetGameStats(void)
{
    // New Game, reset my stats.
    if (g_pGameStats == NULL)
    {
        g_pGameStats = memAllocFastClear(sizeof(tGameStats));
    }

    g_pGameStats->_buildings = g_bBuildings;
    g_pGameStats->_cityPKEnergy = 0;
    g_pGameStats->_men = 3;
    g_pGameStats->_money = 10000;
    g_pGameStats->_traps =0;
    g_pGameStats->_carType = 0x00;
    g_pGameStats->_items = 0x00;
    
}