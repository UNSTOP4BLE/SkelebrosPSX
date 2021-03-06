/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfsans.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"
#include "../timer.h"

//Boyfriend skull fragments
static SkullFragment char_bfsans_skull[15] = {
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

//Boyfriend player types
enum
{
	BFSans_ArcMain_BFSans0,
	BFSans_ArcMain_BFSans1,
	BFSans_ArcMain_BFSans2,
	BFSans_ArcMain_BFSans3,
	BFSans_ArcMain_BFSans4,
	BFSans_ArcMain_BFSans5,
	BFSans_ArcMain_BFSans6,
	BFSans_ArcMain_BFSans7,
	BFSans_ArcMain_Dead0, //BREAK
	BFSans_ArcMain_BFBSans0,
	BFSans_ArcMain_BFBSans1,
	BFSans_ArcMain_BFBSans2,
	BFSans_ArcMain_BFBSans3,
	BFSans_ArcMain_BFBSans4,
	BFSans_ArcMain_BFBSans5,
	BFSans_ArcMain_BFBSans6,
	BFSans_ArcMain_BFBSans7,
	BFSans_ArcMain_BFBone0,
	BFSans_ArcMain_BFBone1,
	BFSans_ArcMain_BFBone2,
	BFSans_ArcMain_BFBone3,
	BFSans_ArcMain_BFBone4,
	BFSans_ArcMain_BFBone5,
	BFSans_ArcMain_BFBone6,
	BFSans_ArcMain_BFBoneb0,
	BFSans_ArcMain_BFBoneb1,
	BFSans_ArcMain_BFBoneb2,
	BFSans_ArcMain_BFBoneb3,
	BFSans_ArcMain_BFBoneb4,
	BFSans_ArcMain_BFBoneb5,
	BFSans_ArcMain_BFBoneb6,

	BFSans_ArcMain_Max,
};

enum
{
	BFSans_ArcDead_Dead1, //Mic Drop
	BFSans_ArcDead_Dead2, //Twitch
	BFSans_ArcDead_Retry, //Retry prompt
	
	BFSans_ArcDead_Max,
};

#define BFSans_Arc_Max BFSans_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFSans_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfsans_skull)];
	u8 skull_scale;
} Char_BFSans;

