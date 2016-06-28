/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,    *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                     *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael      *
 *  Chastain, Michael Quan, and Mitchell Tse.                  *
 *                                     *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                           *
 *                                     *
 *  Much time and thought has gone into this software and you are      *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                          *
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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_groups    );
DECLARE_DO_FUN(do_help      );
DECLARE_DO_FUN(do_say       );

/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *trainer;
   int gn = 0, sn = 0;
   bool introspect = FALSE;

   if (IS_NPC(ch))
	  return;

   /* find a trainer */
   for ( trainer = ch->in_room->people; trainer != NULL; trainer = trainer->next_in_room )
	if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
	   break;

   if (trainer == NULL || !can_see(ch,trainer))
     {
	if ((get_skill(ch,gsn_introspection)) > 1 && !IS_REMORT(ch))
	{
	  if ((get_skill(ch,gsn_introspection)) > number_percent() )
	  {
	     act( "$n thinks over what $e has experienced recently.",ch,NULL,NULL,TO_ROOM );
	     check_improve(ch,gsn_introspection,TRUE,8);
	     introspect = TRUE;
          }
	  else
	  {
	     send_to_char("You find nothing meaningful in your introspection.\n\r",ch);
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


	one_argument(argument,arg);
	if (arg[0] == '\0')
	{
	   if ( introspect )
	      send_to_char("See HELP GAIN.\n\r",ch);
	    else
	      do_say(trainer,"Pardon me?");
	   return;
	}
	else if (!str_prefix(arg,"list"))
	{
	   int col;
	   col = 0;
	   sprintf(buf, "%-16s    %-5s %-16s    %-5s %-16s    %-5s\n\r",
		 "group","cost","group","cost","group","cost");
	   send_to_char(buf,ch);
	   for (gn = 0; gn < MAX_GROUP; gn++)
	   {
	     if (group_table[gn].name == NULL)
	         break;
	     if (!ch->pcdata->group_known[gn]
		 &&  group_table[gn].rating[ch->class] > 0)
		{
		   sprintf(buf,"%-17s    %-5d ",
			group_table[gn].name,
			group_table[gn].rating[ch->class]);
		   send_to_char(buf,ch);
		   if (++col % 3 == 0)
		      send_to_char("\n\r",ch);
		}
	   }
	   if (col % 3 != 0)
	     send_to_char("\n\r",ch);
	   send_to_char("\n\r",ch);
	   col = 0;
	   sprintf(buf, "%-16slvl/%-5s %-16slvl/%-5s %-16slvl/%-5s\n\r",
	                "skill","cost","skill","cost","skill","cost");
	   send_to_char(buf,ch);
	   for (sn = 0; sn < MAX_SKILL; sn++)
	   {
		if (skill_table[sn].name == NULL)
			break;
		if (!ch->pcdata->learned[sn]
			 &&  skill_table[sn].rating[ch->class] > 0
			 &&  skill_table[sn].spell_fun == spell_null)
			  {
			  sprintf(buf,"%-17s%2d/%-5d ",
			     skill_table[sn].name,
			     skill_table[sn].skill_level[ch->class],
			     skill_table[sn].rating[ch->class]);
			  send_to_char(buf,ch);
			  if (++col % 3 == 0)
				  send_to_char("\n\r",ch);
			  }
		}
		if (col % 3 != 0)
		  send_to_char("\n\r",ch);
		return;
	   }
	else if (!str_cmp(arg,"convert"))
	{
		  if (ch->practice < 10)
		  {
		    if ( introspect )
		       send_to_char("You are not ready.\n\r",ch);
		    else
		       act("$N tells you 'You are not yet ready.'",
		           ch,NULL,trainer,TO_CHAR );
		    return;
		  }
    		  if ( introspect )
		       send_to_char("You apply your practice to training.\n\r",ch);
		   else
		       act("$N helps you apply your practice to training.",
		           ch,NULL,trainer,TO_CHAR );
	   ch->practice -= 10;
	   ch->train +=1 ;
	   return;
	}
	else if (!str_cmp(arg,"revert"))
	{
	   if (ch->train < 1)
	   {
	      if ( introspect )
		send_to_char("You are not ready.\n\r",ch);
	      else
	        act("$N tells you 'You are not yet ready.'",
	             ch,NULL,trainer,TO_CHAR);
		return;
	   }
	   if ( introspect )
	      send_to_char("You apply your training to practices.\n\r",ch);   
	   else
	      act("$N helps you apply your training to practices.",
	         ch,NULL,trainer,TO_CHAR);
	   ch->train -=1 ;
	   ch->practice += 8;
	   return;
	}
	else if (!str_cmp(arg,"points"))
	{
	   if (ch->train < 2)
	   {
		  if ( introspect )
		     send_to_char("You are not ready.\n\r",ch);
		   else
		     act("$N tells you 'You are not yet ready.'",
		          ch,NULL,trainer,TO_CHAR );
		  return;
	   }
	   if (ch->pcdata->points <= 50)
	   {
		  if ( introspect )
		     send_to_char("There would be no point in that.\n\r",ch);
		  else
		     act("$N tells you 'There would be no point in that.'",
		        ch,NULL,trainer,TO_CHAR );
		  return;
	   }
	   if ( introspect )
	      send_to_char("You train, feeling more at ease with your skills.\n\r",ch);
	   else
	     act("$N trains you, and you feel more at ease with your skills.",
	        ch,NULL,trainer,TO_CHAR);
	   ch->train -= 2;
	   ch->pcdata->points -= 1;
		   ch->exp = exp_per_level(ch,ch->pcdata->points) * ch->level;
		   return;
	}
	else if (!str_cmp(arg, "swaphp"))
	{
	   if (ch->pcdata->perm_hit <= 100)
	   {
		  if ( introspect )
		     send_to_char("That wouldn't be prudent.\n\r",ch);             
		   else
		     act("$N does not think that would be prudent.",
		          ch,NULL,trainer,TO_CHAR );
		  return;
	   }
	   if ( introspect )
	      send_to_char("You transfer some hit points to mana.\n\r",ch);   
	   else
	      act( "$N channels power from the gods, and transfers some of your hit points to mana.",
	            ch,NULL,trainer,TO_CHAR );
	   ch->hit = UMAX(1,ch->hit-15);
	   ch->mana += 10;
	   ch->max_hit -= 15;
	   ch->max_mana += 10;
	   ch->pcdata->perm_hit -= 15;
	   ch->pcdata->perm_mana += 10;
	   return;
	}
	else if (!str_cmp(arg, "swapmana"))
	{
	   if (ch->pcdata->perm_mana <= 100)
	   {
	   if ( introspect )
	      send_to_char("That wouldn't be prudent.\n\r",ch);
	   else
	      act( "$N does not think that would be prudent.",
	            ch,NULL,trainer,TO_CHAR );
	   return;
	   }
	   if ( introspect )
	      send_to_char("You transfer some mana to hit points.\n\r",ch);
	   else
	      act( "$N channels power from the gods, and transfers some of your mana to hit points.",
	         ch,NULL,trainer,TO_CHAR );
	   ch->mana = UMAX(1,ch->mana-15);
	   ch->hit += 10;
	   ch->max_mana -= 15;
	   ch->max_hit += 10;
	   ch->pcdata->perm_mana -= 15;
	   ch->pcdata->perm_hit += 10;
	   return;
	}
	else if (!str_cmp(arg, "losehp"))
	{
	   if (ch->pcdata->perm_hit <= 100)
	   {
		  if ( introspect )
		     send_to_char("That wouldn't be prudent.\n\r",ch);
		  else
		     act( "$N does not think that would be prudent.",
		          ch,NULL,trainer,TO_CHAR );
		  return;
	   }
	   if ( introspect )
	      send_to_char("You transfer some hit points to 1 train.\n\r",ch);
	   else
	      act( "$N channels power from the gods, and transfers some of your hit points to 1 train.",
	           ch,NULL,trainer,TO_CHAR );
	   ch->hit = UMAX(1,ch->hit-20);
	   ch->pcdata->perm_hit -= 20;
	   ch->max_hit -= 20;
	   ch->train += 1;
	   return;
	}
	else if (!str_cmp(arg, "losemana"))
	{
	   if (ch->pcdata->perm_mana <= 100)
	   {
		  if ( introspect )
		     send_to_char("That wouldn't be prudent.\n\r",ch);
		  else
		     act( "$N does not think that would be prudent.",
		           ch,NULL,trainer,TO_CHAR );
		  return;
	   }
	   if ( introspect )
	      send_to_char("You transfer some mana to 1 train.\n\r",ch);
	   else
	      act( "$N channels power from the gods, and transfers some of your mana to 1 train.",
	           ch,NULL,trainer,TO_CHAR );
	   ch->mana=UMAX(1,ch->mana-20);
	   ch->pcdata->perm_mana -= 20;
	   ch->max_mana -= 20;
	   ch->train += 1;
	   return;
	}
	else if ( (gn = group_lookup(argument)) > 0)
	{
	   if (ch->pcdata->group_known[gn])
	   {
		if ( introspect )
		  send_to_char("You already know that group!\n\r",ch);
		else
		  act( "$N tells you 'You already know that group!'",
		      ch,NULL,trainer,TO_CHAR );
		return;
	   }
	   if (group_table[gn].rating[ch->class] <= 0)
	   {
		if ( introspect )
		  send_to_char("That group is beyond your powers.\n\r",ch);
		else
		  act( "$N tells you 'That group is beyond your powers.'",
		       ch,NULL,trainer,TO_CHAR );
		return;
	   }
	   if (ch->train < group_table[gn].rating[ch->class])
	   {
		if ( introspect )
		  send_to_char("You aren't ready for that group.\n\r",ch);
		else
		  act ("$N tells you 'You are not yet ready for that group.'",
		      ch,NULL,trainer,TO_CHAR );
		return;
	   }
	   gn_add(ch,gn);
	   if ( introspect )
	      act("You learn the art of $t",
		  ch,group_table[gn].name,NULL,TO_CHAR);
	   else
	      act("$N trains you in the art of $t",
	          ch,group_table[gn].name,trainer,TO_CHAR );
	   ch->train -= group_table[gn].rating[ch->class];
	   return;
	}
	else if ((sn = skill_lookup(argument)) > -1)
	{
	   if (skill_table[sn].spell_fun != spell_null)
	   {
		if ( introspect )
		  send_to_char("You must learn the full group.\n\r",ch);
		else
		  act( "$N tells you 'You must learn the full group.'",
		     ch,NULL,trainer,TO_CHAR );
		return;
	   }
	   if (ch->pcdata->learned[sn])
	   {
		if ( introspect )
		  send_to_char("You already know that skill.\n\r",ch);
		else
		  act( "$N tells you 'You already know that skill!'",
		      ch,NULL,trainer,TO_CHAR );
		return;
	   }
			
	   if (skill_table[sn].rating[ch->class] <= 0)
	   {
		if ( introspect )
		  send_to_char("That skill is beyond your powers.\n\r",ch);
		else
		  act( "$N tells you 'That skill is beyond your powers.'",
		      ch,NULL,trainer,TO_CHAR );
		return;
	   }
	   if (ch->train < skill_table[sn].rating[ch->class])
	   {
		if ( introspect )
		  send_to_char("You aren't ready for that skill.\n\r",ch);
		else
		  act( "$N tells you 'You are not yet ready for that skill.'",
		      ch,NULL,trainer,TO_CHAR );
		return;
	   }
	   ch->pcdata->learned[sn] = 1;
	   if ( introspect )
	      act("You learn the art of $t.",
		  ch,skill_table[sn].name,NULL,TO_CHAR);
	   else
	      act("$N trains you in the art of $t.",
	          ch,skill_table[sn].name,trainer,TO_CHAR);
	   ch->train -= skill_table[sn].rating[ch->class];
	   return;
	}		
	if ( introspect )
	   send_to_char( "See HELP GAIN.\n\r", ch );
	else
	   act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
	
}

/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, char *argument)
{
	BUFFER *buffer;
	char arg[MAX_INPUT_LENGTH];
	char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
	char spell_columns[LEVEL_HERO + 1];
	int sn, level, min_lev = 1, max_lev = LEVEL_HERO, mana;
	bool fAll = FALSE, found = FALSE;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	  return;

	if (argument[0] != '\0')
	{
	fAll = TRUE;

	if (str_prefix(argument,"all"))
	{
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		send_to_char("Arguments must be numerical or all.\n\r",ch);
		return;
		}
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
		sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		send_to_char(buf,ch);
		return;
		}

		if (argument[0] != '\0')
		{
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
			send_to_char("Arguments must be numerical or all.\n\r",ch);
			return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
			sprintf(buf,
			"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
			send_to_char(buf,ch);
			return;
		}

		if (min_lev > max_lev)
		{
			send_to_char("That would be silly.\n\r",ch);
			return;
		}
		}
	}
	}


	/* initialize data */
	for (level = 0; level < LEVEL_HERO + 1; level++)
	{
		spell_columns[level] = 0;
		spell_list[level][0] = '\0';
	}

	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL )
		break;

	if ((level = skill_table[sn].skill_level[ch->class]) < LEVEL_HERO + 1
	&&  (fAll || level <= ch->level)
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun != spell_null
	&&  ch->pcdata->learned[sn] > 0)
		{
		found = TRUE;
		level = skill_table[sn].skill_level[ch->class];
		if (ch->level < level)
			sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
		else
		{
		mana = UMAX(skill_table[sn].min_mana,
			100/(2 + ch->level - level));
			sprintf(buf,"%-18s  %3d mana  ",skill_table[sn].name,mana);
		}

		if (spell_list[level][0] == '\0')
			sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
		else /* append */
		{
			if ( ++spell_columns[level] % 2 == 0)
			strcat(spell_list[level],"\n\r          ");
			strcat(spell_list[level],buf);
		}
	}
	}

	/* return results */

	if (!found)
	{
		send_to_char("No spells found.\n\r",ch);
		return;
	}

	buffer = new_buf();
	for (level = 0; level < LEVEL_HERO + 1; level++)
		if (spell_list[level][0] != '\0')
		add_buf(buffer,spell_list[level]);
	add_buf(buffer,"\n\r");
	page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
}

