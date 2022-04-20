/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "charaps.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"
#include "../timer.h"

//Boyfriend skull fragments
static SkullFragment char_charaps_skull[15] = {
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
    charaps_ArcMain_CH0,
	charaps_ArcMain_CH1,
	charaps_ArcMain_CH2,
	charaps_ArcMain_CH3,
	charaps_ArcMain_CH4,
	charaps_ArcMain_CH5,
    charaps_ArcMain_CHB0,
	charaps_ArcMain_CHB1,
	charaps_ArcMain_CHB2,
	charaps_ArcMain_CHB3,
    charaps_ArcMain_Bone0,
    charaps_ArcMain_Bone1,
    charaps_ArcMain_Bone2,
    charaps_ArcMain_Bone3,
    charaps_ArcMain_Bone4,
    charaps_ArcMain_Bone5,
    charaps_ArcMain_Bone6,
    charaps_ArcMain_Boneb0,
    charaps_ArcMain_Boneb1,
    charaps_ArcMain_Boneb2,
    charaps_ArcMain_Boneb3,
    charaps_ArcMain_Boneb4,
    charaps_ArcMain_Boneb5,
    charaps_ArcMain_Boneb6,
	charaps_ArcMain_Dead0, //BREAK

	charaps_ArcMain_Max,
};

enum
{
	charaps_ArcDead_Dead1, //Mic Drop
	charaps_ArcDead_Dead2, //Twitch
	charaps_ArcDead_Retry, //Retry prompt
	
	charaps_ArcDead_Max,
};

#define charaps_Arc_Max charaps_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[charaps_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_charaps_skull)];
	u8 skull_scale;
} Char_charaps;

