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

//mono background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Textures
//	Gfx_Tex tex_back0; //bg

} Back_mono;

void Back_mono_DrawBG(StageBack *back)
{
	Back_mono *this = (Back_mono*)back;
    fixed_t fx, fy;

	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;
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
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_mono_DrawBG;
	this->back.free = Back_mono_Free;
	
	//Load background textures
	//IO_Data arc_back = IO_Read("\\MONO\\BACK.ARC;1");
	//Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
//	Mem_Free(arc_back);

	Gfx_SetClear(0, 0, 0);

	return (StageBack*)this;
}
