/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "menu.h"

#include "mem.h"
#include "main.h"
#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "archive.h"
#include "mutil.h"
#include "network.h"

#include "movie.h"

#include "font.h"
#include "trans.h"
#include "loadscr.h"

#include "stage.h"
#include "character/playerm.h"
#include "character/spm.h"

u32 moveuptitle;
//menu characters variable
s16 transparency,moving;
u16 movingposi = 9;
boolean check,checkchar = false;

//Menu messages
static const char *funny_messages[][2] = {
	{"PSX PORT BY CUCKYDEV", "YOU KNOW IT"},
	{"PORTED BY 2 GUYS", "WHAT YOU GONNA DO"},
	{"FUNKIN", "FOREVER"},
	{"WHAT THE HELL", "SKELETON PSX"},
	{"LIKE PARAPPA", "BUT COOLER"},
	{"THE JAPI", "EL JAPI"},
	{"EL CUTSCENES", "BABY"},
	{"OPENGL BACKEND", "BY CLOWNACY"},
	{"CUCKYFNF", "SETTING STANDARDS"},
	{"lool", "inverted colours"},
	{"NEVER LOOK AT", "THE ISSUE TRACKER"},
	{"PSXDEV", "HOMEBREW"},
	{"ZERO POINT ZERO TWO TWO EIGHT", "ONE FIVE NINE ONE ZERO FIVE"},
	{"DOPE ASS GAME", "PLAYSTATION MAGAZINE"},
	{"NO FPU", "NO PROBLEM"},
	{"OK OKAY", "WATCH THIS"},
	{"ITS MORE MALICIOUS", "THAN ANYTHING"},
	{"USE A CONTROLLER", "LOL"},
	{"POSSUMS", "ARE REALLY CUTE"},
	{"SHITS UNOFFICIAL", "NOT A PROBLEM"},
	{"SYSCLK", "RANDOM SEED"},
	{"THEY DIDNT HIT THE GOAL", "STOP"},
	{"SUS", "MOGUS"},
	{"EL", "JAPI"},
	{"PSXFUNKIN BY CUCKYDEV", "SUCK IT DOWN"},
	{"PLAYING ON EPSXE HUH", "YOURE THE PROBLEM"},
	{"NEXT IN LINE", "SEGA"},
	{"HAHAHA", "I DONT CARE"},
	{"GET ME TO STOP", "TRY"},
	{"FRANS", "STOP"},
	{"OPEN SOURCE", "FOREVER"},
	{"WOW GATO", "WOW GATO"},
	{"BALLS FISH", "FAIL FISH"},
};

#ifdef PSXF_NETWORK

//Menu string type
#define MENUSTR_CHARS 0x20
typedef char MenuStr[MENUSTR_CHARS + 1];

#endif

//Menu state
static struct
{
	//Menu state
	u8 page, next_page;
	boolean page_swap;
	u8 select, next_select,skip_selectup,skip_selectdown;
	
	fixed_t scroll;
	fixed_t trans_time;
	
	//Page specific state
	union
	{
		struct
		{
			u8 funny_message;
		} opening;
		struct
		{
			fixed_t logo_bump;
			fixed_t fade, fadespd;
		} title;
		struct
		{
			fixed_t fade, fadespd;
		} story;
		struct
		{
			fixed_t back_r, back_g, back_b;
		} freeplay;
        struct
		{
			fixed_t back_r, back_g, back_b;
		} credit;
	#ifdef PSXF_NETWORK
		struct
		{
			boolean type;
			MenuStr port;
			MenuStr pass;
		} net_host;
		struct
		{
			boolean type;
			MenuStr ip;
			MenuStr port;
			MenuStr pass;
		} net_join;
		struct
		{
			boolean swap;
		} net_op;
	#endif
	} page_state;
	
	union
	{
		struct
		{
			u8 id, diff;
			boolean story;
		} stage;
	} page_param;
	
	//Menu assets
	Gfx_Tex tex_back,tex_cooleffect, tex_ng, tex_story, tex_storybg, tex_crediti, tex_title, tex_titlebg, tex_names;
	FontData font_bold,font_arial;
	
	Character *playerm; //Title Girlfriend
	Character *spm; //Title Girlfriend
} menu;

//Menu Chaacter
static struct
{
	//Menu Character assets
	Gfx_Tex tex_menuchar0,tex_menuchar1, tex_menuchar2, tex_menuchar3;
	Gfx_Tex swap;
	
} menuchar;

#ifdef PSXF_NETWORK

//Menu string functions
static void MenuStr_Process(MenuStr *this, s32 x, s32 y, const char *fmt, boolean select, boolean type)
{
	//Append typed input
	if (select && type)
	{
		if (pad_type[0] != '\0')
			strncat(*this, pad_type, MENUSTR_CHARS - strlen(*this));
		if (pad_backspace)
		{
			size_t i = strlen(*this);
			if (i != 0)
				(*this)[i - 1] = '\0';
		}
	}
	
	//Get text to draw
	char buf[0x100];
	sprintf(buf, fmt, *this);
	if (select && type && (animf_count & 2))
		strcat(buf, "_");
	
	//Draw text
	menu.font_arial.draw_col(&menu.font_arial, buf, x, y, FontAlign_Left, 0x80, 0x80, select ? 0x00 : 0x80);
	menu.font_arial.draw_col(&menu.font_arial, buf, x+1, y+1, FontAlign_Left, 0x00, 0x00, 0x00);
}

#endif


//Internal menu functions
char menu_text_buffer[0x100];

