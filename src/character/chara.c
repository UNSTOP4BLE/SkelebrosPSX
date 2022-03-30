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
	Chara_ArcMain_Up0,
	Chara_ArcMain_Up1,
	Chara_ArcMain_Right0,
	Chara_ArcMain_Right1,

	//alt
	Chara_ArcMain_Idleb0,
	Chara_ArcMain_Idleb1,
	Chara_ArcMain_Idleb2,
	Chara_ArcMain_Idleb3,
	Chara_ArcMain_Leftb0,
	Chara_ArcMain_Leftb1,
	Chara_ArcMain_Downb0,
	Chara_ArcMain_Downb1,
	Chara_ArcMain_Upb0,
	Chara_ArcMain_Upb1,
	Chara_ArcMain_Rightb0,
	Chara_ArcMain_Rightb1,

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
	{Chara_ArcMain_Idle0, {  0,   1, 101, 154}, { 58, 154}}, //0 idle 1
	{Chara_ArcMain_Idle0, {101,   0, 102, 155}, { 58, 155}}, //1 idle 2
	{Chara_ArcMain_Idle1, {  0,   0, 103, 155}, { 58, 155}}, //2 idle 3
	{Chara_ArcMain_Idle1, {103,   0, 100, 155}, { 56, 155}}, //3 idle 4
	{Chara_ArcMain_Idle2, {  0,   0,  99, 155}, { 55, 155}}, //4 idle 5
	{Chara_ArcMain_Idle2, { 99,   0, 100, 155}, { 56, 155}}, //5 idle 6
	{Chara_ArcMain_Idle3, {  0,   0, 100, 156}, { 57, 156}}, //6 idle 7
	{Chara_ArcMain_Idle3, {100,   0, 100, 155}, { 56, 155}}, //7 idle 8
	
	{Chara_ArcMain_Left0, {  0,   0, 126, 137}, { 92, 137}}, //8 left 1
	{Chara_ArcMain_Left1, {  0,   0, 130, 134}, { 96, 134}}, //9 left 2
	{Chara_ArcMain_Left2, {  0,   0, 131, 133}, { 98, 133}}, //10 left 1
	{Chara_ArcMain_Left3, {  0,   0, 132, 133}, { 99, 133}}, //11 left 2
	
	{Chara_ArcMain_Down0, {  0,   0, 140, 150}, { 73, 150}}, //12 down 1
	{Chara_ArcMain_Down1, {  0,   0, 136, 152}, { 72, 152}}, //13 down 2
	{Chara_ArcMain_Down2, {  0,   0, 117, 172}, { 60, 172}}, //14 down 1
	{Chara_ArcMain_Down2, {117,   1, 111, 171}, { 63, 171}}, //15 down 2
	
	{Chara_ArcMain_Up0, {  0,   0,  84, 163}, { 47, 163}}, //16 up 1
	{Chara_ArcMain_Up0, { 84,   0,  87, 168}, { 50, 168}}, //17 up 2
	{Chara_ArcMain_Up1, {  0,   0,  87, 168}, { 49, 168}}, //18 up 1
	{Chara_ArcMain_Up1, { 87,   0,  87, 168}, { 49, 168}}, //19 up 2

	{Chara_ArcMain_Right0, {  0,   0,  99, 149}, { 56, 149}}, //20 right 1
	{Chara_ArcMain_Right0, { 99,   1, 104, 148}, { 58, 148}}, //21 right 2
	{Chara_ArcMain_Right1, {  0,   0,  89, 150}, { 55, 150}}, //22 right 1
	{Chara_ArcMain_Right1, { 92,   1,  92, 150}, { 54, 150}}, //23 right 2

	//alt
	{Chara_ArcMain_Idleb0, {  0,   0, 106, 154}, {  0,   0}}, //0 idle 1
	{Chara_ArcMain_Idleb0, {106,   0, 106, 154}, {  0,   0}}, //1 idle 2
	{Chara_ArcMain_Idleb1, {  0,   0, 106, 155}, {  0,   0}}, //2 idle 3
	{Chara_ArcMain_Idleb1, {106,   0, 104, 155}, {  0,   0}}, //3 idle 4
	{Chara_ArcMain_Idleb2, {  0,   0, 102, 156}, {  0,   0}}, //4 idle 5
	{Chara_ArcMain_Idleb2, {102,   0, 103, 156}, {  0,   0}}, //5 idle 6
	{Chara_ArcMain_Idleb3, {  0,   0, 103, 156}, {  0,   0}}, //6 idle 7
	{Chara_ArcMain_Idleb3, {103,   0, 103, 156}, {  0,   0}}, //7 idle 8
	
	{Chara_ArcMain_Leftb0, {  0,   0, 108, 154}, {  0,   0}}, //8 left 1
	{Chara_ArcMain_Leftb0, {108,   0, 108, 153}, {  0,   0}}, //9 left 2
	{Chara_ArcMain_Leftb1, {  0,   0, 105, 156}, {  0,   0}}, //10 left 1
	{Chara_ArcMain_Leftb1, {105,   0, 109, 156}, {  0,   0}}, //11 left 2
	
	{Chara_ArcMain_Downb0, {  0,   0, 102, 150}, {  0,   0}}, //12 down 1
	{Chara_ArcMain_Downb0, {102,   0, 101, 149}, {  0,   0}}, //13 down 2
	{Chara_ArcMain_Downb1, {  0,   0, 108, 153}, {  0,   0}}, //14 down 1
	{Chara_ArcMain_Downb1, {107,   0, 106, 152}, {  0,   0}}, //15 down 2
	
	{Chara_ArcMain_Upb0, {  0,   0,   115, 163}, {  0,   0}}, //16 up 1
	{Chara_ArcMain_Upb0, {115,   0,   114, 164}, {  0,   0}}, //17 up 2
	{Chara_ArcMain_Upb1, {  0,   0,   112, 168}, {  0,   0}}, //18 up 1
	{Chara_ArcMain_Upb1, {112,   0,   114, 167}, {  0,   0}}, //19 up 2

	{Chara_ArcMain_Rightb0, {  0,   0,  88, 152}, {  0,   0}}, //20 right 1
	{Chara_ArcMain_Rightb0, { 88,   0,  89, 152}, {  0,   0}}, //21 right 2
	{Chara_ArcMain_Rightb1, {  0,   0,  90, 155}, {  0,   0}}, //22 right 1
	{Chara_ArcMain_Rightb1, { 91,   0,  89, 154}, {  0,   0}}, //23 right 2
};

