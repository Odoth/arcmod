This isnt a source file, it contains various snippets to put in
other source files.  For the functions you already have, be careful
not to overwrite any modifications you've already made.

###  merc.h stuff.

#define CON_PLAYING              0
#define CON_GET_NAME             1
#define CON_GET_OLD_PASSWORD     2
#define CON_CONFIRM_NEW_NAME     3
#define CON_GET_NEW_PASSWORD     4
#define CON_CONFIRM_NEW_PASSWORD 5
#define CON_GET_NEW_RACE         6
#define CON_GET_NEW_SEX          7
#define CON_GET_NEW_CLASS        8
#define CON_GET_ALIGNMENT        9
#define CON_DEFAULT_CHOICE      10 
#define CON_GEN_GROUPS          11 
#define CON_PICK_WEAPON         12
#define CON_READ_IMOTD          13
#define CON_READ_MOTD           14
#define CON_BREAK_CONNECT       15
#define CON_GET_CREATION_MODE   19
#define CON_ROLL_STATS          20
#define CON_GET_STAT_PRIORITY   21
#define CON_COPYOVER_RECOVER    22
#define MAX_CON_STATE           23

#define CON_SMITH_WELCOME       1
#define CON_SMITH_TYPE          2
#define CON_SMITH_SUBTYPE       3
#define CON_SMITH_PURCHASE      4
#define CON_SMITH_MATERIAL      5
#define CON_SMITH_QUALITY       6
#define CON_SMITH_COLOR         7
#define CON_SMITH_PERSONAL      8
#define CON_SMITH_LEVEL         9
#define CON_SMITH_KEYWORDS      10
#define CON_SMITH_SELECT        11
#define CON_SMITH_INVENTORY     12
#define CREATION_UNKNOWN         0
#define CREATION_INSTANT         1
#define CREATION_QUICK           2
#define CREATION_NORMAL          3
#define CREATION_REMORT          4
#define CREATION_BLACKSMITH     5

#define STAT_STR        0
#define STAT_CON        1
#define STAT_VIT        2
#define STAT_AGI        3
#define STAT_DEX        4
#define STAT_INT        5
#define STAT_WIS        6
#define STAT_DIS        7
#define STAT_CHA        8
#define STAT_LUC        9
#define MAX_STATS   10
#define MAX_CURRSTAT 200

#define SEX_NEUTRAL           0
#define SEX_MALE              1
#define SEX_FEMALE            2
#define SEX_BOTH            3

#define ROOM_BLACKSMITH     (W)
     
#define IS_HERO(ch)     (!IS_NPC(ch)&&((ch)->level >= (90+(ch)->pcdata->remorts)))

struct pc_race_type  /* additional data for pc races */
{
	char *  name;           /* MUST be in race_type */
	char    who_name[9];
	sh_int  class_mult[MAX_CLASS];  /* exp multiplier for class, * 100 */
	sh_int  num_skills;
	char *  skills[5];
	sh_int  skill_level[5];
	sh_int  skill_percent[5];
	sh_int  min_stats[MAX_STATS];   /* minimum stats */
	sh_int  max_stats[MAX_STATS];   /* maximum stats */
	sh_int  size;           /* aff bits for the race */
	sh_int  gender;
	sh_int  remorts;
	sh_int  skill_gsns[5];      /* bonus skills for the race */
};

struct  class_type
{
	char *  name;           /* the full name of the class */
	char    who_name    [4];    /* Three-letter name for 'who'  */
	sh_int  attr_prime;     /* Prime attribute      */
	sh_int  attr_second[2]; /* Secondary attributes  */
	sh_int  stat_priority[MAX_STATS-3];
	sh_int  weapon;         /* First weapon         */
	sh_int  guild[MAX_GUILD];   /* Vnum of guild rooms      */
	sh_int  skill_adept;        /* Maximum skill level      */
	sh_int  thac0_00;       /* Thac0 for level  0       */
	sh_int  thac0_32;       /* Thac0 for level 32       */
	sh_int  hp_min;         /* Min hp gained on leveling    */
	sh_int  hp_max;         /* Max hp gained on leveling    */
	bool    fMana;          /* Class gains mana on level    */
	char *  base_group;     /* base skills gained       */
	char *  default_group;      /* default skills gained    */
};



