/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gold.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

int mog = 0;
int sog = 0;

//gold character structure
enum
{
	gold_ArcMain_Idle,
	gold_ArcMain_Left0,
	gold_ArcMain_Left1,
	gold_ArcMain_Down,
	gold_ArcMain_Up0,
	gold_ArcMain_Up1,
	gold_ArcMain_Right0,
	gold_ArcMain_Right1,

	//fade thing
	gold_ArcMain_Spec0,
	gold_ArcMain_Spec1,
	gold_ArcMain_Spec2,

	gold_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[gold_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_gold;

//gold character definitions
static const CharFrame char_gold_frame[] = {
	{gold_ArcMain_Idle, {  0,   0,  97, 146}, { 63, 146}}, //0 idle 1
	{gold_ArcMain_Idle, { 97,   0,  98, 146}, { 63, 146}}, //1 idle 2

	{gold_ArcMain_Left0, {  0,   0, 136, 128}, { 125 - 1, 128 - 1}}, //4 left 1
	{gold_ArcMain_Left0, {  0, 128, 136, 124}, { 125 - 4, 124 - 4}}, //5 left 2
	{gold_ArcMain_Left1, {  0,   0, 136, 121}, { 125 - 7, 121 - 7}},//4 left 1
	{gold_ArcMain_Left1, {  0, 121, 136, 123}, { 125 - 10, 123 - 10}}, //5 left 2
	
	{gold_ArcMain_Down, {  0,   0, 121, 107}, { 41, 107 - 15 - 1}}, //6 down 1
	{gold_ArcMain_Down, {121,   0, 124, 106}, { 40, 106 - 15 - 4}}, //7 down 2
	{gold_ArcMain_Down, {  0, 107, 125, 107}, { 42, 107 - 15 - 7}}, //6 down 1
	{gold_ArcMain_Down, {125, 106, 123, 108}, { 41, 108 - 15 - 10}}, //7 down 2
	
	{gold_ArcMain_Up0, {  0,   0, 102, 170}, { 62, 170}}, //8 up 1
	{gold_ArcMain_Up0, {102,   0, 108, 153}, { 64, 153}}, //9 up 2
	{gold_ArcMain_Up1, {  0,   0, 108, 160}, { 65, 160}}, //8 up 1
	{gold_ArcMain_Up1, {108,   0, 108, 160}, { 65, 160}}, //9 up 2

	{gold_ArcMain_Right0, {  0,   0, 136, 128}, { 32 + 1, 128 - 1}}, //10 right 1
	{gold_ArcMain_Right0, {  0, 128, 135, 125}, { 33 + 4, 125 - 4}}, //11 right 2
	{gold_ArcMain_Right1, {  0,   0, 137, 122}, { 34 + 7, 122 - 7}}, //10 right 1
	{gold_ArcMain_Right1, {  0, 122, 137, 122}, { 34 + 10, 122 - 10}}, //11 right 2

	//fade thing
	{gold_ArcMain_Spec0, {  0,   0, 98, 147}, { 63, 146}}, //11 right 2
	{gold_ArcMain_Spec0, { 99,   0, 96, 126}, { 63, 146}}, //11 right 2
	{gold_ArcMain_Spec0, {159, 124, 97, 132}, { 63, 146}}, //11 right 2
	{gold_ArcMain_Spec1, {  0,   0, 97, 136}, { 63, 146}}, //11 right 2
	{gold_ArcMain_Spec1, { 97,   0, 99, 140}, { 63, 146}}, //11 right 2
	{gold_ArcMain_Spec2, {  0,   0, 97, 144}, { 63, 146}}, //11 right 2
	{gold_ArcMain_Spec2, { 97,   0, 99, 144}, { 63, 146}}, //11 right 2

};

static const Animation char_gold_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Idle
	{2, (const u8[]){18,19,20,21,22,23,24,  ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Special
	{2, (const u8[]){ 2, 3, 4, 5, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 0}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 14, 15, 16, 17, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//gold character functions
void Char_gold_SetFrame(void *user, u8 frame)
{
	Char_gold *this = (Char_gold*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gold_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_gold_Tick(Character *character)
{
	Char_gold *this = (Char_gold*)character;

	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);

    if (stage.song_step == 6)
		character->set_anim(character, CharAnim_Special);
	
	//Animate and draw
	this->character.number_i = 3;
	this->character.swap_i = stage.song_step % 0x3;
	this->character.swapdeath_i = stage.song_step % 0x3;

	Animatable_Animate(&character->animatable, (void*)this, Char_gold_SetFrame);
	if (stage.song_step > 5)
	Character_Draw(character, &this->tex, &char_gold_frame[this->frame]);
}

void Char_gold_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_gold_Free(Character *character)
{
	Char_gold *this = (Char_gold*)character;

	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_gold_New(fixed_t x, fixed_t y)
{
	//Allocate gold object
	Char_gold *this = Mem_Alloc(sizeof(Char_gold));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_gold_New] Failed to allocate gold object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_gold_Tick;
	this->character.set_anim = Char_gold_SetAnim;
	this->character.free = Char_gold_Free;
	
	Animatable_Init(&this->character.animatable, char_gold_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = stage.tex_sans;
	
	this->character.focus_x = FIXED_DEC(-20,1);
	this->character.focus_y = FIXED_DEC(-70,1);
	this->character.focus_zoom = FIXED_DEC(14,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GOLD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //gold_ArcMain_Idle0
		"left0.tim",  //gold_ArcMain_Left
		"left1.tim",  //gold_ArcMain_Left
		"down.tim",  //gold_ArcMain_Down
		"up0.tim",    //gold_ArcMain_Up
		"up1.tim",    //gold_ArcMain_Up
		"right0.tim", //gold_ArcMain_Right
		"right1.tim", //gold_ArcMain_Right
		"spec0.tim", //gold_ArcMain_Right
		"spec1.tim", //gold_ArcMain_Right
		"spec2.tim", //gold_ArcMain_Right

		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
