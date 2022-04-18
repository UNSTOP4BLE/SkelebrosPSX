#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(19500)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(8300)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(9300)}, //XA_Bopeebo
	{XA_Week1A, XA_LENGTH(15900)}, //XA_Fresh
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(15600)}, //XA_Dadbattle
	{XA_Week1B, XA_LENGTH(15800)}, //XA_Tutorial
	//WEEK2A.XA
	{XA_Week2A, XA_LENGTH(9300)}, //XA_Spookeez
	{XA_Week2A, XA_LENGTH(15900)}, //XA_South
	//WEEK2B.XA
	{XA_Week2B, XA_LENGTH(15600)}, //XA_Monster
	//WEEK3A.XA
	{XA_Week3A, XA_LENGTH(14800)},  //XA_Pico
	{XA_Week3A, XA_LENGTH(12600)}, //XA_Philly
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
	"\\MUSIC\\WEEK2A.XA;1", //XA_Week2A
	"\\MUSIC\\WEEK2B.XA;1", //XA_Week2B
	"\\MUSIC\\WEEK3A.XA;1", //XA_Week3A
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"bopeebo", true}, //XA_Bopeebo
	{"fresh", true},   //XA_Fresh
	//WEEK1B.XA
	{"dadbattle", true}, //XA_Dadbattle
	{"tutorial", true}, //XA_Tutorial
	//WEEK2A.XA
	{"spookeez", true}, //XA_Spookeez
	{"south", true},    //XA_South
	//WEEK2B.XA
	{"monster", true}, //XA_Monster
	//WEEK3A.XA
	{"pico", true},   //XA_Pico
	{"philly", true}, //XA_Philly

	{NULL, false}
};
