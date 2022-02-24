/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week2.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
s16 movingbone,bonex1,bonex2,bonex3,bonex4;
s16 swap = 7;

//Week 2 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Textures
	Gfx_Tex tex_back0; //bg
	Gfx_Tex tex_platform; //platform
	Gfx_Tex tex_bones; //bg

} Back_Week2;

void Week2_Bone1_Draw(Back_Week2 *this, fixed_t x, fixed_t y)
{ 
	//Draw bone 1
	RECT src = {3, 4, 50, 249};
	RECT_FIXED dst = {x, y, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};

	Stage_DrawTex(&this->tex_bones, &src, &dst, stage.camera.bzoom);

}
void Week2_Bone2_Draw(Back_Week2 *this, fixed_t x, fixed_t y)
{
	//Draw bone 2
	RECT src = {57,60, 45, 192};
	RECT_FIXED dst = {x, y, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_bones, &src, &dst, stage.camera.bzoom);
}
void Week2_Bone3_Draw(Back_Week2 *this, fixed_t x, fixed_t y)
{
	//Draw bone 3
	RECT src = {102,113, 32, 137};
	RECT_FIXED dst = {x, y, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_bones, &src, &dst, stage.camera.bzoom);
}
void Week2_Bone4_Draw(Back_Week2 *this, fixed_t x, fixed_t y)
{
	//Draw bone 4
	RECT src = {140,113, 36, 137};
	RECT_FIXED dst = {x, y, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_bones, &src, &dst, stage.camera.bzoom);
}

void Back_Week2_DrawBG(StageBack *back)
{
	Back_Week2 *this = (Back_Week2*)back;
    fixed_t fx, fy;

	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;

	movingbone += swap;
    
	if (movingbone >= 222)
	swap = -17;
    
	else if (movingbone <= 25)
	swap = 7;

	else if (movingbone <= 40 && swap != 7)
	swap = -2;

	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {
		FIXED_DEC(-190,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(590,1),
		FIXED_DEC(350,1)
	};

	RECT platform_src = {0, 0, 145, 42};
	RECT_FIXED platform_dst = {
		FIXED_DEC(-81,1) - fx,
		FIXED_DEC(173,1) - fy,
		FIXED_DEC(145,1),
		FIXED_DEC(42,1)
	};
	RECT_FIXED platformbf_dst = {
		FIXED_DEC(133,1) - fx,
		FIXED_DEC(179,1) - fy,
		FIXED_DEC(145,1),
		FIXED_DEC(42,1)
	};
    
	//removed bg and add plataform when undertale mode it's true
	if (stage.utswap == 0)
	{
		for (int i = 0; i <= 9; i++)
		{
		bonex1 = i*60;
		Week2_Bone1_Draw(this,FIXED_DEC(-117 + bonex1,1) - fx, FIXED_DEC(movingbone,1) - fy);
		}
		for (int i = 0; i <= 15; i++)
		{
		bonex2 = i*40;
		Week2_Bone2_Draw(this,FIXED_DEC(-117 + bonex2,1) - fx, FIXED_DEC(+70 + movingbone,1) - fy);
		}
		for (int i = 0; i <= 14; i++)
		{
		bonex3 = i*30;
		Week2_Bone3_Draw(this,FIXED_DEC(-117 + bonex3,1) - fx, FIXED_DEC(+100 + movingbone,1) - fy);
		}
		for (int i = 0; i <= 17; i++)
		{
		bonex4 = i*25;
		Week2_Bone4_Draw(this,FIXED_DEC(-117 + bonex4,1) - fx, FIXED_DEC(+110 + movingbone,1) - fy);
		}
		Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
	    
	}
	
	else 
	{
		Stage_DrawTex(&this->tex_platform, &platform_src, &platform_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_platform, &platform_src, &platformbf_dst, stage.camera.bzoom);
}
}

void Back_Week2_Free(StageBack *back)
{
	Back_Week2 *this = (Back_Week2*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week2_New(void)
{
	//Allocate background structure
	Back_Week2 *this = (Back_Week2*)Mem_Alloc(sizeof(Back_Week2));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week2_DrawBG;
	this->back.free = Back_Week2_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK2\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_platform, Archive_Find(arc_back, "platform.tim"), 0);
	Gfx_LoadTex(&this->tex_bones, Archive_Find(arc_back, "bones.tim"), 0);

	Mem_Free(arc_back);
	
	Gfx_SetClear(0, 0, 0);

	return (StageBack*)this;
}