void do_skills(CHAR_DATA *ch, char *argument)
{
	BUFFER *buffer;
	char arg[MAX_INPUT_LENGTH];
	char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
	char skill_columns[LEVEL_HERO + 1];
	int sn, level, min_lev = 1, max_lev = LEVEL_HERO;
	bool fAll = FALSE, found = FALSE;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	  return;

	if (argument[0] != '\0')
	{
	fAll = TRUE;

	if (str_prefix(argument,"all"))
	{
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
		send_to_char("Arguments must be numerical or all.\n\r",ch);
		return;
		}
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
		sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
		send_to_char(buf,ch);
		return;
		}

		if (argument[0] != '\0')
		{
		argument = one_argument(argument,arg);
		if (!is_number(arg))
		{
			send_to_char("Arguments must be numerical or all.\n\r",ch);
			return;
		}
		min_lev = max_lev;
		max_lev = atoi(arg);

		if (max_lev < 1 || max_lev > LEVEL_HERO)
		{
			sprintf(buf,
			"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
			send_to_char(buf,ch);
			return;
		}

		if (min_lev > max_lev)
		{
			send_to_char("That would be silly.\n\r",ch);
			return;
		}
		}
	}
	}


	/* initialize data */
	for (level = 0; level < LEVEL_HERO + 1; level++)
	{
		skill_columns[level] = 0;
		skill_list[level][0] = '\0';
	}

	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL )
		break;

	if ((level = skill_table[sn].skill_level[ch->class]) < LEVEL_HERO + 1
	&&  (fAll || level <= ch->level)
	&&  level >= min_lev && level <= max_lev
	&&  skill_table[sn].spell_fun == spell_null
	&&  ch->pcdata->learned[sn] > 0)
		{
		found = TRUE;
		level = skill_table[sn].skill_level[ch->class];
		if (ch->level < level)
			sprintf(buf,"%-18s n/a      ", skill_table[sn].name);
		else
			sprintf(buf,"%-18s %3d%%      ",skill_table[sn].name,
			ch->pcdata->learned[sn]);

		if (skill_list[level][0] == '\0')
			sprintf(skill_list[level],"\n\rLevel %2d: %s",level,buf);
		else /* append */
		{
			if ( ++skill_columns[level] % 2 == 0)
			strcat(skill_list[level],"\n\r          ");
			strcat(skill_list[level],buf);
		}
	}
	}

	/* return results */

	if (!found)
	{
		send_to_char("No skills found.\n\r",ch);
		return;
	}

	buffer = new_buf();
	for (level = 0; level < LEVEL_HERO + 1; level++)
		if (skill_list[level][0] != '\0')
		add_buf(buffer,skill_list[level]);
	add_buf(buffer,"\n\r");
	page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
}