//Boyfriend player definitions
static const CharFrame char_charaps_frame[] = {
    {charaps_ArcMain_CH0, {  0,   0,  66, 112}, { 36 - 25, 111}}, //9 left 1
    {charaps_ArcMain_CH0, { 66,   0,  66, 111}, { 37 - 25, 110}}, //9 left 1
    {charaps_ArcMain_CH0, {132,   0,  66, 116}, { 37 - 25, 115}}, //9 left 1
    {charaps_ArcMain_CH0, {  0, 112,  65, 115}, { 37 - 25, 114}}, //9 left 1
    {charaps_ArcMain_CH0, { 65, 111,  67, 115}, { 38 - 25, 114}}, //9 left 1

    {charaps_ArcMain_CH1, {125, 100,  89, 113}, { 72 - 25, 112}}, //9 left 1
    {charaps_ArcMain_CH2, {  0,   0,  96, 112}, { 76 - 25, 111}}, //9 left 1
    {charaps_ArcMain_CH2, { 96,   0,  93, 113}, { 76 - 25, 112}}, //9 left 1
    {charaps_ArcMain_CH2, {  0, 112,  93, 113}, { 76 - 25, 112}}, //9 left 1

    {charaps_ArcMain_CH1, {  0,   0, 131, 100}, { 68 - 25,   99}}, //9 left 1
    {charaps_ArcMain_CH0, {132, 116, 116, 100}, { 68 - 25,   99}}, //9 left 1
    {charaps_ArcMain_CH1, {131,   0, 119, 100}, { 68 - 25,   99}}, //9 left 1
    {charaps_ArcMain_CH1, {  0, 100, 125, 100}, { 69 - 25,   99}}, //9 left 1

    {charaps_ArcMain_CH3, {174,   0,  82, 105}, { 47 - 25, 105}}, //9 left 1
    {charaps_ArcMain_CH3, { 89, 106,  81, 105}, { 47 - 25, 104}}, //9 left 1
    {charaps_ArcMain_CH3, {170, 105,  82, 105}, { 47 - 25, 104}}, //9 left 1

    {charaps_ArcMain_CH2, { 93, 113,  90, 101}, { 45 - 25, 101}}, //9 left 1
    {charaps_ArcMain_CH3, {  0,   0,  87, 108}, { 47 - 25, 108}}, //9 left 1
    {charaps_ArcMain_CH3, { 87,   0,  87, 106}, { 46 - 25, 106}}, //9 left 1
    {charaps_ArcMain_CH3, {  0, 108,  89, 106}, { 47 - 25, 106}}, //9 left 1
	
	{charaps_ArcMain_CH5, {  0,   0,  80, 111}, { 62 - 25, 110}}, //23 left miss 1
	{charaps_ArcMain_CH5, { 80,   0,  77, 116}, { 60 - 25, 115}}, //24 left miss 2
	
	{charaps_ArcMain_CH4, {  0,   0, 116,  94}, { 64 - 25,  93}}, //25 down miss 1
	{charaps_ArcMain_CH4, {116,   0, 113, 101}, { 63 - 25, 100}}, //26 down miss 2
	
	{charaps_ArcMain_CH5, {  0, 116,  90, 104}, { 49 - 25, 103}}, //27 up miss 1
	{charaps_ArcMain_CH5, { 90, 116,  85, 107}, { 50 - 25, 106}}, //28 up miss 2
	
	{charaps_ArcMain_CH4, {  0, 101,  89, 102}, { 42 - 25, 102}}, //29 right miss 1
	{charaps_ArcMain_CH4, { 89, 101,  90,  98}, { 47 - 25,  98}}, //30 right miss 2
	
    //alt

    {charaps_ArcMain_CHB0, {  0,   0,  66, 112}, { 36 - 25, 111}}, //9 left 1
    {charaps_ArcMain_CHB0, { 66,   0,  66, 111}, { 37 - 25, 110}}, //9 left 1
    {charaps_ArcMain_CHB0, {132,   0,  66, 116}, { 36 - 25, 115}}, //9 left 1
    {charaps_ArcMain_CHB0, {  0, 112,  66, 115}, { 37 - 25, 114}}, //9 left 1
    {charaps_ArcMain_CHB0, { 66, 111,  66, 115}, { 36 - 25, 114}}, //9 left 1

    {charaps_ArcMain_CHB1, {125, 100,  89, 113}, { 72 - 25, 112}}, //9 left 1
    {charaps_ArcMain_CHB2, {  0,   0,  95, 112}, { 76 - 25, 111}}, //9 left 1
    {charaps_ArcMain_CHB2, { 96,   0,  93, 113}, { 76 - 25, 112}}, //9 left 1
    {charaps_ArcMain_CHB2, {  1, 112,  92, 113}, { 76 - 25, 112}}, //9 left 1

    {charaps_ArcMain_CHB1, {  0,   0, 130, 100}, { 68 - 25, 100}}, //9 left 1
    {charaps_ArcMain_CHB0, {132, 116, 117, 100}, { 69 - 25, 100}}, //9 left 1
    {charaps_ArcMain_CHB1, {131,   0, 119, 100}, { 69 - 25, 100}}, //9 left 1
    {charaps_ArcMain_CHB1, {  0, 100, 125, 100}, { 69 - 25, 100}}, //9 left 1

    {charaps_ArcMain_CHB3, {174,   0,  82, 105}, { 47 - 25, 105}}, //9 left 1
    {charaps_ArcMain_CHB3, { 89, 106,  81, 106}, { 46 - 25, 105}}, //9 left 1
    {charaps_ArcMain_CHB3, {171, 105,  81, 106}, { 46 - 25, 105}}, //9 left 1

    {charaps_ArcMain_CHB2, { 94, 114,  89, 100}, { 44 - 25, 100}}, //9 left 1
    {charaps_ArcMain_CHB3, {  0,   0,  86, 108}, { 46 - 25, 108}}, //9 left 1
    {charaps_ArcMain_CHB3, { 87,   0,  87, 106}, { 44 - 25, 106}}, //9 left 1
    {charaps_ArcMain_CHB3, {  0, 108,  89, 106}, { 45 - 25, 106}}, //9 left 1

    //bone

    {charaps_ArcMain_Bone0, {  0,   0, 135, 144}, { 61 - 25, 141}}, //9 left 1
    {charaps_ArcMain_Bone1, {  0,   1, 136, 143}, { 60 - 25, 141}}, //9 left 1
    {charaps_ArcMain_Bone2, {  1,   0, 144, 145}, { 60 - 25, 142}}, //9 left 1
    {charaps_ArcMain_Bone3, {  0,   1, 135, 143}, { 61 - 25, 141}}, //9 left 1
    {charaps_ArcMain_Bone4, {  0,   0, 135, 145}, { 61 - 25, 141}}, //9 left 1
    {charaps_ArcMain_Bone5, {  0,   0, 135, 144}, { 61 - 25, 140}}, //9 left 1
    {charaps_ArcMain_Bone6, {  0,   0, 140, 144}, { 61 - 25, 141}}, //9 left 1

    {charaps_ArcMain_Boneb0, {  5,   0, 127, 136}, { 56 - 25, 132}}, //9 left 1
    {charaps_ArcMain_Boneb1, {  6,   1, 127, 138}, { 53 - 25, 134}}, //9 left 1
    {charaps_ArcMain_Boneb2, {  3,   3, 143, 139}, { 53 - 25, 135}}, //9 left 1
    {charaps_ArcMain_Boneb3, {  4,   1, 127, 136}, { 54 - 25, 132}}, //9 left 1
    {charaps_ArcMain_Boneb4, {  3,   3, 130, 136}, { 55 - 25, 132}}, //9 left 1
    {charaps_ArcMain_Boneb5, {  2,   2, 134, 136}, { 55 - 25, 132}}, //9 left 1
    {charaps_ArcMain_Boneb6, {  1,   3, 139, 136}, { 55 - 25, 132}}, //9 left 1

    //ded 
	{charaps_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //31 dead0 0
	{charaps_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //32 dead0 1
	{charaps_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //33 dead0 2
	{charaps_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //34 dead0 3
	
	{charaps_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //35 dead1 0
	{charaps_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //36 dead1 1
	{charaps_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //37 dead1 2
	{charaps_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //38 dead1 3
	
	{charaps_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //39 dead2 body twitch 0
	{charaps_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //40 dead2 body twitch 1
	{charaps_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //41 dead2 balls twitch 0
	{charaps_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //42 dead2 balls twitch 1
};

static const Animation char_charaps_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 52, 53, 54, ASCR_BACK, 0}},       //CharAnim_Special
	{2, (const u8[]){ 5, 6, 7, 8, ASCR_BACK, 0}},             //CharAnim_Left
	{2, (const u8[]){ 48, 49, 50, 51, ASCR_CHGANI, CharAnim_Special}},       //CharAnim_LeftAlt
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
	
	{5, (const u8[]){31+31, 32+31, 33+31, 34+31, 34+31, 34+31, 34+31, 34+31, 34+31, 34+31, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){34+31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){35+31, 36+31, 37+31, 38+31, 38+31, 38+31, 38+31, 38+31, 38+31, 38+31, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){38+31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){39+31, 40+31, 38+31, 38+31, 38+31, 38+31, 38+31, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){41+31, 42+31, 38+31, 38+31, 38+31, 38+31, 38+31, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){38+31, 38+31, 38+31, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){41+31, 42+31, 38+31, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

static const Animation char_charaps_animb[PlayerAnim_Max] = {
	{2, (const u8[]){ 28,  29,  30,  31,  32, ASCR_BACK, 0}}, //CharAnim_Idle
	{2, (const u8[]){ 59, 60, 61, ASCR_BACK, 0}},       //CharAnim_Special
	{2, (const u8[]){ 33, 34, 35, 36, ASCR_BACK, 0}},             //CharAnim_Left
	{2, (const u8[]){ 55, 56, 57, 58, ASCR_CHGANI, CharAnim_Special}},       //CharAnim_LeftAlt
	{2, (const u8[]){37, 38, 39, 40, ASCR_BACK, 0}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){41, 42, 43, ASCR_BACK, 0}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){44, 45, 46, 47, ASCR_BACK, 0}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){33, 20, 20, 21, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){37, 22, 22, 23, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){41, 24, 24, 25, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){44, 26, 26, 27, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},     //PlayerAnim_Peace
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},     //PlayerAnim_Sweat
	
	{5, (const u8[]){31+31, 32+31, 33+31, 34+31, 34+31, 34+31, 34+31, 34+31, 34+31, 34+31, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){34+31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){35+31, 36+31, 37+31, 38+31, 38+31, 38+31, 38+31, 38+31, 38+31, 38+31, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){38+31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){39+31, 40+31, 38+31, 38+31, 38+31, 38+31, 38+31, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){41+31, 42+31, 38+31, 38+31, 38+31, 38+31, 38+31, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){38+31, 38+31, 38+31, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){41+31, 42+31, 38+31, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_charaps_SetFrame(void *user, u8 frame)
{
	Char_charaps *this = (Char_charaps*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_charaps_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void charaps_CheckEndSing(Character *this)
{
	if ((this->animatable.anim == CharAnim_Left ||
	     this->animatable.anim == CharAnim_LeftAlt ||
	     this->animatable.anim == CharAnim_Down ||
	     this->animatable.anim == CharAnim_DownAlt ||
	     this->animatable.anim == CharAnim_Up ||
	     this->animatable.anim == CharAnim_UpAlt ||
	     this->animatable.anim == CharAnim_Right ||
	     this->animatable.anim == CharAnim_RightAlt ||
	    ((this->spec & CHAR_SPEC_MISSANIM) &&
	    (this->animatable.anim == PlayerAnim_LeftMiss ||
	     this->animatable.anim == PlayerAnim_DownMiss ||
	     this->animatable.anim == PlayerAnim_UpMiss ||
	     this->animatable.anim == PlayerAnim_RightMiss))) &&
	    stage.note_scroll >= this->sing_end)
		this->set_anim(this, (bonesystem.bone == 1 && bonesystem.buttonpresscount != 10 ) ? CharAnim_Special : CharAnim_Idle);
}

void Char_charaps_Tick(Character *character)
{
	Char_charaps *this = (Char_charaps*)character;
	
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
		charaps_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if ((Animatable_Ended(&character->animatable) || Animatable_Ended(&character->animatableb)) &&
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
			character->set_anim(character, (bonesystem.bone == 1 && bonesystem.buttonpresscount != 10 ) ? CharAnim_Special : CharAnim_Idle);
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
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_charaps, skull); i++, frag++)
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
	if (stage.utswap)
		Animatable_Animate(&character->animatableb, (void*)this, Char_charaps_SetFrame);
	else
		Animatable_Animate(&character->animatable, (void*)this, Char_charaps_SetFrame);

	//if ((character->animatable.anim < PlayerAnim_LeftMiss && stage.state == StageState_Play) || (stage.state != StageState_Play) || (animf_count & 2))
	Character_Draw(character, &this->tex, &char_charaps_frame[this->frame]);
}

void Char_charaps_SetAnim(Character *character, u8 anim)
{
	Char_charaps *this = (Char_charaps*)character;
	
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
				"dead1.tim", //charaps_ArcDead_Dead1
				"dead2.tim", //charaps_ArcDead_Dead2
				"retry.tim", //charaps_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[charaps_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
    Animatable_SetAnim(&character->animatableb, anim);
	Character_CheckStartSing(character);
}

void Char_charaps_Free(Character *character)
{
	Char_charaps *this = (Char_charaps*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_charaps_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_charaps *this = Mem_Alloc(sizeof(Char_charaps));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_charaps_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_charaps_Tick;
	this->character.set_anim = Char_charaps_SetAnim;
	this->character.free = Char_charaps_Free;
	
	Animatable_Init(&this->character.animatable, char_charaps_anim);
    Animatable_Init(&this->character.animatableb, char_charaps_animb);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;

	this->character.health_i = stage.tex_bf;
	
	this->character.focus_x = FIXED_DEC(-45,1);
	this->character.focus_y = FIXED_DEC(-103,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CHARAPS.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"ch0.tim",   //BFSans_ArcMain_BFSans0
		"ch1.tim",   //BFSans_ArcMain_BFSans1
		"ch2.tim",   //BFSans_ArcMain_BFSans2
		"ch3.tim",   //BFSans_ArcMain_BFSans3
        "ch4.tim",   //BFSans_ArcMain_BFSans3
        "ch5.tim",   //BFSans_ArcMain_BFSans3
        "chb0.tim",   //BFSans_ArcMain_BFSans0
		"chb1.tim",   //BFSans_ArcMain_BFSans1
		"chb2.tim",   //BFSans_ArcMain_BFSans2
		"chb3.tim",   //BFSans_ArcMain_BFSans3
        "bone0.tim",   //BFSans_ArcMain_BFSans0
        "bone1.tim",   //BFSans_ArcMain_BFSans0
        "bone2.tim",   //BFSans_ArcMain_BFSans0
        "bone3.tim",   //BFSans_ArcMain_BFSans0
        "bone4.tim",   //BFSans_ArcMain_BFSans0
        "bone5.tim",   //BFSans_ArcMain_BFSans0
        "bone6.tim",   //BFSans_ArcMain_BFSans0
        "boneb0.tim",   //BFSans_ArcMain_BFSans0
        "boneb1.tim",   //BFSans_ArcMain_BFSans0
        "boneb2.tim",   //BFSans_ArcMain_BFSans0
        "boneb3.tim",   //BFSans_ArcMain_BFSans0
        "boneb4.tim",   //BFSans_ArcMain_BFSans0
        "boneb5.tim",   //BFSans_ArcMain_BFSans0
        "boneb6.tim",   //BFSans_ArcMain_BFSans0
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
	memcpy(this->skull, char_charaps_skull, sizeof(char_charaps_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_charaps, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
