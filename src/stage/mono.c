/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mono.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"

/*
#include "../pad.h"
int mog = -159;
int sog = -119;
int zog = 320;
int nog = 240;
*/
//mono background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Textures
	Gfx_Tex tex_ded; //imdead
	Gfx_Tex tex_scream; //ded paps

} Back_mono;

void Back_mono_DrawFG(StageBack *back)
{
	Back_mono *this = (Back_mono*)back;
    fixed_t fx, fy;

/*
	FntPrint("x%d y%d zog %d nog %d", mog, sog, zog, nog);

	if (pad_state.held & PAD_UP)
		sog --;
	if (pad_state.held & PAD_DOWN)
		sog ++;
	if (pad_state.held & PAD_LEFT)
		mog --;
	if (pad_state.held & PAD_RIGHT)
		mog ++;
	
	if (pad_state.held & PAD_TRIANGLE)
		zog --;
	if (pad_state.held & PAD_CROSS)
		zog ++;
	if (pad_state.held & PAD_SQUARE)
		nog --;
	if (pad_state.held & PAD_CIRCLE)
		nog ++;
*/

	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;

	RECT scream_src = {0, 0, 256, 256};
	RECT_FIXED scream_dst = {
		FIXED_DEC(-162,1),
		FIXED_DEC(-87,1),
		FIXED_DEC(307,1),
		FIXED_DEC(174,1)
	};
	
	if ((stage.song_step >= 504 && stage.song_step <= 511) || (stage.song_step >= 951 && stage.song_step <= 958) || (stage.song_step >= 964 && stage.song_step <= 966) || (stage.song_step >= 972 && stage.song_step <= 975) || (stage.song_step >= 980 && stage.song_step <= 982) || (stage.song_step >= 1117 && stage.song_step <= 1119) || (stage.song_step >= 1208 && stage.song_step <= 1214) || (stage.song_step >= 1536 && stage.song_step <= 1543))
		Stage_DrawTex(&this->tex_scream, &scream_src, &scream_dst, stage.camera.bzoom);

	RECT ded_src = {0, 0, 109, 93};
	RECT_FIXED ded_dst = {
		FIXED_DEC(-50,1),
		FIXED_DEC(-80,1),
		FIXED_DEC(109 - 16,1),
		FIXED_DEC(93 - 16,1)
	};

	RECT ded2_src = {0, 93, 256, 122};
	RECT_FIXED ded2_dst = {
		FIXED_DEC(-98,1),
		FIXED_DEC(-29,1),
		FIXED_DEC(256 - 64,1),
		FIXED_DEC(122 - 32,1)
	};

	if (stage.song_step >= 368 && stage.song_step <= 384)
	Stage_DrawTex(&this->tex_ded, &ded_src, &ded_dst, stage.camera.bzoom);
	if (stage.song_step >= 376 && stage.song_step <= 384)
	Stage_DrawTex(&this->tex_ded, &ded2_src, &ded2_dst, stage.camera.bzoom);
}

void Back_mono_Free(StageBack *back)
{
	Back_mono *this = (Back_mono*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_mono_New(void)
{
	//Allocate background structure
	Back_mono *this = (Back_mono*)Mem_Alloc(sizeof(Back_mono));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_mono_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = NULL;
	this->back.free = Back_mono_Free;

	//Load background textures
	IO_Data arc_back = IO_Read("\\MONOCH\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_ded, Archive_Find(arc_back, "ded.tim"), 0);
	Gfx_LoadTex(&this->tex_scream, Archive_Find(arc_back, "scream.tim"), 0);
	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);

	return (StageBack*)this;
}