/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch)
{
	char buf[100];
	int gn,sn,col;

	if (IS_NPC(ch))
	return;

	col = 0;

	sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s\n\r","group","cp","group","cp","group","cp");
	send_to_char(buf,ch);

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
	if (group_table[gn].name == NULL)
		break;

		if (!ch->gen_data->group_chosen[gn]
	&&  !ch->pcdata->group_known[gn]
	&&  group_table[gn].rating[ch->class] > 0)
	{
		sprintf(buf,"%-18s %-5d ",group_table[gn].name,
					group_table[gn].rating[ch->class]);
		send_to_char(buf,ch);
		if (++col % 3 == 0)
		send_to_char("\n\r",ch);
	}
	}
	if ( col % 3 != 0 )
		send_to_char( "\n\r", ch );
	send_to_char("\n\r",ch);

	col = 0;

	sprintf(buf,"%-16s%-6s    %-16s%-6s    %-16s%-6s\n\r","skill","lvl/cp","skill","lvl/cp","skill","lvl/cp");
	send_to_char(buf,ch);

	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL)
			break;

		if (!ch->gen_data->skill_chosen[sn]
	&&  ch->pcdata->learned[sn] == 0
	&&  skill_table[sn].spell_fun == spell_null
	&&  skill_table[sn].rating[ch->class] > 0)
		{
			sprintf(buf,"%-16s %2d/%-5d ",
			   skill_table[sn].name,
			   skill_table[sn].skill_level[ch->class],
			   skill_table[sn].rating[ch->class]);
			send_to_char(buf,ch);
			if (++col % 3 == 0)
				send_to_char("\n\r",ch);
		}
	}
	if ( col % 3 != 0 )
		send_to_char( "\n\r", ch );
	send_to_char("\n\r",ch);

	sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	send_to_char(buf,ch);
	sprintf(buf,"Experience per level: %d\n\r",
		exp_per_level(ch,ch->gen_data->points_chosen));
	send_to_char(buf,ch);
	return;
}


