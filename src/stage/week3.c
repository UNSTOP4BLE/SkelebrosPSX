/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week3.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"

//Week 3 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Textures
	Gfx_Tex tex_back0; //bg
	Gfx_Tex tex_misses; //no more than 10 misses

} Back_Week3;

void Back_Week3_DrawFG(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;

	//Draw misses
	RECT miss_src = {1, 1, 254, 16};
	RECT_FIXED miss_dst = {
		FIXED_DEC(-159,1),
		FIXED_DEC(-36,1),
		FIXED_DEC(254,1),
		FIXED_DEC(16,1)
	};

	RECT miss2_src = {1, 18, 64, 13};
	RECT_FIXED miss2_dst = {
		FIXED_DEC(95,1),
		FIXED_DEC(-33,1),
		FIXED_DEC(64,1),
		FIXED_DEC(13,1)
	};
	if (stage.stage_id == StageId_1_4 && stage.song_step >= 12 && stage.song_step <= 56) 
	{
		Stage_DrawTex(&this->tex_misses, &miss_src, &miss_dst, stage.bump);
		Stage_DrawTex(&this->tex_misses, &miss2_src, &miss2_dst, stage.bump);
	}
}

void Back_Week3_DrawBG(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
    fixed_t fx, fy;

	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;

	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {
		FIXED_DEC(-278,1) - fx,
		FIXED_DEC(-180,1) - fy,
		FIXED_DEC(512,1),
		FIXED_DEC(360,1)
	};
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Week3_Free(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week3_New(void)
{
	//Allocate background structure
	Back_Week3 *this = (Back_Week3*)Mem_Alloc(sizeof(Back_Week3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Week3_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week3_DrawBG;
	this->back.free = Back_Week3_Free;
	
	Gfx_SetClear(0, 0, 0);

	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK3\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_misses, Archive_Find(arc_back, "misses.tim"), 0);
	Mem_Free(arc_back);

	return (StageBack*)this;
}
