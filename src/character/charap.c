/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "charap.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"
#include "../timer.h"

//Boyfriend skull fragments
static SkullFragment char_charap_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

enum
{
    charap_ArcMain_CH0,
	charap_ArcMain_CH1,
	charap_ArcMain_CH2,
	charap_ArcMain_CH3,
	charap_ArcMain_CH4,
	charap_ArcMain_CH5,
	charap_ArcMain_Dead0, //BREAK

	charap_ArcMain_Max,
};

enum
{
	charap_ArcDead_Dead1, //Mic Drop
	charap_ArcDead_Dead2, //Twitch
	charap_ArcDead_Retry, //Retry prompt
	
	charap_ArcDead_Max,
};

#define charap_Arc_Max charap_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[charap_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_charap_skull)];
	u8 skull_scale;
} Char_charap;

//Boyfriend player definitions
static const CharFrame char_charap_frame[] = {
    {charap_ArcMain_CH0, {  0,   0,  66, 112}, { 36 - 25, 111}}, //9 left 1
    {charap_ArcMain_CH0, { 66,   0,  66, 111}, { 37 - 25, 110}}, //9 left 1
    {charap_ArcMain_CH0, {132,   0,  66, 116}, { 37 - 25, 115}}, //9 left 1
    {charap_ArcMain_CH0, {  0, 112,  65, 115}, { 37 - 25, 114}}, //9 left 1
    {charap_ArcMain_CH0, { 65, 111,  67, 115}, { 38 - 25, 114}}, //9 left 1

    {charap_ArcMain_CH1, {125, 100,  89, 113}, { 72 - 25, 112}}, //9 left 1
    {charap_ArcMain_CH2, {  0,   0,  96, 112}, { 76 - 25, 111}}, //9 left 1
    {charap_ArcMain_CH2, { 96,   0,  93, 113}, { 76 - 25, 112}}, //9 left 1
    {charap_ArcMain_CH2, {  0, 112,  93, 113}, { 76 - 25, 112}}, //9 left 1

    {charap_ArcMain_CH1, {  0,   0, 131, 100}, { 68 - 25,   99}}, //9 left 1
    {charap_ArcMain_CH0, {132, 116, 116, 100}, { 68 - 25,   99}}, //9 left 1
    {charap_ArcMain_CH1, {131,   0, 119, 100}, { 68 - 25,   99}}, //9 left 1
    {charap_ArcMain_CH1, {  0, 100, 125, 100}, { 69 - 25,   99}}, //9 left 1

    {charap_ArcMain_CH3, {174,   0,  82, 105}, { 47 - 25, 105}}, //9 left 1
    {charap_ArcMain_CH3, { 89, 106,  81, 105}, { 47 - 25, 104}}, //9 left 1
    {charap_ArcMain_CH3, {170, 105,  82, 105}, { 47 - 25, 104}}, //9 left 1

    {charap_ArcMain_CH2, { 93, 113,  90, 101}, { 45 - 25, 101}}, //9 left 1
    {charap_ArcMain_CH3, {  0,   0,  87, 108}, { 47 - 25, 108}}, //9 left 1
    {charap_ArcMain_CH3, { 87,   0,  87, 106}, { 46 - 25, 106}}, //9 left 1
    {charap_ArcMain_CH3, {  0, 108,  89, 106}, { 47 - 25, 106}}, //9 left 1
	
	{charap_ArcMain_CH5, {  0,   0,  80, 111}, { 62 - 25, 110}}, //23 left miss 1
	{charap_ArcMain_CH5, { 80,   0,  77, 116}, { 60 - 25, 115}}, //24 left miss 2
	
	{charap_ArcMain_CH4, {  0,   0, 116,  94}, { 64 - 25,  93}}, //25 down miss 1
	{charap_ArcMain_CH4, {116,   0, 113, 101}, { 63 - 25, 100}}, //26 down miss 2
	
	{charap_ArcMain_CH5, {  0, 116,  90, 104}, { 49 - 25, 103}}, //27 up miss 1
	{charap_ArcMain_CH5, { 90, 116,  85, 107}, { 50 - 25, 106}}, //28 up miss 2
	
	{charap_ArcMain_CH4, {  0, 101,  89, 102}, { 42 - 25, 102}}, //29 right miss 1
	{charap_ArcMain_CH4, { 89, 101,  90,  98}, { 47 - 25,  98}}, //30 right miss 2
	
	{charap_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //31 dead0 0
	{charap_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //32 dead0 1
	{charap_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //33 dead0 2
	{charap_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //34 dead0 3
	
	{charap_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //35 dead1 0
	{charap_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //36 dead1 1
	{charap_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //37 dead1 2
	{charap_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //38 dead1 3
	
	{charap_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //39 dead2 body twitch 0
	{charap_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //40 dead2 body twitch 1
	{charap_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //41 dead2 balls twitch 0
	{charap_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //42 dead2 balls twitch 1
};

static const Animation char_charap_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_BACK, 0}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_Special
	{2, (const u8[]){ 5, 6, 7, 8, ASCR_BACK, 0}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){9, 10, 11, 12, ASCR_BACK, 0}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 0}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){16, 17, 18, 19, ASCR_BACK, 0}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 5, 20, 20, 21, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 9, 22, 22, 23, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){13, 24, 24, 25, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){16, 26, 26, 27, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},     //PlayerAnim_Peace
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},     //PlayerAnim_Sweat
	
	{5, (const u8[]){31-4, 32-4, 33-4, 34-4, 34-4, 34-4, 34-4, 34-4, 34-4, 34-4, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){34-4, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){35-4, 36-4, 37-4, 38-4, 38-4, 38-4, 38-4, 38-4, 38-4, 38-4, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){38-4, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){39-4, 40-4, 38-4, 38-4, 38-4, 38-4, 38-4, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){41-4, 42-4, 38-4, 38-4, 38-4, 38-4, 38-4, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){38-4, 38-4, 38-4, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){41-4, 42-4, 38-4, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_charap_SetFrame(void *user, u8 frame)
{
	Char_charap *this = (Char_charap*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_charap_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_charap_Tick(Character *character)
{
	Char_charap *this = (Char_charap*)character;
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
    
	this->character.number_i = 1;
	this->character.swap_i = stage.song_step % 0x1;
	this->character.swapdeath_i = stage.song_step *2 % 0x1;

	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_charap, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_charap_SetFrame);
	//if ((character->animatable.anim < PlayerAnim_LeftMiss && stage.state == StageState_Play) || (stage.state != StageState_Play) || (animf_count & 2))
	Character_Draw(character, &this->tex, &char_charap_frame[this->frame]);
}

void Char_charap_SetAnim(Character *character, u8 anim)
{
	Char_charap *this = (Char_charap*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;

			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //charap_ArcDead_Dead1
				"dead2.tim", //charap_ArcDead_Dead2
				"retry.tim", //charap_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[charap_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_charap_Free(Character *character)
{
	Char_charap *this = (Char_charap*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_charap_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_charap *this = Mem_Alloc(sizeof(Char_charap));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_charap_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_charap_Tick;
	this->character.set_anim = Char_charap_SetAnim;
	this->character.free = Char_charap_Free;
	
	Animatable_Init(&this->character.animatable, char_charap_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;

	this->character.health_i = stage.tex_bf;
	
	this->character.focus_x = FIXED_DEC(-45,1);
	this->character.focus_y =  FIXED_DEC(-85,1);
	this->character.focus_zoom = FIXED_DEC(9,10);

	//Load art
	this->arc_main = IO_Read("\\CHAR\\CHARAP.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"ch0.tim",   //BFSans_ArcMain_BFSans0
		"ch1.tim",   //BFSans_ArcMain_BFSans1
		"ch2.tim",   //BFSans_ArcMain_BFSans2
		"ch3.tim",   //BFSans_ArcMain_BFSans3
        "ch4.tim",   //BFSans_ArcMain_BFSans3
        "ch5.tim",   //BFSans_ArcMain_BFSans3
		"dead0.tim", //BFSans_ArcMain_Dead0

		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);

	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_charap_skull, sizeof(char_charap_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_charap, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