void list_group_chosen(CHAR_DATA *ch)
{
	char buf[100];
	int gn,sn,col;

	if (IS_NPC(ch))
		return;

	col = 0;

	sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","group","cp","group","cp","group","cp\n\r");
	send_to_char(buf,ch);

	for (gn = 0; gn < MAX_GROUP; gn++)
	{
		if (group_table[gn].name == NULL)
			break;

		if (ch->gen_data->group_chosen[gn]
	&&  group_table[gn].rating[ch->class] > 0)
		{
			sprintf(buf,"%-18s %-5d ",group_table[gn].name,
									group_table[gn].rating[ch->class]);
			send_to_char(buf,ch);
			if (++col % 3 == 0)
				send_to_char("\n\r",ch);
		}
	}
	if ( col % 3 != 0 )
		send_to_char( "\n\r", ch );
	send_to_char("\n\r",ch);

	col = 0;

	sprintf(buf,"%-18s %-5s %-18s %-5s %-18s %-5s","skill","cp","skill","cp","skill","cp\n\r");
	send_to_char(buf,ch);

	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL)
			break;

		if (ch->gen_data->skill_chosen[sn]
	&&  skill_table[sn].rating[ch->class] > 0)
		{
			sprintf(buf,"%-18s %-5d ",skill_table[sn].name,
									skill_table[sn].rating[ch->class]);
			send_to_char(buf,ch);
			if (++col % 3 == 0)
				send_to_char("\n\r",ch);
		}
	}
	if ( col % 3 != 0 )
		send_to_char( "\n\r", ch );
	send_to_char("\n\r",ch);

	sprintf(buf,"Creation points: %d\n\r",ch->gen_data->points_chosen);
	send_to_char(buf,ch);
	sprintf(buf,"Experience per level: %d\n\r",
		exp_per_level(ch,ch->gen_data->points_chosen));
	send_to_char(buf,ch);
	return;
}

