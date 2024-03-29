#include "driving.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/state.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/blit.h>
#include <ace/utils/palette.h>
#include <ace/managers/rand.h>
#include <ace/managers/viewport/tilebuffer.h>
#include "maps/city16x.h"
#include "bob_new.h"
#include "assets.h"
#include "gamestate.h"
#include "ghostbusters.h"
#include "fade.h"
typedef void (*tCbLogo)(void);
typedef UBYTE (*tCbFadeOut)(void);

void driveFadeIn(void);
void driveWait(void);
UBYTE driveFadeOut(void);

static UBYTE s_ubFrame = 0;
static tFadeState s_eFadeState;

static UBYTE s_ubFadeoutCnt;
static tCbLogo s_cbFadeIn = 0, s_cbWait = 0;
static tCbFadeOut s_cbFadeOut = 0;
static UBYTE s_isAnyPressed = 0;

static BYTE s_bSpeedMod = 0;


void driveHandleInput(BYTE *bDirX, BYTE *bDirY)
{
	if (keyCheck(KEY_D))
	{
		*bDirX += 1;
	}
	if (keyCheck(KEY_A))
	{
		*bDirX -= 1;
	}

	if (joyCheck(JOY1_RIGHT))
	{
		*bDirX += 1;
	}
	if (joyCheck(JOY1_LEFT))
	{
		*bDirX -= 1;
	}
}


static void driveGsCreate(void)
{
	s_ubFadeoutCnt = 0;
	s_eFadeState = FADE_STATE_IN;

	s_cbFadeIn = driveFadeIn;
	s_cbFadeOut = driveFadeOut;
	s_cbWait = driveWait;
	bobNewDiscardUndraw();
	UBYTE **pTiles = g_pMainBuffer->pTileData;
	//int y=0;
	for (int y = 0; y < 24; y++)
	{
		for (int x = 0; x < g_pTownMap._width; x++)
		{
			if (x < 2 || x > 17)
			{
				pTiles[x][y] = 243; // black
			}
			else
			{
				pTiles[x][y] = 117 + ROAD_layer[(y)*16 + x-2]; // Road plus offset into tilemap.
			}
		}
	}

	g_pMainPlayer->_locMapX = 140;
	g_pMainPlayer->_locMapY = 120;
	

//	g_pMainBuffer->pCamera->uPos.uwY -=2;
	
	tileBufferRedrawAll(g_pMainBuffer);
}

static void driveGsLoop(void)
{
bobNewBegin();
	s_isAnyPressed = (keyUse(KEY_RETURN) | keyUse(KEY_ESCAPE) | keyUse(KEY_SPACE) |
					  keyUse(KEY_LSHIFT) | keyUse(KEY_RSHIFT) |
					  joyUse(JOY1 + JOY_FIRE) | joyUse(JOY2 + JOY_FIRE));
	if (s_eFadeState == FADE_STATE_IN) 
	{
		if (s_ubFadeoutCnt >= 20)
		{
			s_eFadeState = FADE_STATE_IDLE;
			s_ubFrame = 0;
		}
		else
		{
			if (s_cbFadeIn)
			{
				s_cbFadeIn();
			}
			++s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, g_pVpMain->pPalette, 32, (15 * s_ubFadeoutCnt) / 20);
		}
	}
	else if (s_eFadeState == FADE_STATE_IDLE)
	{
		if (s_cbWait)
		{
			s_cbWait();
		}

		
	}
	else if (s_eFadeState == FADE_STATE_OUT)
	{
		if (s_ubFadeoutCnt == 0)
		{
			if (s_cbFadeOut && s_cbFadeOut())
			{
				return;
			}
			else
			{
				s_eFadeState = FADE_STATE_IN;
			}
		}
		else
		{
			--s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, g_pVpMain->pPalette, 32, 15 * s_ubFadeoutCnt / 20);
		}
	}

	BYTE bDirX = 0, bDirY = 0;

	driveHandleInput(&bDirX, &bDirY);
	g_pMainPlayer->_locMapY-=s_bSpeedMod;
	
	g_pMainPlayer->_locMapX += (bDirX * 2);

	if (g_pMainPlayer->_locMapX < 20)
		g_pMainPlayer->_locMapX = 20;
	
	if (g_pMainPlayer->_locMapX > 220)
		g_pMainPlayer->_locMapX = 220;
	
	bobNewPush(&g_pMainPlayer->_bobCarDrive);

	vPortWaitForEnd(g_pVpMain);
	viewUpdateCLUT(g_pView);
		// Finish bob drawing
	
	bobNewPushingDone();
	bobNewEnd();

	// Update HUD state machine and draw stuff
	//hudUpdate();

	viewProcessManagers(g_pView);
	copProcessBlocks();
	vPortWaitForEnd(g_pVpMain);

	g_pMainPlayer->_bobCarDrive.sPos.uwX = g_pMainPlayer->_locMapX;
	g_pMainPlayer->_bobCarDrive.sPos.uwY = g_pMainPlayer->_locMapY;

	g_pMainBuffer->pCamera->uPos.uwY -=s_bSpeedMod;

	s_bSpeedMod +=1;
	if (s_bSpeedMod >8)
	{
		s_bSpeedMod = 8;
	}
}

static void driveGsDestroy(void)
{
	bobNewDiscardUndraw();
	g_pMainBuffer->pCamera->uPos.uwY=0;
}

void driveFadeIn(void)
{
}

void driveWait(void)
{
	++s_ubFrame;
	if (s_eFadeState == FADE_STATE_IDLE)
	{

		if (s_isAnyPressed)
		{
			s_ubFadeoutCnt = 20;
			s_eFadeState = FADE_STATE_OUT;
		}
	}
	// else if(s_ubFrame == 1){
	// 	//ptplayerSfxPlay(s_pSfxLmc, -1, PTPLAYER_VOLUME_MAX, 1);
	// 	// s_eFadeState = FADE_STATE_OUT; // FOR DEBUGGING SFX GLITCHES
	// }


}

UBYTE driveFadeOut(void)
{
	//ptplayerWaitForSfx();
	//ptplayerSfxDestroy(s_pSfxdrive);

	statePop(g_pStateMachineGame);

	return 1;
}

tState g_sStateGameDrive = {
	.cbCreate = driveGsCreate, .cbLoop = driveGsLoop, .cbDestroy = driveGsDestroy};
