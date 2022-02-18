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
	{SPM_ArcMain_Idle0, {  0,   0, 191, 213}, {127, 212}}, //0 idle 1
	{SPM_ArcMain_Idle1, {  0,   0, 198, 209}, {135, 208}}, //1 idle 2
	{SPM_ArcMain_Idle2, {  0,   0, 189, 214}, {125, 213}}, //2 idle 3
	{SPM_ArcMain_Idle3, {  0,   0, 191, 215}, {124, 214}}, //3 idle 4
	{SPM_ArcMain_Idle4, {  0,   0, 188, 215}, {122, 214}}, //2 idle 3
	{SPM_ArcMain_Idle5, {  0,   0, 191, 215}, {124, 214}}, //3 idle 4
};

static const Animation char_spm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 17, ASCR_BACK, 1}}, //CharAnim_Special
	{2, (const u8[]){ 6, 7, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8, 9, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 10, 11, ASCR_BACK, 2}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 12, 13, ASCR_BACK, 2}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

static const Animation char_spm_animb[CharAnim_Max] = {
	{2, (const u8[]){ 18, 19, 20, 21, 22, 23, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 32, 33, 34, 35, 32, 33, 34, 35, 32, 33, 34, 35, ASCR_BACK, 1}}, //CharAnim_Special
	{2, (const u8[]){ 24, 25, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 26, 27, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 28, 29, ASCR_BACK, 2}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 30, 31, ASCR_BACK, 2}},         //CharAnim_Right
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
	
	//Animate and draw
	if ((stage.stage_id == StageId_1_1 && stage.song_step == 324) || (stage.stage_id == StageId_1_1Chara && stage.song_step == 324))
		character->set_anim(character, CharAnim_Special);


	if ((stage.stage_id == StageId_1_1 && stage.song_step == 763) || (stage.stage_id == StageId_1_1Chara && stage.song_step == 763))
		character->set_anim(character, CharAnim_Special);
	

	if ((stage.stage_id == StageId_1_1 && stage.song_step >= 336 && stage.song_step <= 895) || (stage.stage_id == StageId_1_1Chara && stage.song_step >= 336 && stage.song_step <= 895))
		Animatable_Animate(&character->animatableb, (void*)this, Char_SPM_SetFrame);
	else
		Animatable_Animate(&character->animatable, (void*)this, Char_SPM_SetFrame);

	Character_Draw(character, &this->tex, &char_spm_frame[this->frame]);
}

void Char_SPM_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatableb, anim);
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
	Animatable_Init(&this->character.animatableb, char_spm_animb);	
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