int exp_per_level(CHAR_DATA *ch, int points)
{
	int expl,inc;

	if (IS_NPC(ch))
	return 1000;

	expl = 1000;
	inc = 500;

	if (points < 50)
	return 10 * (pc_race_table[ch->race].class_mult[ch->class] ?
			   pc_race_table[ch->race].class_mult[ch->class] : 100);

	/* processing */
	points -= 50;

	points = UMIN(points, 200);

	while (points > 9)
	{
	expl += inc;
		points -= 10;
		if (points > 9)
	{
		expl += inc;
		inc *= 2;
		points -= 10;
	}
	}

	expl += points * inc / 10;

	expl = (expl * pc_race_table[ch->race].class_mult[ch->class])/100;

	return UMIN(expl, 30000);
}

/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,char *argument)
{
   char arg[MAX_INPUT_LENGTH];
   char buf[100];
   int gn,sn,i;

   if (argument[0] == '\0')
	  return FALSE;

   argument = one_argument(argument,arg);

   if (!str_prefix(arg,"help"))
   {
	  if (argument[0] == '\0')
	  {
		 do_help(ch,"group help");
		 return TRUE;
	  }
	
	  do_help(ch,argument);
	  return TRUE;
   }

   if (!str_prefix(arg,"add"))
   {
	  if (argument[0] == '\0')
	  {
		 send_to_char("You must provide a skill name.\n\r",ch);
		 return TRUE;
	  }
	
	  gn = group_lookup(argument);
	  if (gn != -1)
	  {
		 if (ch->gen_data->group_chosen[gn]
			||  ch->pcdata->group_known[gn])
		 {
			send_to_char("You already know that group!\n\r",ch);
			return TRUE;
		 }
		
		 if (group_table[gn].rating[ch->class] < 1)
		 {
			send_to_char("That group is not available.\n\r",ch);
			return TRUE;
		 }
		
		 /* Added 1/10/98 by Rimbol. Code suggested by dennis@realms.reichel.net */
		 for ( i=0; group_table[gn].spells[i] != NULL ; i++)
		 {
			if ( group_lookup( group_table[gn].spells[i] ) == -1 )
			   continue;
			if ( ch->pcdata->group_known[group_lookup( group_table[gn].spells[i])] )
			{
			   send_to_char("That group contains groups you already know.\n\r",ch);
			   send_to_char("Please \"drop\" them if you wish to gain this one.\n\r",ch);
			   return TRUE;
			}
		 }
		
		 for ( i=0; group_table[gn].spells[i] != NULL ; i++)
		 {
			if ( skill_lookup( group_table[gn].spells[i] ) == -1 )
			   continue;
			if ( ch->gen_data->skill_chosen[skill_lookup( group_table[gn].spells[i])]  )
			{
			   send_to_char("That group contains skills/spells you already know.\n\r",ch);
			   send_to_char("Please \"drop\" them if you wish to gain this one.\n\r",ch);
			   return TRUE;
			}
		 }
		
		 sprintf(buf,"%s group added\n\r",group_table[gn].name);
		 send_to_char(buf,ch);
		 ch->gen_data->group_chosen[gn] = TRUE;
		 ch->gen_data->points_chosen += group_table[gn].rating[ch->class];
		 gn_add(ch,gn);
		 ch->pcdata->points += group_table[gn].rating[ch->class];
		 return TRUE;
	  }
	
	  sn = skill_lookup(argument);
	  if (sn != -1)
	  {
		 if (ch->gen_data->skill_chosen[sn]
			||  ch->pcdata->learned[sn] > 0)
		 {
			send_to_char("You already know that skill!\n\r",ch);
			return TRUE;
		 }
		
		 if (skill_table[sn].rating[ch->class] < 1
			||  skill_table[sn].spell_fun != spell_null)
		 {
			send_to_char("That skill is not available.\n\r",ch);
			return TRUE;
		 }
		 sprintf(buf, "%s skill added\n\r",skill_table[sn].name);
		 send_to_char(buf,ch);
		 ch->gen_data->skill_chosen[sn] = TRUE;
		 ch->gen_data->points_chosen += skill_table[sn].rating[ch->class];
		 ch->pcdata->learned[sn] = 1;
		 ch->pcdata->points += skill_table[sn].rating[ch->class];
		 return TRUE;
	  }
	
	  send_to_char("No skills or groups by that name...\n\r",ch);
	  return TRUE;
   }

   if (!strcmp(arg,"drop"))
   {
	  if (argument[0] == '\0')
	  {
		 send_to_char("You must provide a skill to drop.\n\r",ch);
		 return TRUE;
	  }
	
	  gn = group_lookup(argument);
	  if (gn != -1 && ch->gen_data->group_chosen[gn])
	  {
		 send_to_char("Group dropped.\n\r",ch);
		 ch->gen_data->group_chosen[gn] = FALSE;
		 ch->gen_data->points_chosen -= group_table[gn].rating[ch->class];
		 gn_remove(ch,gn);
		 for (i = 0; i < MAX_GROUP; i++)
		 {
			if (ch->gen_data->group_chosen[gn])
			   gn_add(ch,gn);
		 }
		 ch->pcdata->points -= group_table[gn].rating[ch->class];
		 return TRUE;
	  }
	
	  sn = skill_lookup(argument);
	  if (sn != -1 && ch->gen_data->skill_chosen[sn])
	  {
		 send_to_char("Skill dropped.\n\r",ch);
		 ch->gen_data->skill_chosen[sn] = FALSE;
		 ch->gen_data->points_chosen -= skill_table[sn].rating[ch->class];
		 ch->pcdata->learned[sn] = 0;
		 ch->pcdata->points -= skill_table[sn].rating[ch->class];
		 return TRUE;
	  }
	
	  send_to_char("You haven't bought any such skill or group.\n\r",ch);
	  return TRUE;
   }

   if (!str_prefix(arg,"premise"))
   {
	  do_help(ch,"premise");
	  return TRUE;
   }

   if (!str_prefix(arg,"list"))
   {
	  list_group_costs(ch);
	  return TRUE;
   }

   if (!str_prefix(arg,"learned"))
   {
	  list_group_chosen(ch);
	  return TRUE;
   }

   if (!str_prefix(arg,"info"))
   {
	  do_groups(ch,argument);
	  return TRUE;
   }

   return FALSE;
}
		
	


		

