/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "dad.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Dad_ArcMain_Idle0,
	Dad_ArcMain_Idle1,
	Dad_ArcMain_Idle2,
	Dad_ArcMain_Idle3,
	Dad_ArcMain_Idle4,
	Dad_ArcMain_Idle5,
	Dad_ArcMain_Left0,
	Dad_ArcMain_Left1,
	Dad_ArcMain_Left2,
	Dad_ArcMain_Left3,
	Dad_ArcMain_Down0,
	Dad_ArcMain_Down1,
	Dad_ArcMain_Down2,
	Dad_ArcMain_Down3,
	Dad_ArcMain_Up0,
	Dad_ArcMain_Up1,
	Dad_ArcMain_Up2,
	Dad_ArcMain_Up3,
	Dad_ArcMain_Right0,
	Dad_ArcMain_Right1,
	Dad_ArcMain_Right2,
	Dad_ArcMain_Right3,
	
	//alt
	Dad_ArcMain_Idleb0,
	Dad_ArcMain_Idleb1,
	Dad_ArcMain_Idleb2,
	Dad_ArcMain_Idleb3,
	Dad_ArcMain_Idleb4,
	Dad_ArcMain_Idleb5,
	Dad_ArcMain_Leftb0,
	Dad_ArcMain_Leftb1,
	Dad_ArcMain_Leftb2,
	Dad_ArcMain_Leftb3,
	Dad_ArcMain_Downb0,
	Dad_ArcMain_Downb1,
	Dad_ArcMain_Downb2,
	Dad_ArcMain_Downb3,
	Dad_ArcMain_Upb0,
	Dad_ArcMain_Upb1,
	Dad_ArcMain_Upb2,
	Dad_ArcMain_Upb3,
	Dad_ArcMain_Rightb0,
	Dad_ArcMain_Rightb1,
	Dad_ArcMain_Rightb2,
	Dad_ArcMain_Rightb3,

	Dad_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Dad_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Dad;

//Dad character definitions
static const CharFrame char_dad_frame[] = {
	{Dad_ArcMain_Idle0, {  0,   0, 191, 213}, {127, 212}}, //0 idle 1
	{Dad_ArcMain_Idle1, {  0,   0, 198, 209}, {135, 208}}, //1 idle 2
	{Dad_ArcMain_Idle2, {  0,   0, 189, 214}, {125, 213}}, //2 idle 3
	{Dad_ArcMain_Idle3, {  0,   0, 191, 215}, {124, 214}}, //3 idle 4
	{Dad_ArcMain_Idle4, {  0,   0, 188, 215}, {122, 214}}, //2 idle 3
	{Dad_ArcMain_Idle5, {  0,   0, 191, 215}, {124, 214}}, //3 idle 4
	
	{Dad_ArcMain_Left0, {  0,   0, 223, 193}, { 85, 192}}, //4 left 1
	{Dad_ArcMain_Left1, {  0,   0, 211, 197}, { 81, 196}}, //5 left 2
	{Dad_ArcMain_Left2, {  0,   0, 219, 194}, { 81, 193}}, //4 left 1
	{Dad_ArcMain_Left3, {  0,   0, 211, 194}, { 81, 193}}, //5 left 2
	
	{Dad_ArcMain_Down0, {  0,   0, 208, 209}, {118, 208}}, //6 down 1
	{Dad_ArcMain_Down1, {  0,   0, 208, 205}, {116, 205}}, //7 down 2
	{Dad_ArcMain_Down2, {  0,   0, 209, 209}, {120, 208}}, //6 down 1
	{Dad_ArcMain_Down3, {  0,   0, 206, 209}, {115, 208}}, //7 down 2
	
	{Dad_ArcMain_Up0, {  0,   0, 197, 233}, {133, 232}}, //8 up 1
	{Dad_ArcMain_Up1, {  0,   0, 195, 227}, {131, 226}}, //9 up 2
	{Dad_ArcMain_Up2, {  0,   0, 197, 230}, {133, 229}}, //8 up 1
	{Dad_ArcMain_Up3, {  0,   0, 194, 230}, {130, 229}}, //9 up 2
	
	{Dad_ArcMain_Right0, {  0,   0, 233, 231}, {114, 230}}, //10 right 1
	{Dad_ArcMain_Right1, {  0,   0, 244, 228}, {115, 227}}, //11 right 2
	{Dad_ArcMain_Right2, {  0,   0, 246, 230}, {114, 230}}, //10 right 1
	{Dad_ArcMain_Right3, {  0,   0, 236, 230}, {107, 230}}, //11 right 2
};

static const Animation char_dad_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 14, 15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){18, 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Dad character functions
void Char_Dad_SetFrame(void *user, u8 frame)
{
	Char_Dad *this = (Char_Dad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_dad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Dad_Tick(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Dad_SetFrame);
	Character_Draw(character, &this->tex, &char_dad_frame[this->frame]);
}

void Char_Dad_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Dad_Free(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Dad_New(fixed_t x, fixed_t y)
{
	//Allocate dad object
	Char_Dad *this = Mem_Alloc(sizeof(Char_Dad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Dad_New] Failed to allocate dad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Dad_Tick;
	this->character.set_anim = Char_Dad_SetAnim;
	this->character.free = Char_Dad_Free;
	
	Animatable_Init(&this->character.animatable, char_dad_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\DAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		"idle2.tim", //Dad_ArcMain_Idle1
		"idle3.tim", //Dad_ArcMain_Idle1
		"idle4.tim", //Dad_ArcMain_Idle1
		"idle5.tim", //Dad_ArcMain_Idle1
		"left0.tim",  //Dad_ArcMain_Left
		"left1.tim",  //Dad_ArcMain_Left
		"left2.tim",  //Dad_ArcMain_Left
		"left3.tim",  //Dad_ArcMain_Left
		"down0.tim",  //Dad_ArcMain_Down
		"down1.tim",  //Dad_ArcMain_Down
		"down2.tim",  //Dad_ArcMain_Down
		"down3.tim",  //Dad_ArcMain_Down
		"up0.tim",    //Dad_ArcMain_Up
		"up1.tim",    //Dad_ArcMain_Up
		"up2.tim",    //Dad_ArcMain_Up
		"up3.tim",    //Dad_ArcMain_Up
		"right0.tim", //Dad_ArcMain_Right
		"right1.tim", //Dad_ArcMain_Right
		"right2.tim", //Dad_ArcMain_Right
		"right3.tim", //Dad_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