### In your get_skill function, add the for loop.
	else if (!IS_NPC(ch))
	{
	if (ch->level < skill_table[sn].skill_level[ch->class])
		skill = 0;
	else
		skill = ch->pcdata->learned[sn];

	for (i=0; i<pc_race_table[ch->race].num_skills; i++)
		if ((pc_race_table[ch->race].skill_gsns[i]==sn)
			&& (ch->level>=pc_race_table[ch->race].skill_level[i]))
			skill = (skill*(100-pc_race_table[ch->race].skill_percent[i]))/100
				+ pc_race_table[ch->race].skill_percent[i];
	}

### Add this to whatever file you want, and declare it in interp.c and interp.h
void do_raceskills( CHAR_DATA *ch, char *argument )
{
	int race, i;
	char buf[MAX_STRING_LENGTH];

	if (argument[0]=='\0')
		race=ch->race;
	else
		race=race_lookup(argument);

	if (race==0)
	{
		send_to_char("Which race do you want to know the skills for?\n\r", ch);
		return;
	}

	if (!race_table[race].pc_race)
	{
		send_to_char("That is not a valid player race.\n\r", ch );
		return;
	}

	send_to_char("Skill                 Level  Percent\n\r",ch);

	for (i=0; i<pc_race_table[race].num_skills; i++)
	{
		sprintf(buf, "%20s  %5d  %5d%%\n\r",
			pc_race_table[race].skills[i],
			pc_race_table[race].skill_level[i],
			pc_race_table[race].skill_percent[i]);
		send_to_char(buf, ch);
	}

	return;
}