/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, char *argument)
{
   char buf[100];
   int gn,sn,col;

   if (IS_NPC(ch))
	  return;

   col = 0;

   if (argument[0] == '\0')
   {   /* show all groups */
	
	  for (gn = 0; gn < MAX_GROUP; gn++)
	  {
		 if (group_table[gn].name == NULL)
			break;
		 if (ch->pcdata->group_known[gn])
		 {
			sprintf(buf,"%-20s ",group_table[gn].name);
			send_to_char(buf,ch);
			if (++col % 3 == 0)
			   send_to_char("\n\r",ch);
		 }
	  }
	  if ( col % 3 != 0 )
		 send_to_char( "\n\r", ch );
	  sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	  send_to_char(buf,ch);
	  return;
   }

   if (!str_cmp(argument,"all"))    /* show all groups */
   {
	  for (gn = 0; gn < MAX_GROUP; gn++)
	  {
		 if (group_table[gn].name == NULL)
			break;
		 sprintf(buf,"%-20s ",group_table[gn].name);
		 send_to_char(buf,ch);
		 if (++col % 3 == 0)
			send_to_char("\n\r",ch);
	  }
	  if ( col % 3 != 0 )
		 send_to_char( "\n\r", ch );
	  return;
   }


   /* show the sub-members of a group */
   gn = group_lookup(argument);
   if (gn == -1)
   {
	  send_to_char("No group of that name exist.\n\r",ch);
	  send_to_char(
		 "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
	  return;
   }

   for (sn = 0; sn < MAX_IN_GROUP; sn++)
   {
	  if (group_table[gn].spells[sn] == NULL)
		 break;
	  sprintf(buf,"[%3d] %-20s ",
		 (skill_lookup(group_table[gn].spells[sn]) < 0 ) ? 0 :
	  skill_table[skill_lookup(group_table[gn].spells[sn])].skill_level[ch->class],
		 group_table[gn].spells[sn]);
	  send_to_char(buf,ch);
	  if (++col % 3 == 0)
		 send_to_char("\n\r",ch);
	
	  if ( col % 3 != 0 )
		 send_to_char( "\n\r", ch );
   }
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier )
{
	int chance;
	char buf[100];

	if (IS_NPC(ch))
	return;

	if (ch->level < skill_table[sn].skill_level[ch->class]
	||  skill_table[sn].rating[ch->class] == 0
	||  ch->pcdata->learned[sn] == 0
	||  ch->pcdata->learned[sn] == 100)
	return;  /* skill is not known */

	if (multiplier>0)
	{
	/* check to see if the character has a chance to learn */
	chance = 10 * ch_int_learn(ch);
	chance /= (     multiplier
		*   skill_table[sn].rating[ch->class]
		*   4);
	chance += ch->level;

	if (number_range(1,1000) > chance)
	return;
	}
	/* now that the character has a CHANCE to learn, see if they really have */

	if (success)
	{
	chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
	if (number_percent() < chance)
	{
		sprintf(buf,"You have become better at %s!\n\r",
			skill_table[sn].name);
		send_to_char(buf,ch);
		ch->pcdata->learned[sn]++;
		gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
	}

	else
	{
	chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
	if (number_percent() < chance)
	{
		sprintf(buf,
		"You learn from your mistakes, and your %s skill improves.\n\r",
		skill_table[sn].name);
		send_to_char(buf,ch);
		ch->pcdata->learned[sn] += number_range(1,3);
		ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
		gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
	}
	}
}

/* returns a group index number given the name */
int group_lookup( const char *name )
{
	int gn;

	for ( gn = 0; gn < MAX_GROUP; gn++ )
	{
		if ( group_table[gn].name == NULL )
			break;
		if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
		&&   !str_prefix( name, group_table[gn].name ) )
			return gn;
	}

	return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn)
{
	int i;
	
	ch->pcdata->group_known[gn] = TRUE;
	for ( i = 0; i < MAX_IN_GROUP; i++)
	{
		if (group_table[gn].spells[i] == NULL)
			break;
		group_add(ch,group_table[gn].spells[i],FALSE);
	}
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn)
{
	int i;

	ch->pcdata->group_known[gn] = FALSE;

	for ( i = 0; i < MAX_IN_GROUP; i ++)
	{
	if (group_table[gn].spells[i] == NULL)
		break;
	group_remove(ch,group_table[gn].spells[i]);
	}
}
	
/* use for processing a skill or group for addition  */
void group_add( CHAR_DATA *ch, const char *name, bool deduct)
{
	int sn,gn;

	if (IS_NPC(ch)) /* NPCs do not have skills */
	return;

	sn = skill_lookup(name);

	if (sn != -1)
	{
	if (ch->pcdata->learned[sn] == 0) /* i.e. not known */
	{
		ch->pcdata->learned[sn] = 1;
		if (deduct)
		ch->pcdata->points += skill_table[sn].rating[ch->class];
	}
	return;
	}
	
	/* now check groups */

	gn = group_lookup(name);

	if (gn != -1)
	{
	if (ch->pcdata->group_known[gn] == FALSE)
	{
		ch->pcdata->group_known[gn] = TRUE;
		if (deduct)
		ch->pcdata->points += group_table[gn].rating[ch->class];
	}
	gn_add(ch,gn); /* make sure all skills in the group are known */
	}
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name)
{
	int sn, gn;
	
	 sn = skill_lookup(name);

	if (sn != -1)
	{
	ch->pcdata->learned[sn] = 0;
	return;
	}

	/* now check groups */

	gn = group_lookup(name);

	if (gn != -1 && ch->pcdata->group_known[gn] == TRUE)
	{
	ch->pcdata->group_known[gn] = FALSE;
	gn_remove(ch,gn);  /* be sure to call gn_add on all remaining groups */
	}
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
	int skill,i;

	if (sn == -1) /* shorthand for level based skills */
	{
	skill = ch->level * 5 / 2;
	}

	else if (sn < -1 || sn > MAX_SKILL)
	{
	bug("Bad sn %d in get_skill.",sn);
	skill = 0;
	}

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

	else /* mobiles */
	{


		if (skill_table[sn].spell_fun != spell_null)
		skill = 40 + 2 * ch->level;

	else if (sn == gsn_sneak || sn == gsn_hide)
		skill = ch->level * 2 + 20;

		else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE))
	||       (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)))
		skill = ch->level * 2;

	else if (sn == gsn_shield_block)
		skill = 10 + 2 * ch->level;

	else if (sn == gsn_second_attack
	&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
		skill = 10 + 3 * ch->level;

	else if (sn == gsn_duck
	&& (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF)))
		skill = 2 * ch->level;

	else if (sn == gsn_third_attack && IS_SET(ch->act,ACT_WARRIOR))
		skill = 4 * ch->level - 40;

	else if (sn == gsn_hand_to_hand)
		skill = 40 + 2 * ch->level;

	else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP))
		skill = 10 + 3 * ch->level;

	else if (sn == gsn_bash && IS_SET(ch->off_flags,OFF_BASH))
		skill = 10 + 3 * ch->level;

	else if (sn == gsn_disarm
		 &&  (IS_SET(ch->off_flags,OFF_DISARM)
		 ||   IS_SET(ch->act,ACT_WARRIOR)
		 ||   IS_SET(ch->act,ACT_THIEF)))
		skill = 20 + 3 * ch->level;

	else if (sn == gsn_berserk && IS_SET(ch->off_flags,OFF_BERSERK))
		skill = 3 * ch->level;

	else if (sn == gsn_lore && IS_SET(ch->act, ACT_MAGE))
		skill = ch->level;

	else if (sn == gsn_kick)
		skill = 10 + 3 * ch->level;

	else if (sn == gsn_backstab && IS_SET(ch->act,ACT_THIEF))
		skill = 20 + 2 * ch->level;

	else if (sn == gsn_circle && IS_SET(ch->act,ACT_THIEF))
		skill = 20 + 2 * ch->level;

	else if (sn == gsn_rescue)
		skill = 40 + ch->level;

	else if (sn == gsn_recall)
		skill = 40 + ch->level;

	else if (sn == gsn_sword
	||  sn == gsn_dagger
	||  sn == gsn_spear
	||  sn == gsn_mace
	||  sn == gsn_axe
	||  sn == gsn_flail
	||  sn == gsn_whip
	||  sn == gsn_polearm
	||  sn == gsn_gun)
		skill = 40 + 5 * ch->level / 2;

	else
	   skill = 0;
	}

	if (ch->daze > 0)
	{
	if (skill_table[sn].spell_fun != spell_null)
		skill /= 2;
	else
		skill = 2 * skill / 3;
	}

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 && !IS_AFFECTED(ch, AFF_BERSERK) )
	skill = 9 * skill / 10;
	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_SMOKE]  < -1 )
	skill = 9 * skill / 10;

	return URANGE(0,skill,100);
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
	 int skill, i, max, total;

	 /* -1 is exotic */
	if (IS_NPC(ch))
	{
	if (sn == -1)
		skill = 3 * ch->level;
	else if (sn == gsn_hand_to_hand)
		skill = 40 + 2 * ch->level;
	else
		skill = 40 + 5 * ch->level / 2;
	}
	
	else
	{
	if (sn == -1)
	{
		max=0;
		total=0;
		for (i=gsn_axe; i<=gsn_whip; i++)
		{
			skill = get_weapon_skill(ch, i);
			total+=skill;
			if (skill>max) max=skill;
		}
		skill=(total/(gsn_whip-gsn_axe)+max)/2;
	}
	else
	{
		if (ch->level < skill_table[sn].skill_level[ch->class])
			skill = 0;
		else
			skill = ch->pcdata->learned[sn];
	}
	}

	return URANGE(0,skill,100);
}



