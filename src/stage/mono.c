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
	IO_Data arc_paps, arc_paps_ptr[1];
	IO_Data arc_nyeh, arc_nyeh_ptr[1];

	Gfx_Tex tex_ded; //imdead
	Gfx_Tex tex_scream; //ded paps

	//paps state
	Gfx_Tex tex_paps;
	Gfx_Tex tex_nyeh;
	u8 paps_frame, paps_tex_id;
	u8 nyeh_frame, nyeh_tex_id;
	
	Animatable nyeh_animatable;
	Animatable paps_animatable;
} Back_mono;

//nyeh animation and rects
static const CharFrame nyeh_frame[9] = {
	{0, {0, 0, 62, 21}, {96, 187}}, //0
	{0, {158, 102, 66, 43}, {96, 187}}, //1
	{0, {62, 0, 72, 51}, {96, 187}}, //2
	{0, {135, 0, 81, 53}, {96, 187}}, //3
	{0, {0, 52, 77, 49}, {96, 187}}, //4
	{0, {78, 54, 72, 40}, {96, 187}}, //5
	{0, {150, 53, 75, 42}, {96, 187}}, //6
	{0, {0, 103, 77, 44}, {96, 187}}, //7
	{0, {77, 94, 76, 49}, {96, 187}}, //8
};

static const Animation nyeh_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 8, 7, 8, 7, 8, 4, 1, 0, ASCR_BACK, 0}}, //Left
};

//nyeh functions
void mono_nyeh_SetFrame(void *user, u8 frame)
{
	Back_mono *this = (Back_mono*)user;
	
	//Check if this is a new frame
	if (frame != this->nyeh_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &nyeh_frame[this->nyeh_frame = frame];
		if (cframe->tex != this->nyeh_tex_id)
			Gfx_LoadTex(&this->tex_nyeh, this->arc_nyeh_ptr[this->nyeh_tex_id = cframe->tex], 0);
	}
}

void mono_nyeh_Draw(Back_mono *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &nyeh_frame[this->nyeh_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_nyeh, &src, &dst, stage.camera.bzoom);
}

//paps animation and rects
static const CharFrame paps_frame[12] = {
	{0, {  0,   0,  60,  74}, { 0,  0}}, //0 left 1
	{0, { 60,   0,  58,  76}, { 0,  0}}, //1 left 2
	{0, {118,   0,  58,  76}, { 0,  0}}, //2 left 3
	{0, {177,   0,  57,  76}, { 0,  0}}, //3 left 4

	{0, {  0,  74,  58,  76}, { 0,  0}}, //4 left 5
	{0, { 58,  76,  56,  78}, { 0,  0}}, //5 left 5
	{0, {114,  76,  56,  78}, { 0,  0}}, //6 left 5
	{0, {170,  76,  59,  80}, { 0,  0}}, //7 left 5

	{0, {  0, 156,  59,  83}, { 0,  0}}, //8 left 5
	{0, { 59, 156,  60,  85}, { 0,  0}}, //9 left 5
	{0, {119, 156,  57,  87}, { 0,  0}}, //10 left 5
	{0, {176, 156,  56,  87}, { 0,  0}}, //11 left 5
};

static const Animation paps_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 5, 4, 3, 2, 1, 0, ASCR_BACK, 0}}, //Left
};

//paps functions
void mono_paps_SetFrame(void *user, u8 frame)
{
	Back_mono *this = (Back_mono*)user;
	
	//Check if this is a new frame
	if (frame != this->paps_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &paps_frame[this->paps_frame = frame];
		if (cframe->tex != this->paps_tex_id)
			Gfx_LoadTex(&this->tex_paps, this->arc_paps_ptr[this->paps_tex_id = cframe->tex], 0);
	}
}

void mono_paps_Draw(Back_mono *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &paps_frame[this->paps_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_paps, &src, &dst, stage.camera.bzoom);
}

void Back_mono_DrawFG(StageBack *back)
{
	Back_mono *this = (Back_mono*)back;
    fixed_t fx, fy;

	//Animate and draw paps
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 0x12)
		{
			case 0:
				Animatable_SetAnim(&this->paps_animatable, 0);
				Animatable_SetAnim(&this->nyeh_animatable, 0);
				break;
		}
	}

	//Draw scream
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
	
	if ((stage.song_step >= 368 && stage.song_step <= 368 + 15) || (stage.song_step >= 817 && stage.song_step <= 817 + 15) || (stage.song_step >= 1264 && stage.song_step <= 1264 + 15))
	{
		Animatable_Animate(&this->nyeh_animatable, (void*)this, mono_nyeh_SetFrame);
		mono_nyeh_Draw(this, FIXED_DEC(0,1) - fx, FIXED_DEC(290,1) - fy);

		Animatable_Animate(&this->paps_animatable, (void*)this, mono_paps_SetFrame);
		mono_paps_Draw(this, FIXED_DEC(-120,1) - fx, FIXED_DEC(60,1) - fy);
	}
}

void Back_mono_Free(StageBack *back)
{
	Back_mono *this = (Back_mono*)back;
	
	//Free structure
	Mem_Free(this);
	Mem_Free(this->arc_paps);
	Mem_Free(this->arc_nyeh);
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
	
	//Load paps textures
	this->arc_paps = IO_Read("\\MONOCH\\BACK.ARC;1");
	this->arc_paps_ptr[0] = Archive_Find(this->arc_paps, "paps.tim");
	this->arc_nyeh = IO_Read("\\MONOCH\\BACK.ARC;1");
	this->arc_nyeh_ptr[0] = Archive_Find(this->arc_nyeh, "nyeh.tim");
	
	//Initialize paps state
	Animatable_Init(&this->paps_animatable, paps_anim);
	Animatable_SetAnim(&this->paps_animatable, 0);
	this->paps_frame = this->paps_tex_id = 0xFF; //Force art load
	Animatable_Init(&this->nyeh_animatable, nyeh_anim);
	Animatable_SetAnim(&this->nyeh_animatable, 0);
	this->nyeh_frame = this->nyeh_tex_id = 0xFF; //Force art load	

	Gfx_SetClear(0, 0, 0);

	return (StageBack*)this;
}
