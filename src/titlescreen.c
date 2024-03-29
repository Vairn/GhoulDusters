/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "loading.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/blit.h>
#include <ace/utils/palette.h>

//#include "menu.h"
#include "fade.h"
#include "ghostbusters.h"

typedef void (*tCbLogo)(void);
typedef UBYTE (*tCbFadeOut)(void);

void titleFadeIn(void);
void titleWait(void);
UBYTE titleFadeOut(void);

static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pBfr;
static UBYTE s_ubFrame = 0;
static tFadeState s_eFadeState;

static UWORD s_pPaletteRef[32];
static UBYTE s_ubFadeoutCnt;
static tCbLogo s_cbFadeIn = 0, s_cbWait = 0;
static tCbFadeOut s_cbFadeOut = 0;
static UBYTE s_isAnyPressed = 0;

static void menuGsCreate(void) {
	logBlockBegin("menuGsCreate()");

	s_pView = viewCreate(0,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_END);

	s_pVp = vPortCreate(0,
		TAG_VPORT_BPP, 5,
		TAG_VPORT_VIEW, s_pView,
	TAG_END);

	s_pBfr = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_VPORT, s_pVp,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 0,
		TAG_SIMPLEBUFFER_USE_X_SCROLLING, 0,
	TAG_END);

	s_ubFadeoutCnt = 0;
	s_eFadeState = FADE_STATE_IN;

	s_cbFadeIn = titleFadeIn;
	s_cbFadeOut = titleFadeOut;
	s_cbWait = titleWait;

	logBlockEnd("menuGsCreate() - tah");
	systemUnuse();
	viewLoad(s_pView);
   
}

static void menuGsLoop(void) {
	s_isAnyPressed = (
		keyUse(KEY_RETURN) | keyUse(KEY_ESCAPE) | keyUse(KEY_SPACE) |
		keyUse(KEY_LSHIFT) | keyUse(KEY_RSHIFT) |
		joyUse(JOY1 + JOY_FIRE) | joyUse(JOY2 + JOY_FIRE)
	);
	if(s_eFadeState == FADE_STATE_IN) {
		if(s_ubFadeoutCnt >= 50) {
			s_eFadeState = FADE_STATE_IDLE;
			s_ubFrame = 0;
		}
		else {
			if(s_cbFadeIn) {
				s_cbFadeIn();
			}
			++s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, s_pVp->pPalette, 32, (15 * s_ubFadeoutCnt) / 50);
		}
	}
	else if(s_eFadeState == FADE_STATE_IDLE) {
		if(s_cbWait) {
			s_cbWait();
		}
	}
	else if(s_eFadeState == FADE_STATE_OUT) {
		if(s_ubFadeoutCnt == 0) {
			if(s_cbFadeOut && s_cbFadeOut()) {
				return;
			}
			else {
				s_eFadeState = FADE_STATE_IN;
			}
		}
		else {
			--s_ubFadeoutCnt;
			paletteDim(s_pPaletteRef, s_pVp->pPalette, 32, 15*s_ubFadeoutCnt/50);
		}
	}

	vPortWaitForEnd(s_pVp);
	viewUpdateCLUT(s_pView);
}

static void menuGsDestroy(void) {
	systemUse();
	logBlockBegin("menuGsDestroy()");
	viewDestroy(s_pView);
	logBlockEnd("menuGsDestroy()");
}

//-------------------------------------------------------------------------- LMC

#define LOGO_WIDTH 320
#define LOGO_HEIGHT 200

void titleFadeIn(void) {
	if(s_ubFadeoutCnt == 0) {
		systemUse();
		paletteLoad("data/GB-Modern.plt", s_pPaletteRef, 1 << s_pVp->ubBPP);
		tBitMap *pLogo = bitmapCreateFromFile("data/GB-Modern.bm", 0);
		//s_pSfxLmc = ptplayerSfxCreateFromFile("data/sfx/lmc.sfx");
		systemUnuse();

		blitCopy(
			pLogo, 0, 0, s_pBfr->pBack,
			(s_pVp->uwWidth - LOGO_WIDTH) / 2, (s_pVp->uwHeight - LOGO_HEIGHT) / 2,
			LOGO_WIDTH, LOGO_HEIGHT, MINTERM_COOKIE
		);

		systemUse();
		bitmapDestroy(pLogo);
		systemUnuse();
	}
	else if(s_isAnyPressed) {
		s_eFadeState = FADE_STATE_OUT;
	}
}

void titleWait(void) {
	++s_ubFrame;
	if( s_isAnyPressed) {
		s_eFadeState = FADE_STATE_OUT;
	}
	else if(s_ubFrame == 1){
		//ptplayerSfxPlay(s_pSfxLmc, -1, PTPLAYER_VOLUME_MAX, 1);
		// s_eFadeState = FADE_STATE_OUT; // FOR DEBUGGING SFX GLITCHES
	}
}

UBYTE titleFadeOut(void) {
	//ptplayerWaitForSfx();
	//ptplayerSfxDestroy(s_pSfxLmc);
	stateChange(g_pStateMachineGame, &g_sStateGameTown);
	return 1;
}

tState g_sStateMenu = {
	.cbCreate = menuGsCreate, .cbLoop = menuGsLoop, .cbDestroy = menuGsDestroy
};
