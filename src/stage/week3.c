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
    IO_Data arc_space, arc_space_ptr[1];

	Gfx_Tex tex_back0; //bg
	Gfx_Tex tex_misses; //no more than 10 misses

	//space state
	Gfx_Tex tex_space;
	u8 space_frame, space_tex_id;
	
	Animatable space_animatable;
} Back_Week3;

//space animation and rects
static const CharFrame space_frame[] = {
	{0, {  0,  71, 74, 76}, { 37 - 50, 57 - 57}},
	{0, { 75,  72, 81, 82}, { 41 - 50, 62 - 57}},
	{0, {191,   0, 65, 70}, { 33 - 50, 51 - 57}},
	{0, {156,  70, 68, 67}, { 34 - 50, 51 - 57}},
    {0, {  0, 157, 59, 59}, { 30 - 50, 45 - 57}},
	{0, { 60, 157, 62, 63}, { 31 - 50, 48 - 57}},
	{0, {122, 157, 54, 54}, { 27 - 50, 41 - 57}},
	{0, {176, 141, 57, 57}, { 28 - 50, 44 - 57}},
    {0, {177, 204, 50, 49}, { 25 - 50, 38 - 57}},
};

static const Animation space_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, ASCR_BACK, 0}}, //Left
};

//space functions
void Week3_space_SetFrame(void *user, u8 frame)
{
	Back_Week3 *this = (Back_Week3*)user;
	
	//Check if this is a new frame
	if (frame != this->space_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &space_frame[this->space_frame = frame];
		if (cframe->tex != this->space_tex_id)
			Gfx_LoadTex(&this->tex_space, this->arc_space_ptr[this->space_tex_id = cframe->tex], 0);
	}
}

void Week3_space_Draw(Back_Week3 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &space_frame[this->space_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_space, &src, &dst, stage.camera.bzoom);
}

void Back_Week3_DrawFG(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;

    //Animate and draw space
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step % 0x9)
		{
			case 0:
				Animatable_SetAnim(&this->space_animatable, 0);
				break;
		}
	}

    if (dodgesystem.dodge)
    {
		Animatable_Animate(&this->space_animatable, (void*)this, Week3_space_SetFrame);	
		Week3_space_Draw(this, FIXED_DEC(-45,1), FIXED_DEC(-40,1));
	}

	//Draw miss
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
	
    //Free space archive
	Mem_Free(this->arc_space);

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
    
    //Load space textures
	this->arc_space = IO_Read("\\STAGE\\SPACE.ARC;1");
	this->arc_space_ptr[0] = Archive_Find(this->arc_space, "space.tim");

	//Initialize space state
	Animatable_Init(&this->space_animatable, space_anim);
	Animatable_SetAnim(&this->space_animatable, 0);
	this->space_frame = this->space_tex_id = 0xFF; //Force art load

	return (StageBack*)this;
}