void do_practice( CHAR_DATA *ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   int sn;
   int skill;

   if ( IS_NPC(ch) )
	  return;

   if ( argument[0] == '\0' )
   {
	  int col;
	
	  col    = 0;
	  for ( sn = 0; sn < MAX_SKILL; sn++ )
	  {
		 if ( skill_table[sn].name == NULL )
			break;
		 if ( (skill=ch->pcdata->learned[sn])< 1
			|| ch->level < skill_table[sn].skill_level[ch->class] )
			continue;
		
		 sprintf( buf, "%-18s %3d%%  ",
			skill_table[sn].name, skill );
		 send_to_char( buf, ch );
		 if ( ++col % 3 == 0 )
			send_to_char( "\n\r", ch );
	  }
	
	  if ( col % 3 != 0 )
		 send_to_char( "\n\r", ch );
	
	  sprintf( buf, "You have %d practice sessions left.\n\r",
		 ch->practice );
	  send_to_char( buf, ch );
   }
   else
   {
	CHAR_DATA *mob;
	  int adept;
	
	  if ( !IS_AWAKE(ch) )
	  {
		 send_to_char( "In your dreams, or what?\n\r", ch );
		 return;
	  }
	
	  for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	  {
		 if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
			break;
	  }

   if ( mob == NULL )
   {
	if (get_skill(ch, gsn_introspection) > 1)
	{
	      act( "$n thinks over what $e has experienced recently.", ch, NULL, NULL, TO_ROOM);
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
	
	  if ( ch->practice <= 0 )
	  {
		 send_to_char( "You have no practice sessions left.\n\r", ch );
		 return;
	  }

	if (!str_cmp("field",argument))
	{
		ch->practice--;
		sn = number_range(ch->pcdata->field/2, ch->pcdata->field);
		ch->pcdata->field-=sn;
		gain_exp(ch, sn);
		return;
	}
	
	  if ( ( sn = find_spell( ch,argument ) ) < 0
		 || ( !IS_NPC(ch)
		 &&   (ch->level < skill_table[sn].skill_level[ch->class]
		 ||    ch->pcdata->learned[sn] < 1 /* skill is not known */
		 ||    skill_table[sn].rating[ch->class] == 0)))
	  {
		 send_to_char( "You can't practice that.\n\r", ch );
		 return;
	  }
	
	  adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;
	
	  if ( ch->pcdata->learned[sn] >= adept )
	  {
		 sprintf( buf, "You are already learned at %s.\n\r",
			skill_table[sn].name );
		 send_to_char( buf, ch );
	  }
	  else
	  {
		ch->practice--;
		 ch->pcdata->learned[sn] +=
			ch_int_learn(ch) / skill_table[sn].rating[ch->class];
		 if ( ch->pcdata->learned[sn] < adept )
		 {
			act( "You practice $T.",
			   ch, NULL, skill_table[sn].name, TO_CHAR );
			act( "$n practices $T.",
			   ch, NULL, skill_table[sn].name, TO_ROOM );
		 }
		 else
		 {
			ch->pcdata->learned[sn] = adept;
			act( "You are now learned at $T.",
			   ch, NULL, skill_table[sn].name, TO_CHAR );
			act( "$n is now learned at $T.",
			   ch, NULL, skill_table[sn].name, TO_ROOM );
		 }
	  }
   }
   return;
}


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