# tables.c
const struct stat_type stat_table[] =
{
	{"strength",    "Str", STAT_STR, {60, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"constitution",    "Con", STAT_CON, { 0,60, 0, 0, 0, 0, 0, 0, 0, 0}},
	{"vitality",    "Vit", STAT_VIT, { 0, 0,60, 0, 0, 0, 0, 0, 0, 0}},
	{"agility",     "Agi", STAT_AGI, { 0, 0, 0,60, 0, 0, 0, 0, 0, 0}},
	{"dexterity",   "Dex", STAT_DEX, { 0, 0, 0, 0,60, 0, 0, 0, 0, 0}},
	{"intelligence",    "Int", STAT_INT, { 0, 0, 0, 0, 0,60, 0, 0, 0, 0}},
	{"wisdom",      "Wis", STAT_WIS, { 0, 0, 0, 0, 0, 0,60, 0, 0, 0}},
	{"discipline",  "Dis", STAT_DIS, { 0, 0, 0, 0, 0, 0, 0,60, 0, 0}},
	{"charisma",    "Cha", STAT_CHA, { 0, 0, 0, 0, 0, 0, 0, 0,60, 0}},
	{"luck",        "Luc", STAT_LUC, { 0, 0, 0, 0, 0, 0, 0, 0, 0,60}},
	{"body",        NULL ,   -1, {15,20,20,15,10, 0, 0, 0, 0, 0}},
	{"mind",        NULL ,   -1, { 0, 0, 0, 0, 0,25,20,15,10,10}},
	{"toughness",   NULL ,   -1, {25,20,10, 0, 0, 0, 0,25, 0, 0}},
	{"speed",       NULL ,   -1, { 0, 0,10,25,30,15, 0, 0, 0, 0}},
	{"wit",     NULL ,   -1, { 0, 0, 0, 0, 0, 0,20, 0,30,30}},
	{ NULL,     NULL ,   -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};


### update.c, in advance_level add this after the add values are set, but
	before they are added.  it is also suggested that you make xp gain
	go down with each remort to prevent a glut of level 100s
   
   if (IS_SET(ch->form, FORM_CENTAUR)) add_move =add_move*3/2;
  
   if (ch->level>90)
   {
	  int bonus= ch->level-89;
	  add_hp*=bonus;
	  add_mana*=bonus;
	  add_move*=bonus;
	  add_prac*=bonus;
	  ch->train+=(bonus-2)/2;
   }


# sample pc races for remort 0-3.  adapt your races to this format or these
races to your mud.  Powerful races are suggested to be higher remorts.
(e.g. Angel, Demon -remort 10; Gold Dragon, Lich - remort 9; Giant, Black Dragon -remort 8;
		Djinn, Green Dragon - remort 7; Vampire, Titan - remort 6)
(classes:  warrior thief cleric mage gladiator samurai paladin
		assassin ninja monk templar illusionist gunslinger ranger)
	{
	"avian",        "Avian ",
	{ 105, 120, 100, 110, 140, 125, 125, 150, 130, 120, 125, 135, 120, 120 },
	2, { "meditation", "lore" }, {2, 40}, {40, 50},
	{  15,  20,  35,  50,  25,      30,  40,  50,  40,  20 },
	{  95,  90, 115, 110,  95,      90, 110, 110, 100, 100 },   
	SIZE_LARGE, SEX_BOTH, 0
	},      

	{   
	"drow",      "Drow  ",
	{ 100, 100, 110, 100, 130, 125, 160, 120, 130, 150, 140, 125, 120, 160 },
	2, { "faerie fire", "fly" }, {8, 50}, {70, 50},
	{  25,  20,  25,  25,  25,      30,  40,  35,  25,  10 },
	{  95,  90,  95, 105, 105,     110, 100,  95,  85,  90 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},
	
	{
	"dwarf",    "Dwarf ",
	{ 100, 120, 110, 125, 120, 125, 130, 130, 150, 140, 125, 135, 125, 155 },
	1, { "drunken fury" }, {25}, {70},
	{  55,  50,  25,  10,  30,      40,  30,  35,  20,  20 },
	{  95, 110, 105,  80, 105,      85, 105, 110,  80,  100 },   
	SIZE_SMALL, SEX_BOTH, 0
	},

	{   
	"elf",      "Elf   ",
	{ 105, 110, 110, 100, 125, 125, 130, 135, 135, 120, 125, 120, 125, 120 },
	2, { "sneak", "hide" }, {20, 10}, {80, 80},
	{  15,  20,  20,  25,  35,      40,  20,  20,  25,  20 },
	{  95,  80, 100, 105, 105,     100, 110,  90, 105, 100 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},
		
     {
	"gimp",    "Gimp  ",
	{ 60, 60, 60, 60, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80 },
	0, { "" }, {0}, {0},
	{  60,  50,  40, 50,  60,   40, 50, 60, 60, 60 },
	{ 80, 80, 80, 80, 80,     80, 80, 80, 80, 80 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},

      {
	"goblin",    "Goblin",
	{ 100, 125, 130, 140, 120, 120, 145, 130, 135, 145, 150, 150, 120, 125},
	3, { "trip", "gouge", "mug" }, {10,30,50}, {80,75,70},
	{  50,  55,  45,  60,  20,      45,  55,  55,  55,  60 },
	{ 110, 105, 100, 115, 100,      75,  80,  75,  75,  80 },   
	SIZE_SMALL, SEX_BOTH, 0
	},

	{
	"golem",    "Golem ",
	{ 100, 140, 125, 130, 120, 150, 145, 160, 160, 145, 150, 150, 130, 160 },
	2, { "stone skin", "bash" }, {5,1}, {70, 90},
	{  40,  40,  60,   1,  10,      15,  25,  40,   5,  30 },
	{ 120, 120, 100,  80,  90,      75,  85, 120,  85,  90 },   
	SIZE_HUGE, SEX_BOTH, 0
	},

	{
	"halfelf",    "HlfElf",
	{ 100, 100, 130, 105, 120, 125, 145, 120, 130, 125, 135, 120, 130, 120 },
	1, { "sneak" }, {25}, {60},
	{  25,  20,  20,  15,  30,      30,  30,  30,  25,  40 },
	{  95,  90, 100, 105, 100,     100, 100,  90, 105, 100 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},

	{
	"halfogre",    "HlfOgr",
	{ 100, 110, 105, 115, 120, 135, 140, 130, 135, 135, 130, 140, 130, 125 },
	1, { "bash" }, {30},{60},
	{  30,  35,  25,  15,  25,      25,  25,  25,  15,  20 },
	{ 110, 105, 105,  95,  95,      85,  95,  95,  95, 100 },   
	SIZE_LARGE, SEX_BOTH, 0
	},

	{
	"halforc",    "HlfOrc",
	{ 100, 100, 130, 125, 120, 135, 160, 120, 130, 155, 160, 145, 130, 135 },
	1, { "backstab" }, {12}, {40},
	{  40,  45,  30,  35,  35,      20,  15,  15,   5,  10 },
	{ 100, 105, 110,  90,  95,     100,  95, 105,  85,  90 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},

	{
	"hobbit",    "Hobbit",
	{ 110, 100, 105, 110, 130, 130, 130, 125, 125, 125, 130, 120, 125, 120 },
	3, { "steal", "sneak", "create food" }, {10, 1, 15}, {60, 80, 50},
	{  15,  40,  35,  40,  20,      30,  30,  20,  40,  30 },
	{  85,  80,  95, 120, 120,      90, 100,  90, 100, 110 },   
	SIZE_SMALL, SEX_BOTH, 0
	},

	{
	"human",    "Human ",
	{ 100, 100, 100, 100, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120 },
	0, { "" },{0},{0},
	{  20,  20,  20,  20,  20,      20,  20,  20,  20,  20 },
	{ 100, 100, 100, 100, 100,     100, 100, 100, 100, 100 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},

	{
	"leprechaun",    "Leprec",
	{ 125, 100, 120, 105, 140, 150, 160, 150, 145, 160, 150, 120, 130, 130 },
	2, { "charm person", "haste" }, {10, 50}, {90, 50},
	{  10,  15,  35,  25,  20,      30,  35,  30,  40,  40 },
	{  80,  90,  95, 105, 100,     115,  85,  80, 115, 120 },   
	SIZE_SMALL, SEX_BOTH, 0
	},

	{
	"mutant",    "Mutant",
	{ 100, 100, 100, 100, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120 },
	2, { "fear", "change sex" }, {40, 20}, {70, 70},

	{   1,   1,   1,   1,   1,       1,   1,   1,   1,   1 },
	{ 110,  90, 110, 110, 110,     110, 110, 110,  90, 110 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},

	{
      "myrddraal",    "Myrdrl",
      { 105, 100, 120, 120, 160, 140, 120, 120, 125, 130, 120, 150, 120, 170 },
      3, { "fear", "hide", "shadowwalk" }, {1, 20, 40}, {50, 50, 50},
      {  35,  35,  25,  10,  25,      20,  10,  40,  1,  1 },
      { 115, 115, 105,  90 , 105,     100, 90,  120, 80, 80 },   
      SIZE_MEDIUM, SEX_BOTH, 0
      },

	{
	"ogre",    "Ogre  ",
	{ 100, 120, 110, 130, 120, 145, 155, 135, 145, 150, 135, 155, 120, 120 },
	2, { "bash", "fast healing" }, {20, 10}, {80, 60},
	{  55,  35,  35,  10,  30,      15,  30,  25,  10,  20 },
	{ 115, 115, 105,  90,  90,      75,  90,  95,  90, 100 },   
	SIZE_HUGE, SEX_BOTH, 0
	},

	{
	"pixie",    "Pixie ",
	{ 140, 105, 110, 100, 155, 150, 150, 160, 140, 140, 135, 120, 140, 120 },
	2, { "faerie fog", "faerie fire" }, {5, 1}, {80, 90},
	{  15,  15,  40,  45,  30,      50,  30,  35,  35,  30 },
	{  75,  75, 100, 105, 100,     110, 120,  75, 105, 100 },   
	SIZE_TINY, SEX_BOTH, 0
	},

	{
	"saurin",    "Saurin",
	{ 100, 105, 105, 110, 120, 125, 125, 125, 125, 125, 130, 130, 120, 130 },
	1, { "venom bite" }, {3},   {60},
	{  40,  40,  40,  20,  15,      25,  50,  25,  40,  20 },
	{ 100, 105, 100, 100,  95,      85,  90, 105,  80, 100 },   
	SIZE_MEDIUM, SEX_BOTH, 0
	},

	{
	"troll",     "Troll ",
	{ 100, 110, 150, 120, 120, 140, 160, 125, 130, 160, 160, 135, 140, 120 },
	1, { "regeneration"}, {1}, {80},
	{  30,  55,  40,   1,  20,      20,  25,   1,   1,   5 },
	{ 110, 115, 120, 100, 100,     100,  85,  80,  80,  90 },   
	SIZE_LARGE, SEX_BOTH, 0
	},


	{
	"amazon",    "Amazon",
	{ 120, 130, 125, 135, 140, 140, 140, 150, 140, 145, 150, 155, 150, 140 },
	2, { "second attack", "war cry" },{10, 30},{50, 60},
	{  30,  30,  20,  30,  30,      30,  30,  25,  25,  30 },
	{ 100, 100, 110, 110, 110,     100, 100, 105, 105, 100 },   
	SIZE_MEDIUM, SEX_FEMALE, 1
	},

	{
	"centaur",    "Centau",
	{ 120, 140, 120, 130, 140, 150, 140, 155, 155, 145, 140, 140, 140, 140 },
	3, { "endurance", "kick", "lore" },{1, 5, 10},{90, 80, 60},
	{  35,  30,  35,  15,  30,      20,  25,  40,  20,  20 },
	{ 105, 110, 125,  75, 110,     100, 105, 110, 100, 105 },   
	SIZE_LARGE, SEX_BOTH, 1
	},

	{
	"khan",    "Khan  ",
	{ 120, 120, 125, 135, 140, 140, 155, 140, 145, 150, 155, 150, 160, 140 },
	3, { "hunt", "hide", "tiger" },{40, 5, 1},{60, 90, 50 },
	{  25,  20,  20,  35,  25,      20,  20,  25,  25,  20 },
	{ 105, 120, 120, 115,  95,      90, 100, 105, 105, 100 },   
	SIZE_SMALL, SEX_MALE, 1
	},

      {
	"nymph",    "Nymph ",
	{ 135, 135, 120, 120, 145, 150, 145, 145, 140, 150, 140, 150, 155, 145},
	3, { "faerie fire", "charm person", "tree golem" }, {10,35,70}, {85,50,60},
	{  40,  60,  35,  55,  40,      25,  15,  40,  15,  60 },
	{  85,  85, 100,  90, 100,     120, 115, 110, 120,  85 },   
	SIZE_MEDIUM, SEX_FEMALE, 1
	},

	{
	"satyr",    "Satyr ",
	{ 125, 120, 120, 120, 150, 145, 145, 155, 145, 145, 140, 140, 150, 140 },
	4,{"colour spray","sleep","charm person","cure serious"},{10,5,15,20},{60,60,60,60},
	{  15,  20,  35,  20,  40,      30,  20,  20,  25,  30 },
	{  95, 100, 105,  90, 120,     110, 120,  90, 125, 110 },   
	SIZE_MEDIUM, SEX_BOTH, 1
	},

      {
	"zombie",    "Zombie",
	{ 120, 120, 135, 135, 140, 145, 150, 140, 145, 145, 155, 150, 145, 145},
	3, { "bite", "giant strength", "animate dead" }, {10,35,70}, {75,65,50},
	{  40,  45,  35,  40,  60,      25,  35,  15,  60,  60 },
	{ 110, 120,  95,  85,  85,     120, 100, 115,  85,  85 },   
	SIZE_MEDIUM, SEX_BOTH, 1
	},     
	
	{
	"cyclops",    "Cyclop",
	{ 140, 170, 160, 165, 160, 170, 185, 180, 185, 195, 175, 200, 200, 200 },
	4, { "bash", "enhanced damage", "curse", "lightning bolt" },{5,30,60,20},{80,50,90,60},
	{  60,  60,  55,  20,  20,      20,  25,  35,  20,  30 },
	{ 130, 120, 115, 100, 100,     100, 105, 115,  90, 110 },   
	SIZE_HUGE, SEX_BOTH, 2
	},

	{
	"dunedain",    "Dunedn",
	{ 140, 150, 140, 140, 165, 160, 160, 170, 160, 160, 160, 165, 160, 160 },
	3, { "hunt", "sustenance", "lore" },{55, 10, 1},{75, 50, 60},
	{  30,  30,  30,  30,  30,      30,  30,  30,  30,  30 },
	{ 110, 110, 110, 110, 110,     110, 110, 110, 110, 110 },   
	SIZE_MEDIUM, SEX_BOTH, 2
	},

	{
	"highelf",    "Hi Elf",
	{ 160, 175, 140, 140, 190, 170, 165, 185, 175, 160, 160, 165, 160, 160 },
	4, { "pacify", "minor group heal", "rescue", "sanctuary" },{50, 40, 5, 80},{60, 60, 50, 33},
	{  30,  25,  25,  40,  40,      30,  25,  30,  50,  30 },
	{ 100,  95, 105, 110, 115,     120, 115, 110, 110, 110 },   
	SIZE_MEDIUM, SEX_BOTH, 2
	},

	{
	"mantis",    "Mantis",
	{ 155, 140, 140, 145, 170, 175, 160, 160, 165, 170, 170, 165, 180, 175},
	4, { "venom bite", "kick", "kung fu", "dual wield" }, {1,10,35,50}, {60,90,85,75},
	{  20,  15,  20,  45,  40,      30,  15,   20,  15,  70 },
	{ 110, 115, 105, 120, 120,     100, 115, 130,  90, 110 },   
	SIZE_MEDIUM, SEX_NEUTRAL, 2
	},
	
	{
	"martian",    "Martan",
	{ 140, 150, 150, 145, 160, 165, 170, 180, 175, 175, 180, 175, 160, 165},
	4, { "guns", "farsight", "ray of truth", "aim" }, {10,25,40,70}, {90,85,85,75},
	{  30,  20,  15,   5,  20,      50,  65,  80,  15,  15 },
	{ 110, 100, 115, 105, 100,     130, 125, 120,  75, 115 },   
	SIZE_MEDIUM, SEX_NEUTRAL, 2
	},          

	{
	"skaven",   "Skaven",
	{ 145, 140, 165, 160, 170, 195, 200, 160, 170, 190, 195, 170, 180, 180 },
	5, {"necrosis", "steal", "feint", "envenom", "sneak"},{30,25,2,10,1},{45,50,90,60,100},
	{  20,  20,  15,  65,  50,      30,  15,   5,  15,  80 },
	{ 100, 100, 115, 125, 130,     110, 115, 105,  75, 120 },   
	SIZE_MEDIUM, SEX_BOTH, 2
	},


      { 
	"draconian",   "Dracon",
	{ 160, 175, 180, 160, 185, 185, 195, 180, 185, 195, 180, 190, 190, 195 },
	3, {"regeneration", "fireball", "haste"},{20,35,60},{75,85,95},
	{  50,  10,  50,  50,  40,      45,  40,  10,  60,  60 },
	{ 125, 120, 120, 115, 105,     120, 120, 110,  90, 100 },   
	SIZE_LARGE, SEX_MALE, 3
	},
      
      {
	"drider",    "Drider",
	{ 170, 160, 180, 165, 185, 180, 185, 190, 180, 190, 185, 190, 195, 200 },
	5, { "venom bite", "faerie fire", "net", "second attack", "third attack" },{15,30,50,65,80},{95,90,90,85,80},
	{  10,  50,  40,  40,  45,      10,  60,  50,  60,  50 },
	{ 110, 115, 120, 110, 135,     110,  90, 115, 100, 120 },   
	SIZE_HUGE, SEX_BOTH, 3
	},

       {
	"phreak",    "Phreak",
	{ 160, 160, 160, 160, 180, 180, 180, 180, 180, 180, 180, 180, 180, 180 },
	5, { "change sex", "giant strength", "plague", "fear", "epidemic" }, {10,20,35,50,75}, {90,90,85,85,80},
	{   5,   5,   5,   5,   5,       5,   5,   5,   5,   5 },
	{ 125, 125, 125, 125, 125,     125, 125, 125, 125, 125 },   
	SIZE_MEDIUM, SEX_BOTH, 3
	},

     {    
	"sprite",    "Sprite",
	{ 190, 165, 170, 160, 210, 185, 180, 185, 190, 200, 195, 180, 200, 195 },
	5, {"detect astral", "dodge", "goodberry", "teleport", "astral projection"},{5,10,25,45,65},{95,95,90,90,85},
	{  60,  10,  60,  10,  50,      50,  50,  60,  45,  45 },
	{  90, 105, 115, 120, 130,     130, 115, 110, 125, 135 },   
	SIZE_SMALL, SEX_BOTH, 3
	},

      { 
	"vampil",   "Vampil",
	{ 165, 160, 170, 185, 180, 195, 185, 180, 185, 180, 185, 185, 185, 195 },
	4, {"sneak", "vampiric bite", "charm person", "energy drain"},{20,35,50,70},{90,85,75,60},
	{  50,  10,  30,  50,  40,      20,  40,  10,  60,  60 },
	{ 115, 125,  90, 115, 125,     120, 110, 130, 120, 105 },   
	SIZE_MEDIUM, SEX_BOTH, 3
	}


