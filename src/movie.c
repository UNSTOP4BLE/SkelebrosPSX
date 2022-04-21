#include "stage.h"
#include "mem.h"
#include "loadscr.h"
#include "movie.h"
#include "font.h"
#include "main.h"
#include "menu.h"
#include "stage.h"
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>
#include "strplay.c"

//#define DEBUG 1
Movie movie;

STRFILE StrFile[] = {
	// File name	Resolution		Frame count
	{"\\STR\\INTRO.STR;1", 320, 240, 508},
    {"\\STR\\SANS.STR;1", 320, 240, 242},
    {"\\STR\\GASTER.STR;1", 320, 240, 202},
    {"\\STR\\CHARA.STR;1", 320, 240, 520},
    {"\\STR\\FINAL.STR;1", 320, 240, 435},
};

void PlayMovie() {
     // Reset and initialize stuff
	ResetCallback();
	CdInit();
	ResetGraph(0);
	SetGraphDebug(0);

}

void Movie_Tick(void) 
{
    Gfx_SetClear(0, 0, 0);
        switch (movie.select)
        {
            case 0:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[0]);
            break;
            case 1:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[1]);
            break;
            case 2:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[2]);
            break;
            case 3:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[3]);
            break;
            case 4:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[4]);
            break;
            default:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[4]);
            break;

  }
			
}