static const char *Menu_LowerIf(const char *text, boolean lower)
{
	//Copy text
	char *dstp = menu_text_buffer;
	if (lower)
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'A' && *srcp <= 'Z')
				*dstp++ = *srcp | 0x20;
			else
				*dstp++ = *srcp;
		}
	}
	else
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'a' && *srcp <= 'z')
				*dstp++ = *srcp & ~0x20;
			else
				*dstp++ = *srcp;
		}
	}
	
	//Terminate text
	*dstp++ = '\0';
	return menu_text_buffer;
}
static void Credits_DrawImage(u8 i, s32 x,s32 y)
{
RECT src = {
		(i % 4) * 64,
	    (i / 4) * 64,
	    64,
		64,
	};
	RECT dst = {
		x,
		y,
		22,
		22,
	};
	
	//Draw credit image
	Gfx_DrawTex(&menu.tex_crediti, &src, &dst);
}
static void Menu_DrawBack(boolean flash, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
	RECT back_src = {0, 0, 255, 255};
	RECT back_dst = {0, 0, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
	
		if (flash || (animf_count & 4) == 0)
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r0, g0, b0);
	else
		Gfx_DrawTexCol(&menu.tex_back, &back_src, &back_dst, r1, g1, b1);
}

static void Menu_DifficultySelector(s32 x, s32 y)
{
	//Change difficulty
	if (menu.next_page == menu.page && Trans_Idle())
	{
		if (pad_state.press & PAD_LEFT)
		{
			if (menu.page_param.stage.diff > StageDiff_BF)
				menu.page_param.stage.diff--;
			else
				menu.page_param.stage.diff = StageDiff_Chara;
		}
		if (pad_state.press & PAD_RIGHT)
		{
			if (menu.page_param.stage.diff < StageDiff_Chara)
				menu.page_param.stage.diff++;
			else
				menu.page_param.stage.diff = StageDiff_BF;
		}
		stage.strdiff = menu.page_param.stage.diff;
	}
	
	//Draw difficulty arrows
	static const RECT arrow_src[2][2] = {
		{{224, 64, 16, 32}, {224, 96, 16, 32}}, //left
		{{240, 64, 16, 32}, {240, 96, 16, 32}}, //right
	};
	
	Gfx_BlitTex(&menu.tex_story, &arrow_src[0][(pad_state.held & PAD_LEFT) != 0], x - 50 - 16, y - 13);
	Gfx_BlitTex(&menu.tex_story, &arrow_src[1][(pad_state.held & PAD_RIGHT) != 0], x + 50, y - 13);
	
	//Draw difficulty
	static const RECT diff_srcs[] = {
		{ 86, 92, 33, 35},
		{  0, 63,100, 25},
	};
	
	const RECT *diff_src = &diff_srcs[menu.page_param.stage.diff];
	Gfx_BlitTex(&menu.tex_story, diff_src, x - (diff_src->w >> 1), y - 9 + ((pad_state.press & (PAD_LEFT | PAD_RIGHT)) != 0));
}

