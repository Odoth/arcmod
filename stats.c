/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.    *
 ***************************************************************************/

/***************************************************************************
*   ROM 2.4 is copyright 1993-1996 Russ Taylor             *
*   ROM has been brought to you by the ROM consortium          *
*       Russ Taylor (rtaylor@efn.org)                  *
*       Gabrielle Taylor                           *
*       Brian Moore (zump@rom.org)                     *
*   By using this code, you have agreed to follow the terms of the     *
*   ROM license, in the file Rom24/doc/rom.license             *
***************************************************************************/

/* ARCmod 1.0 by Mike Smullens 1998 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

void take_default_stats args((CHAR_DATA *ch));
void get_random_stats args((CHAR_DATA *ch));
void roll_dice args((CHAR_DATA *ch));
void calc_stats args((CHAR_DATA *ch));
bool parse_roll_stats args((CHAR_DATA *ch,char *argument));
bool parse_stat_priority args((CHAR_DATA *ch, char *argument));
void do_help args((CHAR_DATA *ch, char *argument));


/* command for retrieving stats */
int get_curr_stat( CHAR_DATA *ch, int stat )
{
	int max = UMIN(ch->perm_stat[stat]+20,MAX_CURRSTAT);
  
	return URANGE(1,ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

int dice_lookup(char *stat)
{
	int i;

	for (i=0; stat_table[i].name!=NULL; i++)
		if (!str_prefix(stat, stat_table[i].name))
			break;

	return i;
}

int stat_lookup(char *stat)
{
	int i;

	for (i=0; i<MAX_STATS; i++)
		if (!str_prefix(stat, stat_table[i].name))
			break;

	return -1;
}

int remort_bonus (CHAR_DATA *ch, int stat )
{
	int r;

	if (ch->pcdata->remorts ==0) return 0;

	r=ch->pcdata->remorts - pc_race_table[ch->race].remorts;
	r*=2;
	
	return ((r*ch->pcdata->history_stats[stat])/(30*ch->pcdata->remorts));
}

int class_bonus( int class, int stat )
{
	if (class_table[class].attr_prime == stat)
		return 10;

	if (class_table[class].attr_second[0] == stat
			|| class_table[class].attr_second[1] == stat)
		return 5;

	return 0;
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat )
{
	int max;

	if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
	return MAX_CURRSTAT;

	max = pc_race_table[ch->race].max_stats[stat];
	max += ch->pcdata->original_stats[stat];
	max += class_bonus(ch->class, stat);
	max += remort_bonus(ch, stat)/3;
	max /= 2;

	return UMIN(max,MAX_CURRSTAT);
}

int dex_app_tohit(int x)
{
	return ((x-60)/8);
};

int str_app_todam(int x)
{
	return ((x-60)/6);
};

int str_app_carry(int x)
{
	if (x>100) return ((x-90)*((x*(x+40))/500));
	return ((x*(x+40))/50);
};

int str_app_wield(int x)
{
	if (x<60) return (x*2);
	if (x<90) return (x*3-60);
	if (x<120) return (x*5-240);
	return (x*8-600);
};

int int_app_learn(int x)
{
	if (x<90) return ((2*x)/3);
	return (x-60); 
};

int dis_app_practice(int x)
{
	return (x/16);
};

int agi_app_defensive(int x)
{
	if (x<80) return (60-x);
	if (x<120) return (140-2*x);
	return (620-6*x);
};

int dex_app_extrahit(int x)
{
	if (x<100) return ((x-60)/5);
	return (x/2-42);
};

int con_app_shock(int x)
{
	return (100 - ((160-x)*(160-x))/256);
};

int con_app_hitp(int x)
{
	return ((x-50)/8);
};

int cha_app_aggro(int x)
{
	return ((x-60)/5);
}

int int_app_field(int x)
{
	return (x/3+2);
}

int wis_app_field(int x)
{
	return (75-(5*x)/16);
}

int dis_app_field(int x)
{
	return (x*10);  
}

int ch_dex_tohit(CHAR_DATA *ch)
{
	return (((ch->level+30)*dex_app_tohit(get_curr_stat(ch, STAT_DEX)))/30);
};

int ch_str_todam(CHAR_DATA *ch)
{
	return (((ch->level+30)*str_app_todam(get_curr_stat(ch, STAT_STR)))/30);
};

int ch_str_carry(CHAR_DATA *ch)
{
	return (str_app_carry(get_curr_stat(ch, STAT_STR)));
};

int ch_str_wield(CHAR_DATA *ch)
{
	return(str_app_wield(get_curr_stat(ch, STAT_STR)));
};

int ch_int_learn(CHAR_DATA *ch)
{
	return (int_app_learn(get_curr_stat(ch, STAT_INT)));
};

int ch_dis_practice(CHAR_DATA *ch)
{
	return (dis_app_practice(get_curr_stat(ch, STAT_DIS)));
};

int ch_agi_defensive(CHAR_DATA *ch)
{
	return (agi_app_defensive(get_curr_stat(ch, STAT_AGI)));
};

int ch_dex_extrahit(CHAR_DATA *ch)
{
	return (dex_app_extrahit(get_curr_stat(ch, STAT_DEX)));
};

int ch_con_shock(CHAR_DATA *ch)
{
	return (con_app_shock(get_curr_stat(ch, STAT_CON)));
};

int ch_con_hitp(CHAR_DATA *ch)
{
	return (con_app_hitp(get_curr_stat(ch, STAT_CON)));
};

int ch_cha_aggro(CHAR_DATA *ch)
{
	return ( cha_app_aggro(get_curr_stat(ch, STAT_CHA)));
}

int ch_int_field(CHAR_DATA *ch)
{
	return ( int_app_field(get_curr_stat(ch, STAT_INT)));
}

int ch_wis_field(CHAR_DATA *ch)
{
	return ( wis_app_field(get_curr_stat(ch, STAT_WIS)));
}

int ch_dis_field(CHAR_DATA *ch)
{
	return ( dis_app_field(get_curr_stat(ch, STAT_DIS)));
}

int ch_luc_quest(CHAR_DATA *ch)
{
	return (get_curr_stat(ch, STAT_LUC)/6+2);
}

void do_train( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *mob;
   sh_int stat = - 1;
   int cost, max;
   
   if ( IS_NPC(ch) )
	  return;
   
	  /*
	  * Check for trainer.
   */
   for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
   {
	  if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
		 break;
   }
   
   if ( mob == NULL )
	{
	if (get_skill(ch, gsn_introspection) > 1)
	{
	act( "$n thinks over what $e has experienced recently.", ch, NULL, NULL, TO_ROOM );
	if ((get_skill(ch,gsn_introspection)) > number_percent ()) 
		check_improve(ch,gsn_introspection,TRUE,8);
		else
		{
		send_to_char("You've learned nothing from your recent experiences.\n\r",ch);     
		check_improve(ch,gsn_introspection,FALSE,8);
		return;
		}
	}    
		else
			{
			send_to_char( "You can't do that here.\n\r", ch );
			return;
			}
	}
   
   if ( argument[0] == '\0' )
   {
	  sprintf( buf, "You have %d training sessions.\n\r", ch->train );
	  send_to_char( buf, ch );
	  argument = "foo";
   }
   
   cost = 1;

   if ( !str_cmp(argument, "hp" ) )
	  cost = 1;
   
   else if ( !str_cmp(argument, "mana" ) )
	  cost = 1;

   else   
   for (stat=0; stat<MAX_STATS; stat++)
	if (!str_prefix(argument, stat_table[stat].name))
	{
		cost=1;
		break;
	} 

   if (stat==MAX_STATS)
   {
	  strcpy( buf, "You can train:" );
	for (stat=0; stat<MAX_STATS; stat++)
		if ( ch->perm_stat[stat] < get_max_train(ch,stat))
		{
			strcat( buf, " "); 
				strcat( buf, stat_table[stat].abbreviation );
		}
	  if (ch->pcdata->perm_hit<MAX_PC_HP+ch->pcdata->remorts*500) strcat( buf, " hp");
	if (ch->pcdata->perm_mana<MAX_PC_MANA+ch->pcdata->remorts*500) strcat( buf, " mana");
	  
	  if ( buf[strlen(buf)-1] != ':' )
	  {
		 strcat( buf, ".\n\r" );
		 send_to_char( buf, ch );
	  }
	  else
	  {
	  /*
	  * This message dedicated to Jordan ... you big stud!
		 */
		 act( "You have nothing left to train, you $T!",
			ch, NULL,
			ch->sex == SEX_MALE   ? "big stud" :
		 ch->sex == SEX_FEMALE ? "hot babe" :
		 "wild thing",
			TO_CHAR );
	  }
	  
	  return;
   }
   
   if (!str_cmp("hp",argument))
   {
	  if ( cost > ch->train )
	  {
		 send_to_char( "You don't have enough training sessions.\n\r", ch );
		 return;
	  }

	  if (ch->pcdata->perm_hit>=MAX_PC_HP+ch->pcdata->remorts*500)
	{
	   send_to_char( "You have as many hps as you possibly can, you freak.\n\r", ch);
	   return;
	}
	 
	  ch->train -= cost;
	  ch->pcdata->perm_hit = UMIN(ch->pcdata->perm_hit + 10, MAX_PC_HP+ch->pcdata->remorts*500);
	  ch->max_hit += 10;
	  ch->hit +=10;
	  act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
	  act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
	  return;
   }
   
   if (!str_cmp("mana",argument))
   {
	  if ( cost > ch->train )
	  {
		 send_to_char( "You don't have enough training sessions.\n\r", ch );
		 return;
	  }
	  
	  if (ch->pcdata->perm_mana>=MAX_PC_MANA+ch->pcdata->remorts*500)
	{
	   send_to_char( "You have as much mana as you possibly can, you freak.\n\r", ch);
	   return;
	}
	 
	  ch->pcdata->perm_mana = UMIN(ch->pcdata->perm_mana + 10, MAX_PC_MANA+ch->pcdata->remorts*500);
	  ch->train -= cost;
	  ch->max_mana += 10;
	  ch->mana += 10;
	  act( "Your power increases!",ch,NULL,NULL,TO_CHAR);
	  act( "$n's power increases!",ch,NULL,NULL,TO_ROOM);
	  return;
   }
   
   if ( ch->perm_stat[stat]  >= (max=get_max_train(ch,stat)) )
   {
	  act( "Your $T is already at maximum.", ch, NULL, stat_table[stat].name, TO_CHAR );
	  return;
   }
   
   if ( cost > ch->train )
   {
	  send_to_char( "You don't have enough training sessions.\n\r", ch );
	  return;
   }
   
   ch->train       -= cost;

   max-=ch->perm_stat[stat];

   if (max>45)
	cost=5;
   else if (max>21)
	cost=4;
   else if (max>9)
	cost=3;
   else if (max>3)
	  cost=2;
   else
	cost=1;

   cost=UMIN(1 + ch->perm_stat[stat]/15, cost); 

   ch->perm_stat[stat]     += cost;
   act( "Your $T increases!", ch, NULL, stat_table[stat].name, TO_CHAR );
   act( "$n's $T increases!", ch, NULL, stat_table[stat].name, TO_ROOM );
   return;
}

void compute_mob_stats(CHAR_DATA *mob)
{
	sh_int base, i;
	
	if (mob->level<90) base = 12+(2*mob->level)/3;
	else base = 2*mob->level-108;

	for (i = 0; i < MAX_STATS; i ++)
		mob->perm_stat[i] = base + number_range(1,16);
		
	if (IS_SET(mob->act,ACT_WARRIOR))
	{
		mob->perm_stat[STAT_STR] += 15;
		mob->perm_stat[STAT_VIT] += 8;
		mob->perm_stat[STAT_INT] -= 6;
		mob->perm_stat[STAT_CON] += 10;
	}
	else
	if (IS_SET(mob->act,ACT_THIEF))
	{
		mob->perm_stat[STAT_DEX] += 12;
		mob->perm_stat[STAT_AGI] += 15;
		mob->perm_stat[STAT_INT] += 4;
		mob->perm_stat[STAT_WIS] -= 6;
	}
	else
	if (IS_SET(mob->act,ACT_CLERIC))
	{
		mob->perm_stat[STAT_WIS] += 15;
		mob->perm_stat[STAT_DIS] += 4;
		mob->perm_stat[STAT_CHA] += 7;
		mob->perm_stat[STAT_AGI] -= 4;
	}
	else
	if (IS_SET(mob->act,ACT_MAGE))
	{
		mob->perm_stat[STAT_INT] += 15;
		mob->perm_stat[STAT_STR] -= 8;
		mob->perm_stat[STAT_CON] -= 4;
		mob->perm_stat[STAT_AGI] += 5;
		mob->perm_stat[STAT_WIS] += 3;
	}
	
	if (IS_SET(mob->off_flags,OFF_FAST))
		mob->perm_stat[STAT_DEX] += 10;
		
	mob->perm_stat[STAT_STR] += (mob->size - SIZE_MEDIUM)*5;
	mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) *3;

	for (i = 0; i < MAX_STATS; i ++)
		mob->perm_stat[i] = URANGE(5,mob->perm_stat[i],MAX_CURRSTAT);

	return;
}

void do_stats( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int i, j;
   BUFFER *output;

   output = new_buf();

	add_buf(output, "===Race=== =Str== =Con== =Vit== =Agi== =Dex==");
	add_buf(output, " =Int== =Wis== =Dis== =Cha== =Luc==");
	
	for (i=1; i<MAX_PC_RACE; i++)
	{
		sprintf(buf, "\n\r%10s", pc_race_table[i].name);
	  	add_buf(output,buf);

		for (j=0; j<MAX_STATS; j++)
		{
			sprintf(buf, " %2d-%3d", pc_race_table[i].min_stats[j],
							pc_race_table[i].max_stats[j]);
	 		add_buf(output,buf);
		}
	}
	add_buf(output,"\n\r\n\rTo get more information, type: HELP STATS\n\r");

   page_to_char(buf_string(output),ch);
   free_buf(output);

	return;
}

void do_etls( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int i, j;
   BUFFER *output;

   output = new_buf();

	add_buf(output, "===Race=== ");

	for (i=0; i<MAX_CLASS; i++)
	{
		sprintf(buf, " %3s ", class_table[i].who_name);
		add_buf(output, buf);
	}

	for (i=1; i<MAX_PC_RACE; i++)
	{
		sprintf(buf, "\n\r%10s", pc_race_table[i].name);
		add_buf(output, buf);  
		
		for (j=0; j<MAX_CLASS; j++)
		{
			sprintf(buf, " %4d", 10*pc_race_table[i].class_mult[j]);
			add_buf(output, buf);
		}
	}

	add_buf(output,"\n\r\n\rTo get more information, type: HELP ETLS\n\r");

   page_to_char(buf_string(output),ch);
   free_buf(output);

	return;
}

void roll_dice (CHAR_DATA *ch)
{
	int i, j, swap;
	
	ch->gen_data->unused_die[0]=number_range(90,100);
	ch->gen_data->unused_die[1]=number_range(85,100);
	ch->gen_data->unused_die[2]=number_range(80,100);
	ch->gen_data->unused_die[3]=number_range(70,100);
	ch->gen_data->unused_die[4]=number_range(60,100);
	for (i=5; i<12; i++)
		ch->gen_data->unused_die[i]=number_range(15,85);
	ch->gen_data->unused_die[12]=number_range(0, 50);
	ch->gen_data->unused_die[13]=number_range(0, 40);
	ch->gen_data->unused_die[14]=number_range(0, 20);

	for (i=0; i<14; i++)
		for (j=i+1; j<15; j++)
			if (ch->gen_data->unused_die[j]>ch->gen_data->unused_die[i])
			{
				swap = ch->gen_data->unused_die[j];
				ch->gen_data->unused_die[j]=ch->gen_data->unused_die[i];
				ch->gen_data->unused_die[i]=swap;
			}

	for (i=0; i<15; i++)
		ch->gen_data->assigned_die[i]=-1;
	
	return;
}

void calc_stats(CHAR_DATA *ch)
{
	int i, j;
	int race=ch->race;
	long int stat, range;
	
	for (i=0; i<MAX_STATS; i++)
		ch->perm_stat[i]=pc_race_table[race].min_stats[i]+class_bonus(ch->class,i)
			+remort_bonus(ch, i);

	if (ch->pcdata->true_sex==SEX_MALE)
	{
		ch->perm_stat[STAT_AGI]-=5;
		ch->perm_stat[STAT_STR]+=5;
	}
	else if (ch->pcdata->true_sex==SEX_FEMALE)
	{
		ch->perm_stat[STAT_STR]-=5;
		ch->perm_stat[STAT_AGI]+=5;
	}

	ch->perm_stat[STAT_WIS]+=ch->alignment/125;
	ch->perm_stat[STAT_INT]-=ch->alignment/125;
	ch->perm_stat[STAT_LUC]+=ch->alignment/125;
	ch->perm_stat[STAT_CHA]-=ch->alignment/125;

	for (i=0; i<MAX_STATS; i++)
	{
		stat=0;
		for (j=0; stat_table[j].name!=NULL; j++)
			stat += stat_table[j].dice[i] *
			UMAX(ch->gen_data->assigned_die[j],0);
		range = pc_race_table[race].max_stats[i] - pc_race_table[race].min_stats[i];    
		if (ch->pcdata->true_sex==SEX_MALE)
		{
			if (i==STAT_STR) range-=5;
			else if (i==STAT_AGI) range+=5;
		}
		else if (ch->pcdata->true_sex==SEX_FEMALE)
		{
			if (i==STAT_AGI) range-=5;
			else if (i==STAT_STR) range+=5;
		}
		if (i==STAT_WIS)
			range-=ch->alignment/125;
		else if (i==STAT_INT)
			range+=ch->alignment/125;
		else if (i==STAT_CHA)
			range+=ch->alignment/125;
		else if (i==STAT_LUC)
			range-=ch->alignment/125;
		range-=(2*remort_bonus(ch, i))/3;
		stat*=range;
		stat/=10000;
		ch->perm_stat[i] = URANGE(1, ch->perm_stat[i] + stat, MAX_CURRSTAT);
		ch->pcdata->original_stats[i]=ch->perm_stat[i];
	}

	return;
}

void get_random_stats(CHAR_DATA *ch)
{
	int i,j, swap;

	roll_dice(ch);
	
	for (i=0; stat_table[i].name!=NULL; i++)
	{
		ch->gen_data->assigned_die[i]=0;
		for (j=0; j<MAX_STATS; j++)
			ch->gen_data->assigned_die[i]+=(MAX_STATS-j) *
				stat_table[i].dice[ch->gen_data->stat_priority[j]];
	}

	for (i=0; stat_table[i].name!=NULL; i++)
		ch->gen_data->stat_priority[i]=i;

	for (i=0; i<14; i++)
		for (j=i+1; j<15; j++)
			if (ch->gen_data->assigned_die[j]>ch->gen_data->assigned_die[i])
			{
				swap = ch->gen_data->assigned_die[j];
				ch->gen_data->assigned_die[j]=ch->gen_data->assigned_die[i];
				ch->gen_data->assigned_die[i]=swap;
				swap = ch->gen_data->stat_priority[j];
				ch->gen_data->stat_priority[j]=ch->gen_data->stat_priority[i];
				ch->gen_data->stat_priority[i]=swap;
			}

	for (i=0; i<15; i++)
		ch->gen_data->assigned_die[ch->gen_data->stat_priority[i]]=
			ch->gen_data->unused_die[i];

	calc_stats(ch);

	return;
}

void take_default_stats(CHAR_DATA *ch)
{
	int i;
	ch->gen_data = new_gen_data();
	ch->gen_data->stat_priority[0]=class_table[ch->class].attr_prime;
	ch->gen_data->stat_priority[1]=class_table[ch->class].attr_second[0];
	ch->gen_data->stat_priority[2]=class_table[ch->class].attr_second[1];
	for (i = 3; i<MAX_STATS; i++)
		ch->gen_data->stat_priority[i]=class_table[ch->class].stat_priority[i-3];

	get_random_stats(ch);

	free_gen_data(ch->gen_data);
	ch->gen_data=NULL;
	
	return;
}

bool parse_stat_priority (CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[100];
	int i,j;
 
	argument = one_argument(argument,arg);

	if (arg[0] == '\0') return TRUE;

	if (!str_prefix(arg,"help"))
	{
	if (argument[0] == '\0')
	{
		do_help(ch,"priority header");
	} else do_help(ch,argument);
	}
	else if (!str_prefix(arg, "undo"))
	{
		for(i=0; i<MAX_STATS; i++)
			if (ch->gen_data->stat_priority[i]==-1) break;
		if (i==0)
		{
			send_to_char("There is nothing to undo.\n\r", ch);
			return TRUE;
		}
		ch->gen_data->unused_die[ch->gen_data->stat_priority[i-1]]=TRUE;
		ch->gen_data->stat_priority[i-1]=-1;
	}
	else
	{
		for (i=0; i<MAX_STATS; i++)
			if (!str_prefix(arg, stat_table[i].name)) break;
		if (i==MAX_STATS) return FALSE;
		if (!ch->gen_data->unused_die[i])
		{
			send_to_char("You've already assigned that.\n\r",ch);
			return TRUE;
		}
		for(j=0; j<MAX_STATS; j++)
			if (ch->gen_data->stat_priority[j]==-1) break;
		ch->gen_data->stat_priority[j]=i;
		ch->gen_data->unused_die[i]=FALSE;
	}

	strcpy(buf, "Statistics left to assign:");
	for (i=0; i<MAX_STATS; i++)
		if (ch->gen_data->unused_die[i])
		{
			strcat(buf, " ");           
			strcat(buf, stat_table[i].name);
		}

	strcat(buf, "\n\rCurrent statistic priority:");
	for (i=0; i<MAX_STATS; i++)
	{
		if (ch->gen_data->stat_priority[i]==-1) break;
		strcat(buf, " ");           
		strcat(buf, stat_table[ch->gen_data->stat_priority[i]].name);
	}

	strcat(buf, "\n\r");
	send_to_char(buf, ch);

	return TRUE;
}

bool parse_roll_stats (CHAR_DATA *ch,char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[100], buf2[100], buf3[5];
	int i,j,stat,die;
	int curr, train, min, max;
 
	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	if (arg[0] == '\0') return TRUE;

	if (!str_prefix(arg,"help"))
	{
	if (arg2[0] == '\0')
	{
		do_help(ch,"rollstat header");
		return TRUE;
	}

		do_help(ch,arg2);
	  return TRUE;
	}
	else if (!str_prefix(arg, "reroll"))
	{
		roll_dice(ch);
		calc_stats(ch);
	}
	else if (!str_prefix(arg, "unassign"))
	{
		if (arg2[0]=='\0'||(stat=dice_lookup(arg2))==15)
		{
			do_help(ch,"unassign");
			return TRUE;
		}
		if ((die=ch->gen_data->assigned_die[stat])==-1)
		{
			send_to_char("No die has been assigned to that attribute.\n\r",ch);
			return TRUE;
		}
		ch->gen_data->assigned_die[stat]=-1;
		calc_stats(ch);
		
		for(i=0; i<15; i++)
			if (die>ch->gen_data->unused_die[i]) break;
		
		for (j=14; j>i; j--)
			ch->gen_data->unused_die[j]=ch->gen_data->unused_die[j-1];
		ch->gen_data->unused_die[i] = die;  
	}
	else if (!str_prefix(arg, "assign"))
	{
		if (arg2[0]=='\0'||arg3[0]=='\0'||!is_number(arg3)
			||(stat=dice_lookup(arg2))==15)
		{
			do_help(ch, "assign");
			return TRUE;
		}
		if (ch->gen_data->assigned_die[stat]!=-1)
		{
			send_to_char("A die has already been assigned to that attribute.\n\r",ch);
			return TRUE;
		}
		die=atoi(arg3);
		for (i=0; i<15; i++)
			if (ch->gen_data->unused_die[i]==die)
				break;
		if (i==15||die==-1)
		{
			sprintf(buf, "You dont have any %d dice to assign.\n\r", die);
			send_to_char(buf,ch);
			return TRUE;
		}
		ch->gen_data->assigned_die[stat]=die;
		calc_stats(ch);
		for(j=i; j<14; j++)
			ch->gen_data->unused_die[j]=ch->gen_data->unused_die[j+1];
		ch->gen_data->unused_die[14]=-1;
	}
	else if (str_prefix(arg, "show")) return FALSE;
	
	send_to_char("\n\rStat  Curr  Train Die   Min-Max            Attribute\n\r", ch);
	for(i=0; i<10; i++)
	{
		curr=ch->perm_stat[i];
		min=pc_race_table[ch->race].min_stats[i];
		if (ch->pcdata->true_sex==SEX_MALE)
		{
			if (i==STAT_AGI) min=UMAX(1,min-5);
			else if (i==STAT_STR) min+=5;
		}
		else if (ch->pcdata->true_sex==SEX_FEMALE)
		{
			if (i==STAT_STR) min=UMAX(1,min-5);
			else if (i==STAT_AGI) min+=5;
		}
		max=pc_race_table[ch->race].max_stats[i];
		
		j=class_bonus(ch->class, i);
		min+=j;
		max+=j;

		j=remort_bonus(ch, i);
		min+=j;
		max+=j/3;

		if (i==STAT_WIS)
			min=UMAX(1, min+ch->alignment/125);
		else if (i==STAT_INT)
			min=UMAX(1, min-ch->alignment/125);
		else if (i==STAT_CHA)
			min=UMAX(1, min-ch->alignment/125);
		else if (i==STAT_LUC)
			min=UMAX(1, min+ch->alignment/125);

		if (i>4) buf2[0]='\0';
		else
		{
			if (ch->gen_data->assigned_die[i+10]==-1)
				strcpy(buf3, " --");
			else
				sprintf(buf3, "%3d", ch->gen_data->assigned_die[i+10]);
			sprintf(buf2, "%10s %3s", stat_table[i+10].name, buf3);
		}

		if (ch->gen_data->assigned_die[i]==-1)
			strcpy(buf3, " --");
		else
			sprintf(buf3, "%3d", ch->gen_data->assigned_die[i]);
		train=(curr+max)/2;
		
		sprintf(buf, " %3s  %3d   %3d   %3s   %3d-%3d            %s\n\r",
			stat_table[i].abbreviation, curr, train, buf3, min, max, buf2);
		send_to_char(buf, ch);
	}

	strcpy(buf, "\n\rDice left to distribute: ");
	for (i=0; i<15; i++)
	{
		if ((j=ch->gen_data->unused_die[i])==-1) break;
		if (i>0) strcat(buf, ", ");
		sprintf(buf3, "%d", j);
		strcat(buf, buf3);
	}
	strcat(buf, "\n\r");
	send_to_char(buf, ch);

	return TRUE;
}