static const Animation char_chara_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, ASCR_BACK, 1}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Special
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 12, 13, 14, 15, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 16, 17, 18, 19, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 20, 21, 22, 23, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

static const Animation char_chara_animb[CharAnim_Max] = {
	{2, (const u8[]){ 0+24, 1+24, 2+24, 3+24, 4+24, 5+24, 6+24, 7+24, ASCR_BACK, 1}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Special
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_BACK, 1}},         //CharAnim_Left
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
	Animatable_SetAnim(&character->animatableb, anim);
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
	Animatable_Init(&this->character.animatableb, char_chara_animb);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = stage.tex_hud1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-103,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
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
		"up0.tim",    //Chara_ArcMain_Up
		"up1.tim",    //Chara_ArcMain_Up
		"right0.tim", //Chara_ArcMain_Right
		"right1.tim", //Chara_ArcMain_Right

		"idleb0.tim", //Chara_ArcMain_Idle0
		"idleb1.tim", //Chara_ArcMain_Idle1
		"idleb2.tim", //Chara_ArcMain_Idle2
		"idleb3.tim", //Chara_ArcMain_Idle3
		"leftb0.tim",  //Chara_ArcMain_Left
		"leftb1.tim",  //Chara_ArcMain_Left
		"downb0.tim",  //Chara_ArcMain_Down
		"downb1.tim",  //Chara_ArcMain_Down
		"upb0.tim",    //Chara_ArcMain_Up
		"upb1.tim",    //Chara_ArcMain_Up
		"rightb0.tim", //Chara_ArcMain_Right
		"rightb1.tim", //Chara_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