static void Menu_DrawWeek(const char *week, s32 x, s32 y)
{
	//Draw label
	if (week == NULL)
	{
		//Tutorial
		RECT label_src = {0, 0, 120, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
	}
	else
	{
		//Week
		RECT label_src = {0, 32, 80, 32};
		Gfx_BlitTex(&menu.tex_story, &label_src, x, y);
		
		//Number
		x += 80;
		for (; *week != '\0'; week++)
		{
			//Draw number
			u8 i = *week - '0';
			
			RECT num_src = {128 + ((i & 3) << 5), ((i >> 2) << 5), 32, 32};
			Gfx_BlitTex(&menu.tex_story, &num_src, x, y);
			x += 32;
		}
	}
}

//Menu functions
void Menu_Load(MenuPage page)
{
	//Load menu assets
	IO_Data menu_arc = IO_Read("\\MENU\\MENU.ARC;1");
	Gfx_LoadTex(&menu.tex_back,  Archive_Find(menu_arc, "back.tim"),  0);
	Gfx_LoadTex(&menu.tex_cooleffect,  Archive_Find(menu_arc, "coolfx.tim"),  0);
	Gfx_LoadTex(&menu.tex_story, Archive_Find(menu_arc, "story.tim"), 0);
	Gfx_LoadTex(&menu.tex_storybg, Archive_Find(menu_arc, "bgstory.tim"), 0);
	Gfx_LoadTex(&menu.tex_crediti, Archive_Find(menu_arc, "crediti.tim"), 0);
	Gfx_LoadTex(&menu.tex_title, Archive_Find(menu_arc, "title.tim"), 0);
	Gfx_LoadTex(&menu.tex_titlebg, Archive_Find(menu_arc, "titlebg.tim"), 0);
	Gfx_LoadTex(&menu.tex_names, Archive_Find(menu_arc, "boldfona.tim"), 0);
	Mem_Free(menu_arc);
	
	FontData_Load(&menu.font_bold, Font_Bold);
	FontData_Load(&menu.font_arial, Font_Arial);

	//Load menu assets
	IO_Data menuchar_arc = IO_Read("\\MENUCHAR\\MCHAR.ARC;1");
	Gfx_LoadTex(&menuchar.tex_menuchar0,  Archive_Find(menuchar_arc, "mchar0.tim"),  0);
	Gfx_LoadTex(&menuchar.tex_menuchar1,  Archive_Find(menuchar_arc, "mchar1.tim"),  0);
	Gfx_LoadTex(&menuchar.tex_menuchar2,  Archive_Find(menuchar_arc, "mchar2.tim"),    0);
	Gfx_LoadTex(&menuchar.tex_menuchar3,  Archive_Find(menuchar_arc, "mchar3.tim"), 0);
	Mem_Free(menuchar_arc);
	
	menu.playerm = Char_PlayerM_New(FIXED_DEC(72,1), FIXED_DEC(9,1));
	menu.spm = Char_SPM_New(FIXED_DEC(-82,1), FIXED_DEC(17,1));
	stage.camera.x = stage.camera.y = FIXED_DEC(0,1);
	stage.camera.bzoom = FIXED_UNIT;
	
	//Initialize menu state
	menu.select = menu.skip_selectup = menu.skip_selectdown = menu.next_select = 0;
	
	switch (menu.page = menu.next_page = page)
	{
		case MenuPage_Opening:
			//Get funny message to use
			//Do this here so timing is less reliant on VSync
			#ifdef PSXF_PC
				menu.page_state.opening.funny_message = time(NULL) % COUNT_OF(funny_messages);
			#else
				menu.page_state.opening.funny_message = ((*((volatile u32*)0xBF801120)) >> 3) % COUNT_OF(funny_messages); //sysclk seeding
			#endif
			break;
		default:
			break;
	}
	menu.page_swap = true;
	
	menu.trans_time = 0;
	Trans_Clear();
	
	stage.song_step = 0;
	
	//Play menu music
	Audio_PlayXA_Track(XA_GettinFreaky, 0x40, 0, 1);
	Audio_WaitPlayXA();
	
	//Set background colour
	Gfx_SetClear(0,  0,  0);
}

void Menu_Unload(void)
{
	//Free title Girlfriend
	Character_Free(menu.playerm);
	Character_Free(menu.spm);
}

void Menu_ToStage(StageId id, StageDiff diff, boolean story)
{
	menu.next_page = MenuPage_Stage;
	menu.page_param.stage.id = id;
	menu.page_param.stage.story = story;
	menu.page_param.stage.diff = diff;
	Trans_Start();
}

void Menu_Tick(void)
{
	//Clear per-frame flags
	stage.flag &= ~STAGE_FLAG_JUST_STEP;
	
	//Get song position
	u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
	if (next_step != stage.song_step)
	{
		if (next_step >= stage.song_step)
			stage.flag |= STAGE_FLAG_JUST_STEP;
		stage.song_step = next_step;
	}
	
	//Handle transition out
	if (Trans_Tick())
	{
		//Change to set next page
		menu.page_swap = true;
		menu.page = menu.next_page;
		menu.select = menu.next_select;
	}
	
	//Tick menu page
	MenuPage exec_page;
	switch (exec_page = menu.page)
	{
		case MenuPage_Opening:
		{
			u16 beat = stage.song_step >> 2;
			
			//Start title screen if opening ended
			if (beat >= 16)
			{
				menu.page = menu.next_page = MenuPage_Title;
				menu.page_swap = true;
				//Fallthrough
			}
			else
			{
				//Start title screen if start pressed
				if (pad_state.held & PAD_START)
					menu.page = menu.next_page = MenuPage_Title;
				
				//Draw different text depending on beat
				const char **funny_message = funny_messages[menu.page_state.opening.funny_message];
				
				switch (beat)
				{
					case 3:
						menu.font_bold.draw(&menu.font_bold, "PRESENT", SCREEN_WIDTH2, SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 2:
					case 1:
						menu.font_bold.draw(&menu.font_bold, "IGORSOU",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "UNSTOPABLE", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 7:
						menu.font_bold.draw(&menu.font_bold, "NEWGROUNDS",    SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
				//Fallthrough
					case 6:
					case 5:
						menu.font_bold.draw(&menu.font_bold, "NOT ASSOCIATED", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						menu.font_bold.draw(&menu.font_bold, "WITH",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						break;
					
					case 11:
						menu.font_bold.draw(&menu.font_bold, funny_message[1], SCREEN_WIDTH2, SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 10:
					case 9:
						menu.font_bold.draw(&menu.font_bold, funny_message[0], SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 15:
						menu.font_bold.draw(&menu.font_bold, "FUNKIN", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 8, FontAlign_Center);
				//Fallthrough
					case 14:
						menu.font_bold.draw(&menu.font_bold, "NIGHT", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 8, FontAlign_Center);
				//Fallthrough
					case 13:
						menu.font_bold.draw(&menu.font_bold, "FRIDAY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 24, FontAlign_Center);
						break;
				}
				break;
			}
		}
	//Fallthrough
		case MenuPage_Title:
		{  
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(90,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Go to main menu when start is pressed
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
		      Trans_Start();
			
			if ((pad_state.press & PAD_START) && menu.next_page == menu.page && Trans_Idle())
			{
				menu.trans_time = FIXED_UNIT;
				menu.page_state.title.fade = FIXED_DEC(255,1);
				menu.page_state.title.fadespd = FIXED_DEC(300,1);
				menu.next_page = MenuPage_Main;
				menu.next_select = 0;
			}
			
			//Draw Friday Night Funkin' logo
			if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0x3) == 0 && menu.page_state.title.logo_bump == 0)
				menu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
			
			static const fixed_t logo_scales[] = {
				FIXED_DEC(1,1),
				FIXED_DEC(101,100),
				FIXED_DEC(102,100),
				FIXED_DEC(103,100),
				FIXED_DEC(105,100),
				FIXED_DEC(110,100),
				FIXED_DEC(97,100),
			};
			fixed_t logo_scale = logo_scales[(menu.page_state.title.logo_bump * 24) >> FIXED_SHIFT];
			u32 x_rad = (logo_scale * (192 >> 1)) >> FIXED_SHIFT;
			u32 y_rad = (logo_scale * (116 >> 1)) >> FIXED_SHIFT;
			
			RECT logo_src = {0, 0, 192, 116};
			RECT logo_dst = {
				160 - x_rad + (SCREEN_WIDEADD2 >> 1),
				98 - moveuptitle - y_rad,
				x_rad << 1,
				y_rad << 1
			};
			Gfx_DrawTex(&menu.tex_title, &logo_src, &logo_dst);
			
			if (menu.page_state.title.logo_bump > 0)
				if ((menu.page_state.title.logo_bump -= timer_dt) < 0)
					menu.page_state.title.logo_bump = 0;
			
			//Draw "Press Start to Begin"
			if (menu.next_page == menu.page)
			{
				//Blinking blue
				s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
				u8 press_r = 51 >> 1;
				u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
				u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
				
				RECT press_src = {0, 117, 256, 32};
				Gfx_BlitTexCol(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48, press_r, press_g, press_b);
			}
			else
			{
				//Flash white
				moveuptitle += 3;
				RECT press_src = {0, (animf_count & 1) ? 149 : 117, 256, 32};
				Gfx_BlitTex(&menu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48);
			}
            //draw titlebg
			RECT titlebg_src = {0, 0, 256, 144};
	        RECT titlebg_dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	
		    Gfx_DrawTex(&menu.tex_titlebg, &titlebg_src, &titlebg_dst);
			
			//Draw Girlfriend
			break;
		}
		case MenuPage_Main:
		{
			moveuptitle = 0;

			static const char *menu_options[] = {
				"STORY MODE",
				"FREEPLAY",
				"CREDITS",
				"OPTIONS",
				#ifdef PSXF_NETWORK
					"JOIN SERVER",
					"HOST SERVER",
				#endif
			};
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = menu.select *
				#ifndef PSXF_NETWORK
					FIXED_DEC(8,1);
				#else
					FIXED_DEC(12,1);
				#endif
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					check = true;
					checkchar = true;
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					check = true;
					checkchar = true;
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					switch (menu.select)
					{
						case 0: //Story Mode
							menu.next_page = MenuPage_Story;
							break;
						case 1: //Freeplay
							menu.next_page = MenuPage_Freeplay;
							break;
						case 2: //Mods
							menu.next_page = MenuPage_Credits;
							break;
						case 3: //Options
							menu.next_page = MenuPage_Options;
							break;
					#ifdef PSXF_NETWORK
						case 4: //Join Server
							menu.next_page = Network_Inited() ? MenuPage_NetJoin : MenuPage_NetInitFail;
							break;
						case 5: //Host Server
							menu.next_page = Network_Inited() ? MenuPage_NetHost : MenuPage_NetInitFail;
							break;
					#endif
					}
					menu.next_select = 0;
					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to title screen if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Title;
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select *
			#ifndef PSXF_NETWORK
				FIXED_DEC(8,1);
			#else
				FIXED_DEC(12,1);
			#endif
			menu.scroll += (next_scroll - menu.scroll) >> 2;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Title)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					menu.font_bold.draw(&menu.font_bold,
						Menu_LowerIf(menu_options[i], menu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i << 5) - 48,
						FontAlign_Center
					);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				menu.font_bold.draw(&menu.font_bold,
					menu_options[menu.select],
					SCREEN_WIDTH2,
					SCREEN_HEIGHT2 + (menu.select << 5) - 48,
					FontAlign_Center
				);
			}

			RECT coolfx_src = {0, 0, 255, 255};
	        RECT coolfx_dst = {0, 0, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
	
		    Gfx_DrawTex(&menu.tex_cooleffect, &coolfx_src, &coolfx_dst);
			
			//there are definitely better ways to do this but hey it works
			
            //starting move character
			if (check == true)	 
			 moving += movingposi;

            //make transparency be 50%
			if (moving >= 20)
			transparency = 1;

             //make transparency be 100%
			if (moving < 20)
			transparency = 0;

            //allow character swap and move character to the left
			if (moving >= 80)
			{
			checkchar = false;
			movingposi = -9;
			}

			 //reset everything
            if (moving < 0)
			{
			 movingposi = 9;
             moving = 0;
			 check = false;
			}
			 //debug thing
			//FntPrint("moving %d", moving);
            
			switch(menu.select)
			{
			 case 0:
			 if (checkchar == false)
			 menuchar.swap = menuchar.tex_menuchar0;
			 break;
			 case 1:
			 if (checkchar == false)
			 menuchar.swap = menuchar.tex_menuchar1;
			 break;
			 case 2:
			 if (checkchar == false)
			 menuchar.swap = menuchar.tex_menuchar2;
			 break;
			 case 3:
			 if (checkchar == false)
			 menuchar.swap = menuchar.tex_menuchar3;
			 break;

			}
			RECT Char = {0, 0,180, 180};
			RECT CharDst = {moving, 0, 210, 240};
            
			if (moving <= 55)
			Gfx_BlendTex(&menuchar.swap, &Char, &CharDst,transparency);
			
			//Draw background
			Menu_DrawBack(
				menu.next_page == menu.page || menu.next_page == MenuPage_Title,
				253 >> 1, 231 >> 1, 113 >> 1,
				253 >> 1, 231 >> 1, 113 >> 1
			);
			break;
		}
		case MenuPage_Story:
		{
			//swap bg
			u16 swapbg;
			swapbg = 0;
			//Draw Playerm
			menu.playerm->tick(menu.playerm);
			//Draw SPM
			menu.spm->tick(menu.spm);

			static const struct
			{
				const char *week;
				StageId stage;
				StageId stagealt;
				const char *name;
				const char *tracks[3];
			} menu_options[] = {
				{NULL, StageId_1_1,StageId_1_1Chara,"UNDERTALE UNIVERSE", {"NYEH HEH HEH", "BONELY ONE", "NOT ENOUGH"}},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = 0;
				menu.page_param.stage.diff = StageDiff_BF;
				menu.page_state.title.fade = FIXED_DEC(0,1);
				menu.page_state.title.fadespd = FIXED_DEC(0,1);
			}
			
			//Draw white fade
			if (menu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = menu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				menu.page_state.title.fade -= FIXED_MUL(menu.page_state.title.fadespd, timer_dt);
			}
			
			//Draw difficulty selector
			Menu_DifficultySelector(SCREEN_WIDTH - 75, 160);
			
			//Handle option and selection
			if (menu.trans_time > 0 && (menu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (menu.next_page == menu.page && Trans_Idle())
			{
			/*	//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				*/

				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{	
					if (menu.page_param.stage.diff != StageDiff_Chara)										
					menu.next_page = MenuPage_Movie;
					else
					menu.next_page = MenuPage_Stage;

					if (menu.page_param.stage.diff == StageDiff_Chara)
	                menu.page_param.stage.id = menu_options[menu.select].stagealt;
					else
					menu.page_param.stage.id = menu_options[menu.select].stage;

					menu.playerm->set_anim(menu.playerm, CharAnim_Special);
					menu.page_param.stage.story = true;
                    
					//movie stuff
					movie.select = 0;
					movie.diff = menu.page_param.stage.diff;
					movie.id = menu.page_param.stage.id;
					movie.story = menu.page_param.stage.story;
					
					if (menu.page_param.stage.diff != StageDiff_Chara)	
					movie.startmovie = true;
					else
			    	movie.startmovie = false;

					menu.trans_time = FIXED_UNIT;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0; //Story Mode
					Trans_Start();
				}
			}
			/*
			//Draw week name and tracks
			menu.font_bold.draw(&menu.font_bold,
				menu_options[menu.select].name,
				SCREEN_WIDTH - 16,
				8,
				FontAlign_Right
			);
			*/
			//Draw Song Names,"Tracks" and "Undertale Univers2"
			RECT Universe = {8, 48, 140, 9};
			Gfx_BlitTex(&menu.tex_names,&Universe, 170, 10);

			RECT Tracks = {51,  2,  50, 10};
			Gfx_BlitTex(&menu.tex_names,&Tracks, 20, 147+40);

			RECT name_1 = {44, 15,  65, 7};
			Gfx_BlitTex(&menu.tex_names,&name_1, 20, 160+40);

			RECT name_2 = {44, 24,  54, 7};
			Gfx_BlitTex(&menu.tex_names,&name_2, 20, 169+40);

			RECT name_3 = {44, 33,  54, 7};
			Gfx_BlitTex(&menu.tex_names,&name_3, 20, 178+40);
			
			/*
			const char * const *trackp = menu_options[menu.select].tracks;
			for (size_t i = 0; i < COUNT_OF(menu_options[menu.select].tracks); i++, trackp++)
			{
				if (*trackp != NULL)
					menu.font_bold.draw_col(&menu.font_bold,
						*trackp,
						SCREEN_WIDTH - 16,
						SCREEN_HEIGHT - (4 * 24) + (i * 24),
						FontAlign_Right,
						210,
						90,
						108
					);
			}
			*/
			//chara stuffs
			if (menu.page_param.stage.diff == StageDiff_Chara)
			{
			stage.swap_player = true;
			swapbg = 77;
			}
			else
			{
			swapbg = 0;
			stage.swap_player = false;
			}

			//Draw Background story
			RECT bg_src = {0, swapbg, 256, 77};
			RECT bf_dst = {0, 25, SCREEN_WIDTH, 112};

			Gfx_DrawTex(&menu.tex_storybg, &bg_src, &bf_dst);

			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(48,1);
			menu.scroll += (next_scroll - menu.scroll) >> 3;
			
			if (menu.next_page == menu.page || menu.next_page == MenuPage_Main)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(menu_options); i++)
				{
					s32 y = 64 + (i * 48) - (menu.scroll >> FIXED_SHIFT);
					if (y <= 16)
						continue;
					if (y >= SCREEN_HEIGHT)
						break;
					Menu_DrawWeek(menu_options[i].week, 48, y + 80);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				Menu_DrawWeek(menu_options[menu.select].week, 48, 144 + (menu.select * 48) - (menu.scroll >> FIXED_SHIFT));
			}
			
			break;
		}
		case MenuPage_Freeplay:
		{
			static const struct
			{
				StageId stage;
				StageId stagealt;
				u32 col;
				const char *text;
			} menu_options[] = {
				//{StageId_4_4, 0xFFFC96D7, "TEST"},
				{StageId_1_1,StageId_1_1Chara, 0xFF0010, "NYEH HEH HEH"},
				{StageId_1_2,StageId_1_2Chara, 0xAFB3E6, "BONELY ONE"},
				{StageId_1_3,StageId_1_3Chara, 0xAFB3E6, "NOT ENOUGH"},
				{StageId_1_4,StageId_1_4, 0x221C3E, "NO MORE DEALS"},
				{StageId_3_1,StageId_3_1, 0xB38FD5, "EEEECHROME"},
				{StageId_3_2,StageId_3_2, 0xAFB3E6, "PARASITE SANS"},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_BF;
				menu.page_state.freeplay.back_r = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_g = FIXED_DEC(255,1);
				menu.page_state.freeplay.back_b = FIXED_DEC(255,1);
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"FREEPLAY",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw difficulty selector
			Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					menu.next_page = MenuPage_Stage;
					if (menu.page_param.stage.diff == StageDiff_Chara)
	                menu.page_param.stage.id = menu_options[menu.select].stagealt;
					else
					menu.page_param.stage.id = menu_options[menu.select].stage;
					menu.page_param.stage.story = false;
					Trans_Start();
				}
				if (menu.select > 2)
				menu.page_param.stage.diff = StageDiff_BF;
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 1; //Freeplay
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
			
			menu.page_state.freeplay.back_r += (tgt_r - menu.page_state.freeplay.back_r) >> 4;
			menu.page_state.freeplay.back_g += (tgt_g - menu.page_state.freeplay.back_g) >> 4;
			menu.page_state.freeplay.back_b += (tgt_b - menu.page_state.freeplay.back_b) >> 4;
			
			Menu_DrawBack(
				true,
				menu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
				menu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
				menu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
				0, 0, 0
			);
			break;
		}
		case MenuPage_Credits:
		{
			menu.skip_selectup = menu.select - 1;
			menu.skip_selectdown = menu.select + 1;
			static const struct
			{
				u32 col;
				const char *text;
				const char *text2;
				s16 icon;
			} menu_options[] = {
				{0xFF9271FD, "PORTED BY",NULL,-1},
				{0xFF9271FD, "UNSTOPABLE",  "SPRITES CODE OFFSETS",0},
				{0xFF9271FD, "SPARK", "MADE SICK DODGE LABELS",10},
				{0xFF9271FD, "IGORSOU", "SPRITES CODE OFFSETS",1},
				{0xFF9271FD, "",NULL,-1},
				{0xFF9271FD, "PLAYTESTERS",NULL,-1},
				{0xFF9271FD, "JOHN PAUL",  "PLAYTESTER AND FRIEND",4},
				{0xFF941653, "LORD SCOUT", "PLAYTESTER AND FRIEND",6},
				{0xFF9271FD, "",NULL,-1},
				{0xFF9271FD, "SPECIAL THANKS",NULL,-1},
				{0xFF941653, "LUCKY","MISS AND ACCURATE CODE",7 },
				{0xFF941653, "CUCKYDEV","MAKE THE PSXFUNKIN",2 },
				{0xFF941653, "PSXFUNKIN DISCORD","DISCORD",-1},
				{0xFF941653, "ZERIBEN","FRIEND",5},
				{0xFF9271FD, "",NULL,-1},
				{0xFF9271FD, "OG CREATORS",NULL,-1},
				{0xFF941653, "JLOOR","OWNER AND MAIN PROGRAMMER",8},
				{0xFF941653, "ZERO ARTIST","CO OWNER AND MAIN ARTIST",9},
				{0xFF941653, "JHAIX","MAIN MUSICIAN AND CHARTER",-1},
				{0xFF941653, "ALEXR","MUSICIAN",-1},
				{0xFF941653, "MANUX", "PROGRAMMER",-1},
				{0xFF941653, "YIRIUS", "PROGRAMMER",-1},
				{0xFF941653, "REWINDXA", "MAIN CHARTER",-1},
				{0xFF9271FD, "",NULL,-1},
				{0xFF9271FD, "OG SPECIAL THANKS",NULL,-1},
				{0xFF941653, "JLOOR COMMUNITY","DISCORD SERVER",-1},
				{0xFF941653, "CAPE FUNKIN","CHARTER OF NO MORE DEALS",-1 },
				{0xFF941653, "ZETAE","HELPED IN PAPY BG",-1},
				{0xFF941653, "SOYJULIAN","HELPED IN SANS BG",-1},
				{0xFF941653, "JKL", "FRIEND",-1},
				{0xFF941653, "MIGUEL EXTREME STUDIOS", "FRIEND",-1},
				{0xFF941653, "ELNELSON", "FRIEND",-1},
				{0xFF941653, "SEREBEAT", "BF REANIMATED",-1},
				{0xFF9271FD, "",NULL,-1},
				{0xFF9271FD, "OG SONGS",NULL,-1},
				{0xFF941653, "BENLAB CRIMSON","COMPOSER OF NO MORE DEALS",-1 },
				{0xFF941653, "MIDEAR","COMPOSER OF BONELY ONE",-1 },
			};
            //debug thing
			//FntPrint("select %d", menu.select);
			//FntPrint("sselectup %d", menu.skip_selectup);
			//FntPrint("sselectdown %d", menu.skip_selectdown);
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_BF;
				menu.page_state.credit.back_r = FIXED_DEC(255,1);
				menu.page_state.credit.back_g = FIXED_DEC(255,1);
				menu.page_state.credit.back_b = FIXED_DEC(255,1);
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu_options[menu.skip_selectup].text[0] == '\0')
                        menu.select -= 2;
					else if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu_options[menu.skip_selectdown].text[0] == '\0')
					    menu.select += 2;
					else if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 2; //Credits
					Trans_Start();
				}
			}
			      //Draw credits information
			        menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[menu.select].text2, false),
					160,
					SCREEN_HEIGHT2 - 110,
					FontAlign_Center
			);
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				//Draw credits image
				if (menu_options[i].icon != -1)
				Credits_DrawImage(menu_options[i].icon, 240, SCREEN_HEIGHT2 + y - 8);

				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					160,
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Center
				);
			}
			
			//Draw background
			fixed_t tgt_r = (fixed_t)((menu_options[menu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_g = (fixed_t)((menu_options[menu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_b = (fixed_t)((menu_options[menu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
			
			menu.page_state.credit.back_r += (tgt_r - menu.page_state.credit.back_r) >> 4;
			menu.page_state.credit.back_g += (tgt_g - menu.page_state.credit.back_g) >> 4;
			menu.page_state.credit.back_b += (tgt_b - menu.page_state.credit.back_b) >> 4;
			
			Menu_DrawBack(
				true,
				menu.page_state.credit.back_r >> (FIXED_SHIFT + 1),
				menu.page_state.credit.back_g >> (FIXED_SHIFT + 1),
				menu.page_state.credit.back_b >> (FIXED_SHIFT + 1),
				0, 0, 0
			);
			break;
		}
		case MenuPage_Options:
		{
			static const char *gamemode_strs[] = {"NORMAL", "SWAP", "TWO PLAYER"};
			static const struct
			{
				enum
				{
					OptType_Boolean,
					OptType_Enum,
				} type;
				const char *text;
				void *value;
				union
				{
					struct
					{
						int a;
					} spec_boolean;
					struct
					{
						s32 max;
						const char **strs;
					} spec_enum;
				} spec;
			} menu_options[] = {
				{OptType_Enum,    "GAMEMODE", &stage.mode, {.spec_enum = {COUNT_OF(gamemode_strs), gamemode_strs}}},
				//{OptType_Boolean, "INTERPOLATION", &stage.expsync},
				{OptType_Boolean, "GHOST TAP", &stage.ghost, {.spec_boolean = {0}}},
				{OptType_Boolean, "DOWNSCROLL", &stage.downscroll, {.spec_boolean = {0}}},
				{OptType_Boolean, "MIDDLESCROLL", &stage.middlescroll, {.spec_boolean = {0}}},
				{OptType_Boolean, "FLASHING LIGHTS", &stage.flashlight, {.spec_boolean = {0}}},
				{OptType_Boolean, "KADE ENGINE CAMERA", &stage.movecamera, {.spec_boolean = {0}}},
				{OptType_Boolean, "BOTPLAY", &stage.botplay, {.spec_boolean = {0}}},
			};

			if (stage.mode == StageMode_2P)
			stage.middlescroll = false;
			
			//Initialize page
			if (menu.page_swap)
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"OPTIONS",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Handle option changing
				switch (menu_options[menu.select].type)
				{
					case OptType_Boolean:
						if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							*((boolean*)menu_options[menu.select].value) ^= 1;
						break;
					case OptType_Enum:
						if (pad_state.press & PAD_LEFT)
							if (--*((s32*)menu_options[menu.select].value) < 0)
								*((s32*)menu_options[menu.select].value) = menu_options[menu.select].spec.spec_enum.max - 1;
						if (pad_state.press & PAD_RIGHT)
							if (++*((s32*)menu_options[menu.select].value) >= menu_options[menu.select].spec.spec_enum.max)
								*((s32*)menu_options[menu.select].value) = 0;
						break;
				}
				
				//Return to main menu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 3; //Options
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				char text[0x80];
				switch (menu_options[i].type)
				{
					case OptType_Boolean:
						sprintf(text, "%s %s", menu_options[i].text, *((boolean*)menu_options[i].value) ? "ON" : "OFF");
						break;
					case OptType_Enum:
						sprintf(text, "%s %s", menu_options[i].text, menu_options[i].spec.spec_enum.strs[*((s32*)menu_options[i].value)]);
						break;
				}
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				253 >> 1, 113 >> 1, 155 >> 1,
				0, 0, 0
			);
			break;
		}
	#ifdef PSXF_NETWORK
		case MenuPage_NetHost:
		{
			const size_t menu_options = 3;
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.net_host.type = false;
				menu.page_state.net_host.port[0] = '\0';
				menu.page_state.net_host.pass[0] = '\0';
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!menu.page_state.net_host.type)
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = menu_options - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						if (menu.select < menu_options - 1)
							menu.select++;
						else
							menu.select = 0;
					}
					
					//Handle selection when cross is pressed
					if (pad_state.press & (PAD_START | PAD_CROSS))
					{
						switch (menu.select)
						{
							case 0: //Port
							case 1: //Pass
								menu.page_state.net_host.type = true;
								break;
							case 2: //Host
								if (!Network_HostPort(menu.page_state.net_host.port, menu.page_state.net_host.pass))
								{
									menu.next_page = MenuPage_NetOpWait;
									menu.next_select = 0;
									Trans_Start();
								}
								break;
						}
					}
					
					//Return to main menu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						menu.next_page = MenuPage_Main;
						menu.next_select = 5; //Host Server
						Trans_Start();
					}
				}
				else
				{
					//Stop typing when start is pressed
					if (pad_state.press & PAD_START)
					{
						switch (menu.select)
						{
							case 0: //Port
							case 1: //Pass
								menu.page_state.net_host.type = false;
								break;
						}
					}
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"HOST SERVER",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw options
			MenuStr_Process(&menu.page_state.net_host.port, 64 + 3 * 0, 64 + 16 * 0, "Port: %s", menu.select == 0, menu.page_state.net_host.type);
			MenuStr_Process(&menu.page_state.net_host.pass, 64 + 3 * 1, 64 + 16 * 1, "Pass: %s", menu.select == 1, menu.page_state.net_host.type);
			menu.font_bold.draw(&menu.font_bold, Menu_LowerIf("HOST", menu.select != 2), 64 + 3 * 2, 64 + 16 * 2, FontAlign_Left);
			
			//Draw background
			Menu_DrawBack(
				true,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetJoin:
		{
			const size_t menu_options = 4;
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.page_state.net_join.type = false;
				menu.page_state.net_join.ip[0] = '\0';
				menu.page_state.net_join.port[0] = '\0';
				menu.page_state.net_join.pass[0] = '\0';
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!menu.page_state.net_join.type)
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						if (menu.select > 0)
							menu.select--;
						else
							menu.select = menu_options - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						if (menu.select < menu_options - 1)
							menu.select++;
						else
							menu.select = 0;
					}
					
					//Handle selection when cross is pressed
					if (pad_state.press & (PAD_START | PAD_CROSS))
					{
						switch (menu.select)
						{
							case 0: //Ip
							case 1: //Port
							case 2: //Pass
								menu.page_state.net_join.type = true;
								break;
							case 3: //Join
								if (!Network_Join(menu.page_state.net_join.ip, menu.page_state.net_join.port, menu.page_state.net_join.pass))
								{
									menu.next_page = MenuPage_NetConnect;
									menu.next_select = 0;
									Trans_Start();
								}
								break;
						}
					}
					
					//Return to main menu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						menu.next_page = MenuPage_Main;
						menu.next_select = 4; //Join Server
						Trans_Start();
					}
				}
				else
				{
					//Stop typing when start is pressed
					if (pad_state.press & PAD_START)
					{
						switch (menu.select)
						{
							case 0: //Join
							case 1: //Port
							case 2: //Pass
								menu.page_state.net_join.type = false;
								break;
						}
					}
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"JOIN SERVER",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw options
			MenuStr_Process(&menu.page_state.net_join.ip, 64 + 3 * 0, 64 + 16 * 0, "Address: %s", menu.select == 0, menu.page_state.net_join.type);
			MenuStr_Process(&menu.page_state.net_join.port, 64 + 3 * 1, 64 + 16 * 1, "Port: %s", menu.select == 1, menu.page_state.net_join.type);
			MenuStr_Process(&menu.page_state.net_join.pass, 64 + 3 * 2, 64 + 16 * 2, "Pass: %s", menu.select == 2, menu.page_state.net_join.type);
			menu.font_bold.draw(&menu.font_bold, Menu_LowerIf("JOIN", menu.select != 3), 64 + 3 * 3, 64 + 16 * 3, FontAlign_Left);
			
			//Draw background
			Menu_DrawBack(
				true,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetConnect:
		{
			//Change state according to network state
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
				else if (Network_Allowed())
				{
					//Allowed to join
					menu.next_page = MenuPage_NetLobby;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"CONNECTING",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				113 >> 1, 146 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetOpWait:
		{
			//Change state according to network state
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
				else if (Network_HasPeer())
				{
					//Peer has joined
					menu.next_page = MenuPage_NetOp;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"WAITING FOR PEER",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				113 >> 1, 146 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetOp:
		{
			static const struct
			{
				boolean diff;
				StageId stage;
				const char *text;
			} menu_options[] = {
				//{StageId_4_4, "TEST"},
				{true,  StageId_1_4, "TUTORIAL"},
				{true,  StageId_1_1, "BOPEEBO"},
				{true,  StageId_1_2, "FRESH"},
				{true,  StageId_1_3, "DADBATTLE"},
				{true,  StageId_2_1, "SPOOKEEZ"},
				{true,  StageId_2_2, "SOUTH"},
				{true,  StageId_2_3, "MONSTER"},
				{true,  StageId_3_1, "PICO"},
				{true,  StageId_3_2, "PHILLY NICE"},
				{true,  StageId_3_3, "BLAMMED"},
				{true,  StageId_4_1, "SATIN PANTIES"},
				{true,  StageId_4_2, "HIGH"},
				{true,  StageId_4_3, "MILF"},
				{true,  StageId_5_1, "COCOA"},
				{true,  StageId_5_2, "EGGNOG"},
				{true,  StageId_5_3, "WINTER HORRORLAND"},
				{true,  StageId_6_1, "SENPAI"},
				{true,  StageId_6_2, "ROSES"},
				{true,  StageId_6_3, "THORNS"},
				{true,  StageId_7_1, "UGH"},
				{true,  StageId_7_2, "GUNS"},
				{true,  StageId_7_3, "STRESS"},
				{false, StageId_Kapi_1, "WOCKY"},
				{false, StageId_Kapi_2, "BEATHOVEN"},
				{false, StageId_Kapi_3, "HAIRBALL"},
				{false, StageId_Kapi_4, "NYAW"},
				{true,  StageId_Clwn_1, "IMPROBABLE OUTSET"},
				{true,  StageId_Clwn_2, "MADNESS"},
				{true,  StageId_Clwn_3, "HELLCLOWN"},
				{false, StageId_Clwn_4, "EXPURGATION"},
				{false, StageId_2_4, "CLUCKED"},
			};
			
			//Initialize page
			if (menu.page_swap)
			{
				menu.scroll = COUNT_OF(menu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				menu.page_param.stage.diff = StageDiff_Normal;
				menu.page_state.net_op.swap = false;
			}
			
			//Handle option and selection
			if (menu.next_page == menu.page && Trans_Idle())
			{
				//Check network state
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
				else if (!Network_HasPeer())
				{
					//Peer disconnected
					menu.next_page = MenuPage_NetOpWait;
					menu.next_select = 0;
					Trans_Start();
				}
				
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (menu.select > 0)
						menu.select--;
					else
						menu.select = COUNT_OF(menu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (menu.select < COUNT_OF(menu_options) - 1)
						menu.select++;
					else
						menu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					//Load stage
					Network_SetReady(false);
					stage.mode = menu.page_state.net_op.swap ? StageMode_Net2 : StageMode_Net1;
					menu.next_page = MenuPage_Stage;
					menu.page_param.stage.id = menu_options[menu.select].stage;
					if (!menu_options[menu.select].diff)
						menu.page_param.stage.diff = StageDiff_Hard;
					menu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Swap characters if triangle is pressed
				if (pad_state.press & PAD_TRIANGLE)
					menu.page_state.net_op.swap ^= true;
			}
			
			//Draw op controls
			const char *control_txt;
			
			control_txt = menu.page_state.net_op.swap ? "You will be Player 2. Press Triangle to swap." : "You will be Player 1. Press Triangle to swap.";
			menu.font_arial.draw_col(&menu.font_arial, control_txt, 24, SCREEN_HEIGHT - 24 - 12, FontAlign_Left, 0x80, 0x80, 0x80);
			menu.font_arial.draw_col(&menu.font_arial, control_txt, 24 + 1, SCREEN_HEIGHT - 24 - 12 + 1, FontAlign_Left, 0x00, 0x00, 0x00);
			
			//Draw difficulty selector
			if (menu_options[menu.select].diff)
				Menu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Draw options
			s32 next_scroll = menu.select * FIXED_DEC(24,1);
			menu.scroll += (next_scroll - menu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(menu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (menu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				menu.font_bold.draw(&menu.font_bold,
					Menu_LowerIf(menu_options[i].text, menu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Menu_DrawBack(
				true,
				113 >> 1, 253 >> 1, 146 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetLobby:
		{
			//Check network state
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					menu.next_page = MenuPage_NetFail;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"WAITING FOR HOST",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				253 >> 1, 146 >> 1, 113 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetFail:
		{
			//Return to main menu if circle or start is pressed
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (pad_state.press & (PAD_CIRCLE | PAD_START))
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"DISCONNECTED",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				253 >> 1, 30 >> 1, 15 >> 1,
				0, 0, 0
			);
			break;
		}
		case MenuPage_NetInitFail:
		{
			//Return to main menu if circle or start is pressed
			if (menu.next_page == menu.page && Trans_Idle())
			{
				if (pad_state.press & (PAD_CIRCLE | PAD_START))
				{
					menu.next_page = MenuPage_Main;
					menu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			menu.font_bold.draw(&menu.font_bold,
				"WSA INIT FAILED",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Menu_DrawBack(
				true,
				253 >> 1, 30 >> 1, 15 >> 1,
				0, 0, 0
			);
			break;
		}
	#endif
		case MenuPage_Stage:
		{
			//Unload menu state
			Menu_Unload();
			
			//Load new stage
			LoadScr_Start();
			Stage_Load(menu.page_param.stage.id, menu.page_param.stage.diff, menu.page_param.stage.story);
			gameloop = GameLoop_Stage;
			LoadScr_End();
			break;
		}
		case MenuPage_Movie:
		{
			//Unload
			Menu_Unload();

			//Play movie
			LoadScr_Start();
			gameloop = GameLoop_Movie;
			LoadScr_End();
			break;
		}
		default:
			break;
	}
	
	//Clear page swap flag
	menu.page_swap = menu.page != exec_page;
}
