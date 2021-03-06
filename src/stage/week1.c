/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"

fixed_t back_r, back_g, back_b;

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];

	Gfx_Tex tex_back0; //bg

	//Henchmen state
	Gfx_Tex tex_hench;
	u8 hench_frame, hench_tex_id;
	
	Animatable hench_animatable;
} Back_Week1;

//swaptv animation
u8 swaptv = 1;
// TODO make sure to just increase one time the swaptv
boolean avoidproblem;

//Henchmen animation and rects
static const CharFrame henchmen_frame[] = {
	{0, {  0,   0,  68,  50}, { 0,  0}}, //0 left 1
	{0, { 68,   0,  68,  50}, { 0,  0}}, //1 left 2
	{0, {136,   0,  68,  50}, { 0,  0}}, //2 left 3
	{0, {  0,  50,  68,  50}, { 0,  0}}, //3 left 4
	{0, { 68,  50,  68,  50}, { 0,  0}}, //4 left 5

	//static
	{0, {136,  50,  68,  51}, { 0,  0}}, //4 left 5
	{0, {  0, 100,  68,  51}, { 0,  0}}, //4 left 5
	{0, { 68, 100,  68,  51}, { 0,  0}}, //4 left 5
	{0, {136, 100,  68,  51}, { 0,  0}}, //4 left 5

	//spamton lookin doode
	{0, {  0, 151,  68,  51}, { 0,  0}}, //4 left 5
	{0, { 68, 151,  68,  51}, { 0,  0}}, //4 left 5
	{0, {136, 151,  68,  51}, { 0,  0}}, //4 left 5

	//sanz
	{0, {  0, 202,  68,  51}, { 0,  0}}, //4 left 5
	{0, { 68, 202,  68,  51}, { 0,  0}}, //4 left 5
	{0, {136, 202,  68,  51}, { 0,  0}}, //4 left 5
	{1, {  0,   0,  68,  51}, { 0,  0}}, //4 left 5

	//undyne
	{1, { 68,   0,  68, 51}, { 0,  0}}, //5 right 1
	{1, {136,   0,  68, 51}, { 0,  0}}, //6 right 2
	{1, {  0,  51,  68, 51}, { 0,  0}}, //7 right 3

	{1, { 68,  51,  68, 51}, { 0,  0}}, //8 right 4
	{1, {136,  51,  68, 51}, { 0,  0}}, //9 right 5
	{1, {  0, 102,  68, 51}, { 0,  0}}, //9 right 5
};

static const Animation henchmen_anim[] = {
	{1, (const u8[]){5, 5, 6, 7, 8, 5, 6, 7, 8, 5, 6, 7, 8, 5, 6, 7, 8, 5, 6, 7, 8,  ASCR_BACK, 1}}, //static
	{2, (const u8[]){0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //Left
	{2, (const u8[]){9,10,11,9,10,11,9,10,11,9,10,11, 9, ASCR_BACK, 1}}, //fake fish spamton
	{2, (const u8[]){12,13,14,15,12,13,14,15,12,13,14,15,12,13,14,15,12, ASCR_BACK, 1}}, //Sans
	{2, (const u8[]){16,17,18,19,20,21,20,21,20,21,20,21,20,21,20,21, ASCR_BACK, 1}}, //Undyne
};

//Henchmen functions
void Week1_Henchmen_SetFrame(void *user, u8 frame)
{
	Back_Week1 *this = (Back_Week1*)user;
	
	//Check if this is a new frame
	if (frame != this->hench_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &henchmen_frame[this->hench_frame = frame];
		if (cframe->tex != this->hench_tex_id)
			Gfx_LoadTex(&this->tex_hench, this->arc_hench_ptr[this->hench_tex_id = cframe->tex], 0);
	}
}

void Week1_Henchmen_Draw(Back_Week1 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &henchmen_frame[this->hench_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_hench, &src, &dst, stage.camera.bzoom);
}

//Week 1 background functions
void Back_Week1_DrawFG(StageBack *back)
{
	u32 color;
    
	u16 swapcolor = stage.song_step >> 2 & 0x3;

	switch (swapcolor)
	{
	 case 0:
	 color = 0x64ffff;
	 break;
	 case 1 :
	 color = 0xd44790;
	 break;
	 case 2 :
	 color = 0x64d96d;
	 break;
	 case 3 :
	 color = 0x8a47ff;
	 break;
	 default :
	 swapcolor = 0;
	 break;
	}
	//FntPrint("Colors %d", swapcolor);

	fixed_t tgt_r = (fixed_t)((color >> 16) & 0xFF) << FIXED_SHIFT;
	fixed_t tgt_g = (fixed_t)((color >>  8) & 0xFF) << FIXED_SHIFT;
	fixed_t tgt_b = (fixed_t)((color >>  0) & 0xFF) << FIXED_SHIFT;
			
			back_r += (tgt_r - back_r) >> 4;
			back_g += (tgt_g - back_g) >> 4;
			back_b += (tgt_b - back_b) >> 4;

	RECT cool = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    
	if (stage.flashlight)
	if ((stage.stage_id == StageId_1_1  && stage.song_step >= 643 && stage.song_step <= 894) || (stage.stage_id == StageId_1_1Chara && stage.song_step >= 643 && stage.song_step <= 894))
	Gfx_BlendRect(&cool, back_r>> (FIXED_SHIFT + 1), back_g>> (FIXED_SHIFT + 1), back_b>> (FIXED_SHIFT + 1), 1);
}

void Back_Week1_DrawBG(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
    fixed_t fx, fy;

	//Animate and draw henchmen
	fx = stage.camera.x;
	fy = stage.camera.y;

	if (this->hench_animatable.anim == 0 && avoidproblem == true)
	{
	    swaptv += 1;
		avoidproblem = false;
	}
	else if (this->hench_animatable.anim != 0)
	avoidproblem = true;
    
	//reset swaptv
	if (swaptv > 4)
	   swaptv = 1;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step % 10)
		{
			case 0:
                if (this->hench_animatable.anim == 0)
				Animatable_SetAnim(&this->hench_animatable, swaptv);
				else if (this->hench_animatable.anim >= 1)
				Animatable_SetAnim(&this->hench_animatable, 0);
				break;
		}
	}
	Animatable_Animate(&this->hench_animatable, (void*)this, Week1_Henchmen_SetFrame);
	
	Week1_Henchmen_Draw(this, FIXED_DEC(287,1) - fx, FIXED_DEC(0,1) - fy);

	//Draw back
	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {
		FIXED_DEC(-190,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(640,1),
		FIXED_DEC(350,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Week1_Free(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	//Free henchmen archive
	Mem_Free(this->arc_hench);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week1_New(void)
{
	//Allocate background structure
	Back_Week1 *this = (Back_Week1*)Mem_Alloc(sizeof(Back_Week1));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Week1_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week1_DrawBG;
	this->back.free = Back_Week1_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Mem_Free(arc_back);
	
	//Load henchmen textures
	this->arc_hench = IO_Read("\\WEEK1\\BACK.ARC;1");
	this->arc_hench_ptr[0] = Archive_Find(this->arc_hench, "mon0.tim");
	this->arc_hench_ptr[1] = Archive_Find(this->arc_hench, "mon1.tim");

	//Initialize henchmen state
	Animatable_Init(&this->hench_animatable, henchmen_anim);
	Animatable_SetAnim(&this->hench_animatable, 0);
	this->hench_frame = this->hench_tex_id = 0xFF; //Force art load

	return (StageBack*)this;
}
