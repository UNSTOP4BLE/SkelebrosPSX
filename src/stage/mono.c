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
	this->back.draw_bg = NULL;
	this->back.free = Back_mono_Free;

	Gfx_SetClear(0, 0, 0);

	return (StageBack*)this;
}
