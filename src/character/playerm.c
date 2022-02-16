/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "playerm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//PlayerM character structure
enum
{
	PlayerM_ArcMain_BF0,
	PlayerM_ArcMain_BF1,
	PlayerM_ArcMain_BF2,
	PlayerM_ArcMain_BF3,
	
	//alt
	PlayerM_ArcMain_Chara0,
	PlayerM_ArcMain_Chara1,
	
	PlayerM_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[PlayerM_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_PlayerM;

//PlayerM character definitions
static const CharFrame char_playerm_frame[] = {
	{PlayerM_ArcMain_BF0, {  0,   0,  92,  76}, { 29,  68}}, //0 idle 1
	{PlayerM_ArcMain_BF0, { 96,   0,  92,  76}, { 29,  68}}, //1 idle 2
	{PlayerM_ArcMain_BF0, {  0,  78,  89,  77}, { 26,  69}}, //2 idle 3
	{PlayerM_ArcMain_BF0, { 95,  76,  97,  80}, { 34,  72}}, //3 idle 4
	{PlayerM_ArcMain_BF0, {  0, 156,  98,  81}, { 35,  72}}, //2 idle 5
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6

	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	{PlayerM_ArcMain_BF0, {103, 156, 100,  80}, { 37,  72}}, //3 idle 6
	
	//alts
	{PlayerM_ArcMain_Chara0, {  0,   0,  66,  88}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara0, { 68,   0,  65,  88}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara0, {136,   1,  67,  87}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara0, {  0,  89,  65,  92}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara0, { 66,  88,  65,  93}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara0, {136,  90,  66,  91}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara1, {  0,   0,  66,  92}, { 29,  68}}, //22 idle 1
	{PlayerM_ArcMain_Chara1, { 69,   0,  66,  92}, { 29,  68}}, //22 idle 1
};

static const Animation char_playerm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 1, 2, 3, ASCR_BACK, 1}}, //CharAnim_Special
	{2, (const u8[]){ 6, 7, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 8, 9, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 10, 11, ASCR_BACK, 2}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 12, 13, ASCR_BACK, 2}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

static const Animation char_playerm_animb[CharAnim_Max] = {
	{2, (const u8[]){19, 20, 21, 22, 23, 24, 25, 26, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, ASCR_BACK, 1}}, //CharAnim_Special
	{2, (const u8[]){ 24, 25, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 26, 27, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 28, 29, ASCR_BACK, 2}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 30, 31, ASCR_BACK, 2}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//PlayerM character functions
void Char_PlayerM_SetFrame(void *user, u8 frame)
{
	Char_PlayerM *this = (Char_PlayerM*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_playerm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_PlayerM_Tick(Character *character)
{
	Char_PlayerM *this = (Char_PlayerM*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	if ((stage.stage_id == StageId_1_1 && stage.song_step == 324) || (stage.stage_id == StageId_1_1Chara && stage.song_step == 324))
		character->set_anim(character, CharAnim_Special);


	if ((stage.stage_id == StageId_1_1 && stage.song_step == 763) || (stage.stage_id == StageId_1_1Chara && stage.song_step == 763))
		character->set_anim(character, CharAnim_Special);
	

	if (stage.swap_player)
		Animatable_Animate(&character->animatableb, (void*)this, Char_PlayerM_SetFrame);
	else
		Animatable_Animate(&character->animatable, (void*)this, Char_PlayerM_SetFrame);

	Character_Draw(character, &this->tex, &char_playerm_frame[this->frame]);
}

void Char_PlayerM_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatableb, anim);
	Character_CheckStartSing(character);
}

void Char_PlayerM_Free(Character *character)
{
	Char_PlayerM *this = (Char_PlayerM*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_PlayerM_New(fixed_t x, fixed_t y)
{
	//Allocate playerm object
	Char_PlayerM *this = Mem_Alloc(sizeof(Char_PlayerM));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_PlayerM_New] Failed to allocate playerm object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_PlayerM_Tick;
	this->character.set_anim = Char_PlayerM_SetAnim;
	this->character.free = Char_PlayerM_Free;
	
	Animatable_Init(&this->character.animatable, char_playerm_anim);
	Animatable_Init(&this->character.animatableb, char_playerm_animb);	
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\PLAYERM.ARC;1");
	
	const char **pathp = (const char *[]){
		"BF0.tim", //PlayerM_ArcMain_Idle0
		"BF1.tim", //PlayerM_ArcMain_Idle1
		"BF2.tim", //PlayerM_ArcMain_Idle1
		"BF3.tim", //PlayerM_ArcMain_Idle1

		//alt
		"Chara0.tim", //PlayerM_ArcMain_Idle0
		"Chara1.tim", //PlayerM_ArcMain_Idle1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
