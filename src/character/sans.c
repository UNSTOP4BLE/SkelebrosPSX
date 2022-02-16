/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sans.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//sans character structure
enum
{
	sans_ArcMain_Idle0,
	sans_ArcMain_Idle1,
	sans_ArcMain_Left0,
	sans_ArcMain_Left1,
	sans_ArcMain_Down,
	sans_ArcMain_Up0,
	sans_ArcMain_Up1,
	sans_ArcMain_Right,

	sans_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[sans_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_sans;

//sans character definitions
static const CharFrame char_sans_frame[] = {
	{sans_ArcMain_Idle0, {  0,   0, 86, 134}, { 44, 134}}, //0 idle 1
	{sans_ArcMain_Idle0, { 86,   0, 84, 131}, { 45, 131}}, //1 idle 2
	{sans_ArcMain_Idle0, {170,   0, 82, 129}, { 46, 129}}, //2 idle 3
	{sans_ArcMain_Idle1, {  0,   0, 82, 133}, { 44, 133}}, //3 idle 4
	{sans_ArcMain_Idle1, { 82,   0, 90, 133}, { 43, 133}}, //2 idle 3
	{sans_ArcMain_Idle1, {163, 123, 93, 133}, { 44, 133}}, //3 idle 4
	
	{sans_ArcMain_Left0, {  0,   0, 126, 136}, { 71, 136}}, //4 left 1
	{sans_ArcMain_Left0, {126,   0, 126, 136}, { 71, 136}}, //5 left 2
	{sans_ArcMain_Left1, {  0,   0, 126, 136}, { 73, 136}}, //4 left 1
	{sans_ArcMain_Left1, {126,   0, 126, 137}, { 73, 137}}, //5 left 2
	
	{sans_ArcMain_Down, {  0,   0, 105, 120}, { 44, 120}}, //6 down 1
	{sans_ArcMain_Down, {105,   0, 105, 119}, { 44, 119}}, //7 down 2
	{sans_ArcMain_Down, {  0, 120, 106, 116}, { 45, 116}}, //6 down 1
	{sans_ArcMain_Down, {106, 120, 105, 116}, { 44, 116}}, //7 down 2
	
	{sans_ArcMain_Up0, {  0,   0, 90, 139}, { 48, 139}}, //8 up 1
	{sans_ArcMain_Up0, { 90,   0, 90, 140}, { 48, 140}}, //9 up 2
	{sans_ArcMain_Up1, {  0,   0, 91, 140}, { 49, 140}}, //8 up 1
	{sans_ArcMain_Up1, { 91,   0, 92, 139}, { 49, 139}}, //9 up 2

	{sans_ArcMain_Right, {  0,   0, 117, 128}, { 43, 128}}, //10 right 1
	{sans_ArcMain_Right, {117,   0, 116, 128}, { 42, 128}}, //11 right 2
	{sans_ArcMain_Right, {  0, 128, 115, 128}, { 41, 128}}, //10 right 1
	{sans_ArcMain_Right, {116, 128, 116, 128}, { 41, 128}}, //11 right 2
};

static const Animation char_sans_anim[CharAnim_Max] = {
	{3, (const u8[]){ 0, 0, 1, 2, 3, 4, 5, 4, 3, 2, 1, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
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

//sans character functions
void Char_sans_SetFrame(void *user, u8 frame)
{
	Char_sans *this = (Char_sans*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sans_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_sans_Tick(Character *character)
{
	Char_sans *this = (Char_sans*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_sans_SetFrame);
	Character_Draw(character, &this->tex, &char_sans_frame[this->frame]);
}

void Char_sans_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_sans_Free(Character *character)
{
	Char_sans *this = (Char_sans*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_sans_New(fixed_t x, fixed_t y)
{
	//Allocate sans object
	Char_sans *this = Mem_Alloc(sizeof(Char_sans));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_sans_New] Failed to allocate sans object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_sans_Tick;
	this->character.set_anim = Char_sans_SetAnim;
	this->character.free = Char_sans_Free;
	
	Animatable_Init(&this->character.animatable, char_sans_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-78,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SANS.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //sans_ArcMain_Idle0
		"idle1.tim", //sans_ArcMain_Idle1
		"left0.tim",  //sans_ArcMain_Left
		"left1.tim",  //sans_ArcMain_Left
		"down.tim",  //sans_ArcMain_Down
		"up0.tim",    //sans_ArcMain_Up
		"up1.tim",    //sans_ArcMain_Up
		"right.tim", //sans_ArcMain_Right

		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
