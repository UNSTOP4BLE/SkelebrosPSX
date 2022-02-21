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
int mog = 0;
int sog = 0;
*/
//mono background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Textures
	Gfx_Tex tex_ded; //imdead

} Back_mono;

void Back_mono_DrawFG(StageBack *back)
{
	Back_mono *this = (Back_mono*)back;
    fixed_t fx, fy;

/*
	FntPrint("x%d y%d", mog, sog);

	if (pad_state.held & PAD_UP)
		sog --;
	if (pad_state.held & PAD_DOWN)
		sog ++;
	if (pad_state.held & PAD_LEFT)
		mog --;
	if (pad_state.held & PAD_RIGHT)
		mog ++;
*/
	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;

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
	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);

	return (StageBack*)this;
}
