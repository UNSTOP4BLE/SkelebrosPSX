/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "chara.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Chara character structure
enum
{
	Chara_ArcMain_Idle0,
	Chara_ArcMain_Idle1,
	Chara_ArcMain_Left0,
	Chara_ArcMain_Left1,
	Chara_ArcMain_Down,
	Chara_ArcMain_Up0,
	Chara_ArcMain_Up1,
	Chara_ArcMain_Right,
	Chara_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Chara_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Chara;

//Chara character definitions
static const CharFrame char_chara_frame[] = {
	{Chara_ArcMain_Idle0, {  0,   0, 86, 134}, { 44, 134}}, //0 idle 1
	{Chara_ArcMain_Idle0, { 86,   0, 84, 131}, { 45, 131}}, //1 idle 2
	{Chara_ArcMain_Idle0, {170,   0, 82, 129}, { 46, 129}}, //2 idle 3
	{Chara_ArcMain_Idle1, {  0,   0, 82, 133}, { 44, 133}}, //3 idle 4
	{Chara_ArcMain_Idle1, { 82,   0, 90, 133}, { 43, 133}}, //2 idle 3
	{Chara_ArcMain_Idle1, {163, 123, 93, 133}, { 44, 133}}, //3 idle 4
	
	{Chara_ArcMain_Left0, {  0,   0, 126, 136}, { 71, 136}}, //4 left 1
	{Chara_ArcMain_Left0, {126,   0, 126, 136}, { 71, 136}}, //5 left 2
	{Chara_ArcMain_Left1, {  0,   0, 126, 136}, { 73, 136}}, //4 left 1
	{Chara_ArcMain_Left1, {126,   0, 126, 137}, { 73, 137}}, //5 left 2
	
	{Chara_ArcMain_Down, {  0,   0, 105, 120}, { 44, 120}}, //6 down 1
	{Chara_ArcMain_Down, {105,   0, 105, 119}, { 44, 119}}, //7 down 2
	{Chara_ArcMain_Down, {  0, 120, 106, 116}, { 45, 116}}, //6 down 1
	{Chara_ArcMain_Down, {106, 120, 105, 116}, { 44, 116}}, //7 down 2
	
	{Chara_ArcMain_Up0, {  0,   0, 90, 139}, { 48, 139}}, //8 up 1
	{Chara_ArcMain_Up0, { 90,   0, 90, 140}, { 48, 140}}, //9 up 2
	{Chara_ArcMain_Up1, {  0,   0, 91, 140}, { 49, 140}}, //8 up 1
	{Chara_ArcMain_Up1, { 91,   0, 92, 139}, { 49, 139}}, //9 up 2

	{Chara_ArcMain_Right, {  0,   0, 117, 128}, { 43, 128}}, //10 right 1
	{Chara_ArcMain_Right, {117,   0, 116, 128}, { 42, 128}}, //11 right 2
	{Chara_ArcMain_Right, {  0, 128, 115, 128}, { 41, 128}}, //10 right 1
	{Chara_ArcMain_Right, {116, 128, 116, 128}, { 41, 128}}, //11 right 2

};

static const Animation char_chara_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 0, 1, 2, 3, 4, 5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Special
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 14, 15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 18, 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Chara character functions
void Char_Chara_SetFrame(void *user, u8 frame)
{
	Char_Chara *this = (Char_Chara*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_chara_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Chara_Tick(Character *character)
{
	Char_Chara *this = (Char_Chara*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);

	this->character.number_i = 13;
	this->character.swap_i = stage.song_step  % 0xD;
	this->character.swapdeath_i = stage.song_step *2 % 0x6;
    
	Animatable_Animate(&character->animatable, (void*)this, Char_Chara_SetFrame);
	Character_Draw(character, &this->tex, &char_chara_frame[this->frame]);
}

void Char_Chara_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Chara_Free(Character *character)
{
	Char_Chara *this = (Char_Chara*)character;

	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Chara_New(fixed_t x, fixed_t y)
{
	//Allocate Chara object
	Char_Chara *this = Mem_Alloc(sizeof(Char_Chara));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Chara_New] Failed to allocate Chara object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Chara_Tick;
	this->character.set_anim = Char_Chara_SetAnim;
	this->character.free = Char_Chara_Free;
	
	Animatable_Init(&this->character.animatable, char_chara_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = stage.tex_hud1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-103,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CHARA.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Chara_ArcMain_Idle0
		"idle1.tim", //Chara_ArcMain_Idle1
		"left0.tim",  //Chara_ArcMain_Left
		"left1.tim",  //Chara_ArcMain_Left
		"down0.tim",  //Chara_ArcMain_Down
		"up0.tim",    //Chara_ArcMain_Up
		"up1.tim",    //Chara_ArcMain_Up
		"right0.tim", //Chara_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
