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
	Chara_ArcMain_Idle2,
	Chara_ArcMain_Idle3,
	Chara_ArcMain_Left0,
	Chara_ArcMain_Left1,
	Chara_ArcMain_Left2,
	Chara_ArcMain_Left3,
	Chara_ArcMain_Down0,
	Chara_ArcMain_Down1,
	Chara_ArcMain_Down2,
	Chara_ArcMain_Down3,
	Chara_ArcMain_Up0,
	Chara_ArcMain_Up1,
	Chara_ArcMain_Up2,
	Chara_ArcMain_Up3,
	Chara_ArcMain_Right0,
	Chara_ArcMain_Right1,
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
	{Chara_ArcMain_Idle0, {  2,   1, 101, 154}, { 44, 134}}, //0 idle 1
	{Chara_ArcMain_Idle0, {105,   0, 102, 155}, { 44, 134}}, //1 idle 2
	{Chara_ArcMain_Idle1, {  0,   0, 105, 155}, { 44, 134}}, //2 idle 3
	{Chara_ArcMain_Idle1, {108,   0, 100, 155}, { 44, 134}}, //3 idle 4
	{Chara_ArcMain_Idle2, {  6,   0,  99, 156}, { 44, 134}}, //4 idle 5
	{Chara_ArcMain_Idle2, {108,   0, 100, 155}, { 44, 134}}, //5 idle 6
	{Chara_ArcMain_Idle3, {  4,   0, 101, 156}, { 44, 136}}, //6 idle 7
	{Chara_ArcMain_Idle3, {108,   0, 100, 155}, { 44, 134}}, //7 idle 8
	
	{Chara_ArcMain_Left0, {  0,   0, 136, 139}, { 71, 136}}, //8 left 1
	{Chara_ArcMain_Left1, {  0,   0, 136, 139}, { 71, 136}}, //9 left 2
	{Chara_ArcMain_Left2, {  0,   0, 136, 139}, { 71, 136}}, //10 left 1
	{Chara_ArcMain_Left3, {  0,   0, 136, 139}, { 71, 136}}, //11 left 2
	
	{Chara_ArcMain_Down0, {  0,   0, 160, 179}, { 44, 120}}, //12 down 1
	{Chara_ArcMain_Down1, {  0,   0, 160, 179}, { 44, 120}}, //13 down 2
	{Chara_ArcMain_Down2, {  0,   0, 160, 179}, { 44, 120}}, //14 down 1
	{Chara_ArcMain_Down3, {  0,   0, 160, 179}, { 44, 120}}, //15 down 2
	
	{Chara_ArcMain_Up0, {  0,   0,160, 179}, { 48, 139}}, //16 up 1
	{Chara_ArcMain_Up1, {  0,   0,160, 179}, { 48, 139}}, //17 up 2
	{Chara_ArcMain_Up2, {  0,   0,160, 179}, { 48, 139}}, //18 up 1
	{Chara_ArcMain_Up3, {  0,   0,160, 179}, { 48, 139}}, //19 up 2

	{Chara_ArcMain_Right0, {  0,   0, 107, 161}, { 43, 128}}, //20 right 1
	{Chara_ArcMain_Right0, {111,   0, 118, 161}, { 42, 128}}, //21 right 2
	{Chara_ArcMain_Right1, {  0,   0, 107, 161}, { 43, 128}}, //22 right 1
	{Chara_ArcMain_Right1, {111,   0, 118, 161}, { 42, 128}}, //23 right 2

};

static const Animation char_chara_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, ASCR_BACK, 1}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Special
	{2, (const u8[]){ 8, 9,10,11, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
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
		"idle2.tim", //Chara_ArcMain_Idle2
		"idle3.tim", //Chara_ArcMain_Idle3
		"left0.tim",  //Chara_ArcMain_Left
		"left1.tim",  //Chara_ArcMain_Left
		"left2.tim",  //Chara_ArcMain_Left
		"left3.tim",  //Chara_ArcMain_Left
		"down0.tim",  //Chara_ArcMain_Down
		"down1.tim",  //Chara_ArcMain_Down
		"down2.tim",  //Chara_ArcMain_Down
		"down3.tim",  //Chara_ArcMain_Down
		"up0.tim",    //Chara_ArcMain_Up
		"up1.tim",    //Chara_ArcMain_Up
		"up2.tim",    //Chara_ArcMain_Up
		"up3.tim",    //Chara_ArcMain_Up
		"right0.tim", //Chara_ArcMain_Right
		"right1.tim", //Chara_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
