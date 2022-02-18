/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "spm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//SPM character structure
enum
{
	SPM_ArcMain_Idle0,
	SPM_ArcMain_Idle1,
	SPM_ArcMain_Idle2,
	SPM_ArcMain_Idle3,
	SPM_ArcMain_Idle4,
	SPM_ArcMain_Idle5,
	
	SPM_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[SPM_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_SPM;

//SPM character definitions
static const CharFrame char_spm_frame[] = {
	{SPM_ArcMain_Idle0, { 17,   0, 151, 112}, {79, 112}}, //0 idle 1
	{SPM_ArcMain_Idle0, { 19, 116, 151, 112}, {79, 112}}, //1 idle 2
	{SPM_ArcMain_Idle1, { 19,   0, 148, 112}, {78, 112}}, //2 idle 3
	{SPM_ArcMain_Idle1, { 20, 116, 144, 112}, {79, 112}}, //3 idle 4
	{SPM_ArcMain_Idle2, { 20,   0, 140, 112}, {80, 112}}, //2 idle 3
	{SPM_ArcMain_Idle2, { 20, 116, 145, 112}, {81, 112}}, //3 idle 4
	{SPM_ArcMain_Idle3, { 17,   0, 143, 112}, {81, 112}}, //3 idle 4
	{SPM_ArcMain_Idle3, { 21, 116, 155, 112}, {81, 112}}, //3 idle 4
	{SPM_ArcMain_Idle4, { 17,   0, 157, 112}, {81, 112}}, //3 idle 4
	{SPM_ArcMain_Idle4, { 21, 116, 159, 112}, {81, 112}}, //3 idle 4
	{SPM_ArcMain_Idle5, { 19,   0, 160, 112}, {81, 112}}, //3 idle 4
	{SPM_ArcMain_Idle5, { 21, 116, 158, 112}, {81, 112}}, //3 idle 4
	
};

static const Animation char_spm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},  //CharAnim_Special
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//SPM character functions
void Char_SPM_SetFrame(void *user, u8 frame)
{
	Char_SPM *this = (Char_SPM*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_spm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_SPM_Tick(Character *character)
{
	Char_SPM *this = (Char_SPM*)character;
	

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	Animatable_Animate(&character->animatable, (void*)this, Char_SPM_SetFrame);
	Character_Draw(character, &this->tex, &char_spm_frame[this->frame]);
}

void Char_SPM_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_SPM_Free(Character *character)
{
	Char_SPM *this = (Char_SPM*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_SPM_New(fixed_t x, fixed_t y)
{
	//Allocate spm object
	Char_SPM *this = Mem_Alloc(sizeof(Char_SPM));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_SPM_New] Failed to allocate spm object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_SPM_Tick;
	this->character.set_anim = Char_SPM_SetAnim;
	this->character.free = Char_SPM_Free;
	
	Animatable_Init(&this->character.animatable, char_spm_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SPM.ARC;1");
	
	const char **pathp = (const char *[]){
		"sp0.tim", //SPM_ArcMain_Idle0
		"sp1.tim", //SPM_ArcMain_Idle1
		"sp2.tim", //SPM_ArcMain_Idle1
		"sp3.tim", //SPM_ArcMain_Idle1
		"sp4.tim", //SPM_ArcMain_Idle1
		"sp5.tim", //SPM_ArcMain_Idle1

		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