//Boyfriend player definitions
static const CharFrame char_bfsans_frame[] = {
	{BFSans_ArcMain_BFSans0, {  8,   0,  94,  99}, { 50,  93}}, //0 idle 1
	{BFSans_ArcMain_BFSans0, {114,   0,  91,  99}, { 48,  93}}, //1 idle 2
	{BFSans_ArcMain_BFSans0, { 12, 102,  91,  99}, { 47,  93}}, //2 idle 3
	{BFSans_ArcMain_BFSans0, {108, 101,  97, 103}, { 53,  97}}, //3 idle 4
	{BFSans_ArcMain_BFSans1, {  4,   1,  98, 103}, { 54,  97}}, //4 idle 5
	{BFSans_ArcMain_BFSans2, {  1,   4, 101, 103}, { 57,  97}}, //5 idle 6
	{BFSans_ArcMain_BFSans2, {106,   4, 101, 103}, { 56,  97}}, //6 idle 7
	{BFSans_ArcMain_BFSans2, {  1, 111, 100, 103}, { 56,  97}}, //7 idle 8
	{BFSans_ArcMain_BFSans2, {106, 111, 100, 103}, { 56,  97}}, //8 idle 9
	
	{BFSans_ArcMain_BFSans1, {105,   1,  96, 101}, { 58,  96}}, //9 left 1
	{BFSans_ArcMain_BFSans1, {  1, 107,  93, 101}, { 56,  95}}, //10 left 2
	{BFSans_ArcMain_BFSans1, {109, 107,  93, 101}, { 55,  95}}, //11 left 3
	{BFSans_ArcMain_BFSans3, {  2,   1,  93, 101}, { 55,  96}}, //12 left 4
	
	{BFSans_ArcMain_BFSans3, { 99,  13,  94,  89}, { 56,  84}}, //13 down 1
	{BFSans_ArcMain_BFSans3, {  1, 123,  93,  92}, { 54,  87}}, //14 down 2
	{BFSans_ArcMain_BFSans3, { 96, 119,  95,  96}, { 55,  91}}, //15 down 3
	{BFSans_ArcMain_BFSans4, {  2,   0,  94,  98}, { 55,  93}}, //16 down 4
	
	{BFSans_ArcMain_BFSans4, {115,   1,  92, 111}, { 44, 104}}, //17 up 1
	{BFSans_ArcMain_BFSans4, {  3, 100,  94, 111}, { 45, 104}}, //18 up 2
	{BFSans_ArcMain_BFSans5, {  5,   1,  96, 109}, { 46, 102}}, //19 up 3
	
	{BFSans_ArcMain_BFSans5, {105,   9, 102, 101}, { 47,  95}}, //20 right 1
	{BFSans_ArcMain_BFSans5, {  8, 115, 101, 101}, { 47,  96}}, //21 right 2
	{BFSans_ArcMain_BFSans5, {109, 115, 100, 101}, { 47,  96}}, //22 right 3
	
	{BFSans_ArcMain_BFSans6, {  0,   0,  93, 108}, { 52+3, 101+2}}, //23 left miss 1
	{BFSans_ArcMain_BFSans6, { 94,   0,  93, 108}, { 52+3, 101+2}}, //24 left miss 2
	
	{BFSans_ArcMain_BFSans6, {  0, 109,  95,  98}, { 50+3,  90+3}}, //25 down miss 1
	{BFSans_ArcMain_BFSans6, { 96, 109,  95,  97}, { 50+3,  89+3}}, //26 down miss 2
	
	{BFSans_ArcMain_BFSans7, {  0,   0,  90, 107}, { 44+3,  99+3}}, //27 up miss 1
	{BFSans_ArcMain_BFSans7, { 91,   0,  89, 108}, { 44+3, 100+3}}, //28 up miss 2
	
	{BFSans_ArcMain_BFSans7, {  0, 108,  99, 108}, { 43+3, 101+3}}, //29 right miss 1
	{BFSans_ArcMain_BFSans7, {100, 109, 101, 108}, { 43+3, 101+3}}, //30 right miss 2

	{BFSans_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //31 dead0 0
	{BFSans_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //32 dead0 1
	{BFSans_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //33 dead0 2
	{BFSans_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //34 dead0 3
	
	{BFSans_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //35 dead1 0
	{BFSans_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //36 dead1 1
	{BFSans_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //37 dead1 2
	{BFSans_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //38 dead1 3
	
	{BFSans_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //39 dead2 body twitch 0
	{BFSans_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //40 dead2 body twitch 1
	{BFSans_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //41 dead2 balls twitch 0
	{BFSans_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //42 dead2 balls twitch 1

	{BFSans_ArcMain_BFBSans0, {  8,   0,  94,  99}, { 50,  93}}, //0 idle 1
	{BFSans_ArcMain_BFBSans0, {114,   0,  91,  99}, { 48,  93}}, //1 idle 2
	{BFSans_ArcMain_BFBSans0, { 12, 102,  91,  99}, { 47,  93}}, //2 idle 3
	{BFSans_ArcMain_BFBSans0, {108, 101,  97, 103}, { 53,  97}}, //3 idle 4
	{BFSans_ArcMain_BFBSans1, {  4,   1,  98, 103}, { 54,  97}}, //4 idle 5
	{BFSans_ArcMain_BFBSans2, {  1,   4, 101, 103}, { 57,  97}}, //5 idle 6
	{BFSans_ArcMain_BFBSans2, {106,   4, 101, 103}, { 56,  97}}, //6 idle 7
	{BFSans_ArcMain_BFBSans2, {  1, 111, 100, 103}, { 56,  97}}, //7 idle 8
	{BFSans_ArcMain_BFBSans2, {106, 111, 100, 103}, { 56,  97}}, //8 idle 9
	
	{BFSans_ArcMain_BFBSans1, {105,   1,  96, 101}, { 58,  96}}, //52 left 1
	{BFSans_ArcMain_BFBSans1, {  1, 107,  93, 101}, { 56,  95}}, //10 left 2
	{BFSans_ArcMain_BFBSans1, {109, 107,  93, 101}, { 55,  95}}, //11 left 3
	{BFSans_ArcMain_BFBSans3, {  2,   1,  93, 101}, { 55,  96}}, //12 left 4
	
	{BFSans_ArcMain_BFBSans3, { 99,  13,  94,  89}, { 56,  84}}, //13 down 1
	{BFSans_ArcMain_BFBSans3, {  1, 123,  93,  92}, { 54,  87}}, //14 down 2
	{BFSans_ArcMain_BFBSans3, { 96, 119,  95,  96}, { 55,  91}}, //15 down 3
	{BFSans_ArcMain_BFBSans4, {  2,   0,  94,  98}, { 55,  93}}, //16 down 4
	
	{BFSans_ArcMain_BFBSans4, {115,   1,  92, 111}, { 44, 104}}, //17 up 1
	{BFSans_ArcMain_BFBSans4, {  3, 100,  94, 111}, { 45, 104}}, //18 up 2
	{BFSans_ArcMain_BFBSans5, {  5,   1,  96, 109}, { 46, 102}}, //19 up 3
	
	{BFSans_ArcMain_BFBSans5, {105,   9, 102, 101}, { 47,  95}}, //20 right 1
	{BFSans_ArcMain_BFBSans5, {  8, 115, 101, 101}, { 47,  96}}, //21 right 2
	{BFSans_ArcMain_BFBSans5, {109, 115, 100, 101}, { 47,  96}}, //65 right 3
	
	{BFSans_ArcMain_BFBSans6, {  0,   0,  93, 108}, { 52+3, 101+2}}, //66 left miss 1
	{BFSans_ArcMain_BFBSans6, { 94,   0,  93, 108}, { 52+3, 101+2}}, //24 left miss 2
	
	{BFSans_ArcMain_BFBSans6, {  0, 109,  95,  98}, { 50+3,  90+3}}, //25 down miss 1
	{BFSans_ArcMain_BFBSans6, { 96, 109,  95,  97}, { 50+3,  89+3}}, //26 down miss 2
	
	{BFSans_ArcMain_BFBSans7, {  0,   0,  90, 107}, { 44+3,  99+3}}, //27 up miss 1
	{BFSans_ArcMain_BFBSans7, { 91,   0,  89, 108}, { 44+3, 100+3}}, //28 up miss 2
	
	{BFSans_ArcMain_BFBSans7, {  0, 108,  99, 108}, { 43+3, 101+3}}, //29 right miss 1
	{BFSans_ArcMain_BFBSans7, {100, 109, 101, 108}, { 43+3, 101+3}}, //30 right miss 2

	//bonez               
	{BFSans_ArcMain_BFBone0, {  0, 0, 136, 144}, {110 - 50, 134 - 5}}, //74 right miss 1
	{BFSans_ArcMain_BFBone1, {  0, 0, 135, 144}, {108 - 50, 135 - 5}}, //75 right miss 2
	{BFSans_ArcMain_BFBone2, {  0, 0, 145, 146}, {109 - 50, 136 - 5}}, //76 right miss 1
	{BFSans_ArcMain_BFBone3, {  0, 0, 135, 144}, {109 - 50, 135 - 5}}, //77 right miss 2
	{BFSans_ArcMain_BFBone4, {  0, 0, 136, 145}, {109 - 50, 135 - 5}}, //78 right miss 1
	{BFSans_ArcMain_BFBone5, {  0, 0, 135, 144}, {109 - 50, 135 - 5}}, //79 right miss 2
	{BFSans_ArcMain_BFBone6, {  0, 0, 135, 144}, {108 - 50, 135 - 5}}, //80 right miss 1

	{BFSans_ArcMain_BFBoneb0, {  0, 0, 136, 144}, {110 - 50, 134 - 5}}, //74 right miss 1
	{BFSans_ArcMain_BFBoneb1, {  0, 0, 135, 144}, {108 - 50, 135 - 5}}, //75 right miss 2
	{BFSans_ArcMain_BFBoneb2, {  0, 0, 145, 146}, {109 - 50, 136 - 5}}, //76 right miss 1
	{BFSans_ArcMain_BFBoneb3, {  0, 0, 135, 144}, {109 - 50, 135 - 5}}, //77 right miss 2
	{BFSans_ArcMain_BFBoneb4, {  0, 0, 136, 145}, {109 - 50, 135 - 5}}, //78 right miss 1
	{BFSans_ArcMain_BFBoneb5, {  0, 0, 135, 144}, {109 - 50, 135 - 5}}, //79 right miss 2
	{BFSans_ArcMain_BFBoneb6, {  0, 0, 135, 144}, {108 - 50, 135 - 5}}, //80 right miss 1

};

static const Animation char_bfsans_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6,  7,  8, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 78, 79, 80, ASCR_BACK,  1}},       //CharAnim_Special
	{2, (const u8[]){ 9, 10, 11, 12, ASCR_BACK, 1}},             //CharAnim_Left
	{2, (const u8[]){74, 75, 76, 77, ASCR_CHGANI, CharAnim_Special}},       //CharAnim_LeftAlt
	{2, (const u8[]){13, 14, 15, 16, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){17, 18, 19, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){20, 21, 22, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 9, 23, 23, 24,  ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){13, 25, 25, 26,  ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){17, 27, 27, 28,  ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){20, 29, 29, 30,  ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},      //PlayerAnim_Sweat
	
	{5, (const u8[]){31, 32, 33, 34, 34, 34, 34, 34, 34, 34, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){34, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){35, 36, 37, 38, 38, 38, 38, 38, 38, 38, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){38, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){39, 40, 38, 38, 38, 38, 38, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){41, 42, 38, 38, 38, 38, 38, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){38, 38, 38, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){41, 42, 38, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

static const Animation char_bfsans_animb[PlayerAnim_Max] = {
	{2, (const u8[]){ 43,  44,  45,  46,  47,  48,  49,  50,  51, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){85, 86, 87, ASCR_BACK, 1}},       //CharAnim_Special
	{2, (const u8[]){ 52, 53, 54, 55, ASCR_BACK, 1}},             //CharAnim_Left
	{2, (const u8[]){81, 82, 83, 84, ASCR_CHGANI, CharAnim_Special}},       //CharAnim_LeftAlt
	{2, (const u8[]){56, 57, 58, 59, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){60, 61, 62, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){63, 64, 65, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){52, 66, 66, 67, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){56, 68, 68, 69, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){60, 70, 70, 71, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){63, 72, 72, 73, ASCR_BACK, 1}},     //PlayerAnim_RightMiss

	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},      //PlayerAnim_Sweat
	
	{5, (const u8[]){31, 32, 33, 34, 34, 34, 34, 34, 34, 34, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){34, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){35, 36, 37, 38, 38, 38, 38, 38, 38, 38, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){38, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){39, 40, 38, 38, 38, 38, 38, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){41, 42, 38, 38, 38, 38, 38, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){38, 38, 38, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){41, 42, 38, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BFSans_SetFrame(void *user, u8 frame)
{
	Char_BFSans *this = (Char_BFSans*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfsans_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void BFSans_CheckEndSing(Character *this)
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

void Char_BFSans_Tick(Character *character)
{
	Char_BFSans *this = (Char_BFSans*)character;
	
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
		BFSans_CheckEndSing(character);
	
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

	if (stage.utswap)
	{
		this->character.number_i = 3;
		this->character.swap_i = 2 + stage.song_step % 0x1;
		this->character.swapdeath_i = stage.song_step *2 % 0x1;
	}
	else     
	{
		this->character.number_i = 1;
		this->character.swap_i = stage.song_step % 0x1;
		this->character.swapdeath_i = stage.song_step *2 % 0x1;
	}
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFSans, skull); i++, frag++)
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
		Animatable_Animate(&character->animatableb, (void*)this, Char_BFSans_SetFrame);
	else
		Animatable_Animate(&character->animatable, (void*)this, Char_BFSans_SetFrame);

	if ((character->animatable.anim < PlayerAnim_LeftMiss && stage.state == StageState_Play) || (stage.state != StageState_Play) || (animf_count & 2))
	Character_Draw(character, &this->tex, &char_bfsans_frame[this->frame]);
}

void Char_BFSans_SetAnim(Character *character, u8 anim)
{
	Char_BFSans *this = (Char_BFSans*)character;
	
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
				"dead1.tim", //BFSans_ArcDead_Dead1
				"dead2.tim", //BFSans_ArcDead_Dead2
				"retry.tim", //BFSans_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFSans_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatableb, anim);
	Character_CheckStartSing(character);
}

void Char_BFSans_Free(Character *character)
{
	Char_BFSans *this = (Char_BFSans*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BFSans_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFSans *this = Mem_Alloc(sizeof(Char_BFSans));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFSans_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFSans_Tick;
	this->character.set_anim = Char_BFSans_SetAnim;
	this->character.free = Char_BFSans_Free;
	Animatable_Init(&this->character.animatable, char_bfsans_anim);
	Animatable_Init(&this->character.animatableb, char_bfsans_animb);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = stage.tex_bf;
	
	this->character.focus_x = FIXED_DEC(-45,1);
	this->character.focus_y = FIXED_DEC(-103,1);
	this->character.focus_zoom = FIXED_DEC(9,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFSANS.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf0.tim",   //BFSans_ArcMain_BFSans0
		"bf1.tim",   //BFSans_ArcMain_BFSans1
		"bf2.tim",   //BFSans_ArcMain_BFSans2
		"bf3.tim",   //BFSans_ArcMain_BFSans3
		"bf4.tim",   //BFSans_ArcMain_BFSans4
		"bf5.tim",   //BFSans_ArcMain_BFSans5
		"bf6.tim",   //BFSans_ArcMain_BFSans6
		"bf7.tim",   //BFSans_ArcMain_BFSans7
		"dead0.tim", //BFSans_ArcMain_Dead0
		"bfb0.tim",   //BFSans_ArcMain_BFSans0
		"bfb1.tim",   //BFSans_ArcMain_BFSans1
		"bfb2.tim",   //BFSans_ArcMain_BFSans2
		"bfb3.tim",   //BFSans_ArcMain_BFSans3
		"bfb4.tim",   //BFSans_ArcMain_BFSans4
		"bfb5.tim",   //BFSans_ArcMain_BFSans5
		"bfb6.tim",   //BFSans_ArcMain_BFSans6
		"bfb7.tim",   //BFSans_ArcMain_BFSans7
		"bone0.tim",  
		"bone1.tim", 
		"bone2.tim", 
		"bone3.tim", 
		"bone4.tim", 
		"bone5.tim",
		"bone6.tim",  
		"boneb0.tim",  
		"boneb1.tim", 
		"boneb2.tim", 
		"boneb3.tim", 
		"boneb4.tim", 
		"boneb5.tim",
		"boneb6.tim", 

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
	memcpy(this->skull, char_bfsans_skull, sizeof(char_bfsans_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFSans, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
