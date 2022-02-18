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

	//alt
	sans_ArcMain_Idleb0,
	sans_ArcMain_Idleb1,
	sans_ArcMain_Leftb0,
	sans_ArcMain_Leftb1,
	sans_ArcMain_Downb,
	sans_ArcMain_Upb0,
	sans_ArcMain_Upb1,
	sans_ArcMain_Rightb,

	sans_ArcMain_Badtime0,
	sans_ArcMain_Badtime1,
	sans_ArcMain_Badtime2,
	sans_ArcMain_Badtime3,


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

	//alt
	{sans_ArcMain_Idleb0, {  0,   0,  86, 134}, { 44, 134}}, //0 idle 1
	{sans_ArcMain_Idleb0, { 86,   3,  84, 131}, { 46, 131}}, //1 idle 2
	{sans_ArcMain_Idleb0, {170,   5,  83, 129}, { 46, 129}}, //2 idle 3
	{sans_ArcMain_Idleb1, {  0,   0,  82, 133}, { 44, 133}}, //3 idle 4
	{sans_ArcMain_Idleb1, { 82,   0,  90, 134}, { 43, 134}}, //2 idle 3
	{sans_ArcMain_Idleb1, {163, 123,  93, 133}, { 43, 133}}, //3 idle 4
	
	{sans_ArcMain_Leftb0, {  0,   0, 125, 136}, { 70, 136}}, //4 left 1
	{sans_ArcMain_Leftb0, {125,   0, 126, 136}, { 70, 136}}, //5 left 2
	{sans_ArcMain_Leftb1, {  0,   0, 127, 136}, { 73, 136}}, //4 left 1
	{sans_ArcMain_Leftb1, {127,   0, 127, 136}, { 73, 136}}, //5 left 2
	
	{sans_ArcMain_Downb, {  0,   0, 105, 120}, { 44, 120}}, //6 down 1
	{sans_ArcMain_Downb, {105,   0, 106, 119}, { 45, 119}}, //7 down 2
	{sans_ArcMain_Downb, {  0, 120, 106, 115}, { 45, 115}}, //6 down 1
	{sans_ArcMain_Downb, {107, 120, 104, 115}, { 44, 155}}, //7 down 2
	
	{sans_ArcMain_Upb0, {  0,   0,  90, 139}, { 48, 139}}, //8 up 1
	{sans_ArcMain_Upb0, { 90,   0,  90, 140}, { 48, 140}}, //9 up 2
	{sans_ArcMain_Upb1, {  0,   0,  90, 140}, { 48, 140}}, //8 up 1
	{sans_ArcMain_Upb1, { 90,   0,  91, 139}, { 48, 139}}, //9 up 2

	{sans_ArcMain_Rightb, {  0,   0, 116, 128}, { 43, 128}}, //10 right 1
	{sans_ArcMain_Rightb, {116,   0, 116, 128}, { 42, 128}}, //11 right 2
	{sans_ArcMain_Rightb, {  0, 128, 116, 128}, { 41, 128}}, //10 right 1
	{sans_ArcMain_Rightb, {116, 128, 116, 128}, { 41, 128}}, //11 right 2

	//special mogus
	{sans_ArcMain_Badtime0, {  0,   0,  87, 134}, { 44, 134}}, //44 right 1 0
	{sans_ArcMain_Badtime0, { 87,   0,  97, 137}, { 42, 137}}, //45 right 2 1
	{sans_ArcMain_Badtime1, {  0,   0, 106, 139}, { 41, 139}}, //46 right 1 2 
	{sans_ArcMain_Badtime1, {106,   0, 119, 137}, { 38, 137}}, //47 right 2 3
	{sans_ArcMain_Badtime2, {  0,   0, 119, 138}, { 38, 138}}, //48 right 1 4
	{sans_ArcMain_Badtime2, {119,   0, 119, 138}, { 38, 138}}, //49 right 2 5 
	{sans_ArcMain_Badtime3, {  0,   0, 119, 139}, { 38, 139}}, //50 right 1 6
	{sans_ArcMain_Badtime3, {119,   0, 119, 139}, { 38, 139}}, //51 right 2 7
};

static const Animation char_sans_anim[CharAnim_Max] = {
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

static const Animation char_sans_animb[CharAnim_Max] = {
	{3, (const u8[]){ 22, 23, 24, 25, 26, 27, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 44, 45, 46, 47, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,  48, 48, 49, 50, 51, 51, ASCR_BACK, 0}}, //CharAnim_Special
	{2, (const u8[]){ 28, 29, 30, 31, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 32, 33, 34, 35, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 36, 37, 38, 39, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 40, 41, 42, 43, ASCR_BACK, 0}},         //CharAnim_Right
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
	

	//bad time
	if ((stage.stage_id == StageId_1_3 && stage.song_step == 687) || (stage.stage_id == StageId_1_3Chara && stage.song_step == 687))
		character->set_anim(character, CharAnim_Special);

	if ((stage.stage_id == StageId_1_3 && stage.song_step >= 687 && stage.song_step <= 703) || (stage.stage_id == StageId_1_3Chara && stage.song_step >= 687 && stage.song_step <= 703))
		this->character.focus_zoom = FIXED_DEC(14,10);
	else 
		this->character.focus_zoom = FIXED_DEC(9,10);

	if ((stage.stage_id == StageId_1_3 && stage.song_step >= 700 && stage.song_step <= 704) || (stage.stage_id == StageId_1_3Chara && stage.song_step >= 700 && stage.song_step <= 704))
		this->character.focus_x = FIXED_DEC(85,1);
	else 
		this->character.focus_x = FIXED_DEC(65,1);
	//Animate and draw
	if (stage.utswap)
		Animatable_Animate(&character->animatableb, (void*)this, Char_sans_SetFrame);
	else
		Animatable_Animate(&character->animatable, (void*)this, Char_sans_SetFrame);

	Character_Draw(character, &this->tex, &char_sans_frame[this->frame]);
}

void Char_sans_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatableb, anim);
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
	Animatable_Init(&this->character.animatableb, char_sans_animb);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-103,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
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

		"idleb0.tim", //sans_ArcMain_Idle0
		"idleb1.tim", //sans_ArcMain_Idle1
		"leftb0.tim",  //sans_ArcMain_Left
		"leftb1.tim",  //sans_ArcMain_Left
		"downb.tim",  //sans_ArcMain_Down
		"upb0.tim",    //sans_ArcMain_Up
		"upb1.tim",    //sans_ArcMain_Up
		"rightb.tim", //sans_ArcMain_Right
		"specb0.tim", //sans_ArcMain_Right
		"specb1.tim", //sans_ArcMain_Right
		"specb2.tim", //sans_ArcMain_Right
		"specb3.tim", //sans_ArcMain_Right	

		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
