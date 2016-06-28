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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_outfit );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_visible );
AREA_DATA *get_vnum_area( int );

typedef struct remort_table REMORT_TABLE;

struct remort_table
{
	REMORT_TABLE *next;
	char *name;
	int remorts;
	time_t signup;
	time_t limit;	
};

struct remort_chamber
{
	char *name;
	int vnum;
	int availability;
	bool speed;
};

const struct remort_chamber chambers[] =
{
	{"Remort I-II       ",	6600, 3, FALSE},
	{"Remort I-II       ",	6650, 3, TRUE},
	{"Remort: Afterlife ",	6800, 4, FALSE},
	{"Remort: Afterlife ",	22500, 4, FALSE},
	{"Remort: Afterlife ",	22600, 4, FALSE},
	{"Remort: Afterlife ",	22700, 4, FALSE},
	{"Remort: Afterlife ",	22800, 4, TRUE},
	{NULL,			0, 0}
};

REMORT_TABLE *chamber_list[8];
REMORT_TABLE *wait_list;

int power2(int x)
{
	if (x<1) return 1;
	return (2*power2(x-1));
}

char *time_format(time_t t, char *b)
{
	sprintf(b, "%s", (char *) ctime( &t ));
	b[24]='\0';
	return b;
}

void do_remort args( (CHAR_DATA *ch, char *argument) );
void remort_signup args( (CHAR_DATA *ch, CHAR_DATA *adept) );
void remort_cancel args( (CHAR_DATA *ch, CHAR_DATA *adept) );
void remort_status args( (CHAR_DATA *ch, CHAR_DATA *adept) );
void remort_enter args( (CHAR_DATA *ch, CHAR_DATA *adept) );
void remort_speed args( (CHAR_DATA *ch, CHAR_DATA *adept) );
void remort_save args( ( void ) );


void do_remort(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *adept;
	char arg [MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) return;

	argument = one_argument(argument, arg);

	for ( adept = ch->in_room->people; adept != NULL; adept = adept->next_in_room )
	{
		if (!IS_NPC(adept)) continue;
			if (adept->spec_fun == spec_lookup( "spec_remort" )) break;
	}

	if (adept == NULL || adept->spec_fun != spec_lookup( "spec_remort" ))
	{
		send_to_char("There is no remort adept here.\n\r",ch);
		return;
	}

	if ( !can_see(adept, ch))
	{
		do_say(adept, "Whazzat?  Did I hear something?");
		return;
	}

	if (arg[0] != '\0')
		if (!strcmp(arg, "signup"))
		{
			remort_signup(ch, adept);
			return;
		}
		else if (!strcmp(arg, "cancel"))
		{
			remort_cancel(ch, adept);
			return;
		}
		else if (!strcmp(arg, "status"))
		{
			remort_status(ch, adept);
			return;
		}
		else if (!strcmp(arg, "enter"))
		{
			remort_enter(ch, adept);
			return;
		}
		else if (!strcmp(arg, "speed"))
		{
			remort_speed(ch, adept);
			return;
		}

	send_to_char("Remort options: signup, cancel, status, enter.\n\r",ch);
	send_to_char("For more information, type 'HELP REMORT'.\n\r",ch);
}


void remort_signup(CHAR_DATA *ch, CHAR_DATA *adept)
{
	REMORT_TABLE *new; 
	REMORT_TABLE *i;
	int qpcost, goldcost;
	char buf[MAX_STRING_LENGTH];

	if (ch->pcdata->remorts>=10)
	{
		sprintf(buf, "You are far too powerful for that, %s.", ch->name);
		do_say(adept, buf);
		return;
	}

	qpcost=200+50*ch->pcdata->remorts;
	goldcost=1000*(20+8*ch->pcdata->remorts+power2(ch->pcdata->remorts-1));

	for (i=wait_list; i!=NULL; i=i->next)
	{
		if (!str_cmp(ch->name, i->name))
		{
			sprintf(buf, "You are already on the list, %s.", ch->name);
			do_say(adept, buf);
			return;
		}
	}

	if (ch->questpoints<qpcost)
	{
		sprintf(buf, "You need %d quest points to get into remort, %s.", qpcost, ch->name);
		do_say(adept, buf);
		return;
	}

	if (ch->gold<goldcost)
	{
		sprintf(buf, "There is a %d gold remort tax, %s.", goldcost, ch->name);
		do_say(adept, buf);
		return;
	}

	sprintf(buf, "That'll be %d gold, and %d qps, %s.",
		goldcost, qpcost, ch->name);
	do_say(adept, buf);

	ch->questpoints-=qpcost;
	ch->gold-=goldcost;

	new = alloc_mem(sizeof(REMORT_TABLE));
	new->next = NULL;
	new->name = str_dup(ch->name);
	new->remorts = ch->pcdata->remorts;
	new->signup = current_time;
	new->limit = 0;

	if ((i=wait_list)!=NULL)
	{
		for(;i->next!=NULL;) i=i->next;
		i->next=new;
	}
	else
		wait_list=new;

	do_say(adept, "You are now on the list.");

	remort_update();
	save_char_obj(ch);
}


void remort_cancel(CHAR_DATA *ch, CHAR_DATA *adept)
{
	REMORT_TABLE *prev = NULL; 
	REMORT_TABLE *i;
	int qpcost, goldcost;
	char buf[MAX_STRING_LENGTH];
	bool found=FALSE;

	qpcost=200+50*ch->pcdata->remorts;
	goldcost=1000*(20+8*ch->pcdata->remorts+power2(ch->pcdata->remorts-1));

	for (i=wait_list; i!=NULL; i=i->next)
	{
		if (!str_cmp(ch->name, i->name))
		{
			found=TRUE;
			break;
		}
		prev=i;
	}

	if (!found)
	{
		sprintf(buf, "You aren't on the list, %s.", ch->name);
		do_say(adept, buf);
		return;
	}

	sprintf(buf, "You may have your %d gold, and %d qps back, %s.",
		goldcost, qpcost, ch->name);
	do_say(adept, buf);

	ch->questpoints+=qpcost;
	ch->gold+=goldcost;

	if (i==wait_list)
	{
		wait_list=wait_list->next;
	}
	else
		prev->next=i->next;

	free_string(i->name);
	free_mem(i, sizeof(REMORT_TABLE));

	do_say(adept, "You are no longer on the list.");

	remort_update();
	save_char_obj(ch);
}


void remort_status(CHAR_DATA *ch, CHAR_DATA *adept)
{
	REMORT_TABLE *i;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char tbuf[MAX_STRING_LENGTH];
	int j;

	for (j=0; chambers[j].name!=NULL; j++)
	{

		if (chamber_list[j]!=NULL)
		{
			sprintf(buf2, "%18s  %s", chamber_list[j]->name,
				time_format(chamber_list[j]->limit, tbuf));
		}
		else
			sprintf(buf2, "          Nobody");

		sprintf(buf, "%s %s",
			chambers[j].name,
			buf2);

		do_say(adept, buf);
	}

	do_say(adept, "People still waiting for a chamber:");

	for (i=wait_list; i!=NULL; i=i->next)
	{
		if (i->limit!=0)
		{
			sprintf(buf, "%16s %d  *%s", i->name, i->remorts,
				time_format(i->limit, tbuf));
		}
		else
		{
			sprintf(buf, "%16s %d    %s", i->name, i->remorts,
				time_format(i->signup, tbuf));
		}

		do_say(adept, buf);
	}
}


void remort_enter(CHAR_DATA *ch, CHAR_DATA *adept)
{
	REMORT_TABLE *prev=NULL; 
	REMORT_TABLE *i;
	char buf[MAX_STRING_LENGTH];
	bool found=FALSE;
	int j;

	for (i=wait_list; i!=NULL; i=i->next)
	{
		if (!str_cmp(ch->name, i->name))
		{
			found=TRUE;
			break;
		}
		prev=i;
	}

	if (!found)
	{
		sprintf(buf, "You aren't on the list, %s.", ch->name);
		do_say(adept, buf);
		return;
	}

	if (i->limit == 0)
	{
		sprintf(buf, "I'm sorry, there are no chambers available for you, %s.", ch->name);
		do_say(adept, buf);
		return;
	}

	if (i==wait_list)
	{
		wait_list=wait_list->next;
	}
	else
		prev->next=i->next;

	for (j=0; chambers[j].name!=NULL; j++)
	{
		if (chamber_list[j]!=NULL)
			continue;

		if (chambers[j].availability&power2(ch->pcdata->remorts))
		{
			chamber_list[j]=i;
			break;
		}
	}

	act( "$n vanishes into a shimmering vortex.", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, get_room_index(chambers[j].vnum) );
	send_to_char("You step into a shimmering vortex and arrive in another dimension.\n\r",ch);
	do_look( ch, "auto" );
	do_visible(ch, "");

	i->limit=current_time+604800;

	remort_update();
	save_char_obj(ch);
}


void remort_speed(CHAR_DATA *ch, CHAR_DATA *adept)
{
	REMORT_TABLE *prev=NULL; 
	REMORT_TABLE *i;
	char buf[MAX_STRING_LENGTH];
	bool found=FALSE;
	int j;

	for (i=wait_list; i!=NULL; i=i->next)
	{
		if (!str_cmp(ch->name, i->name))
		{
			found=TRUE;
			break;
		}
		prev=i;
	}

	if (!found)
	{
		sprintf(buf, "You aren't on the list, %s.", ch->name);
		do_say(adept, buf);
		return;
	}

	for (j=0; chambers[j].name!=NULL; j++)
	{
		if (chamber_list[j]!=NULL)
			continue;

		if ((chambers[j].availability&power2(ch->pcdata->remorts))&&chambers[j].speed)
		{
			chamber_list[j]=i;
			break;
		}
	}

	if (chambers[j].name==NULL)
	{
		sprintf(buf, "There are no speed chambers ready, %s.  Keep your pants on.", ch->name);
		do_say(adept, buf);
		WAIT_STATE( ch, 120 );	
		return;
	}
		
	if (i==wait_list)
	{
		wait_list=wait_list->next;
	}
	else
		prev->next=i->next;

	act( "$n vanishes into a shimmering vortex.", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, get_room_index(chambers[j].vnum) );
	send_to_char("You step into a shimmering vortex and arrive in another dimension.\n\r",ch);
	do_look( ch, "auto" );
	do_visible(ch, "");

	i->limit=current_time+28800;

	remort_update();
	save_char_obj(ch);
}


void remort_update()
{
	REMORT_TABLE *i;
	REMORT_TABLE *prev=NULL;
	AREA_DATA *pArea;
	bool found=FALSE;
	int j;
	CHAR_DATA *ch=NULL;
	DESCRIPTOR_DATA *d;
	bool used[20];

	for (j=0; chambers[j].name!=NULL; j++)
		if (chamber_list[j]!=NULL &&
			(chamber_list[j]->limit < current_time))
		{
 			OBJ_DATA *obj;
			OBJ_DATA *obj_next;
  			for ( d = descriptor_list; d != NULL; d = d->next )
   			{
      			ch = d->original ? d->original : d->character;
				if ((d->connected == CON_PLAYING
				|| IS_WRITING_NOTE(d->connected))
				&& !str_cmp(ch->name, chamber_list[j]->name))
				{
					found=TRUE;
					break;
				}
			}
			
			if (found)
			{
				send_to_char("You have run out of time to complete remort.\n\r",ch);
				char_from_room( ch );
 				   for ( obj = ch->carrying; obj; obj = obj_next )
 				   {
					obj_next = obj->next_content;
					if (IS_SET(obj->extra_flags, ITEM_REMORT))
					{
						obj_from_char( obj );
						extract_obj( obj );
					}
  				  }
				char_to_room( ch, get_room_index(ROOM_VNUM_RECALL) );
				do_look( ch, "auto" );
			}
			else
			{
				d=new_descriptor();
				load_char_obj(d, chamber_list[j]->name);
				if (d->character!=NULL)
				{
					d->character->in_room = get_room_index(ROOM_VNUM_RECALL);
  				  for ( obj = d->character->carrying; obj; obj = obj_next )
   				 {
					obj_next = obj->next_content;
					if (IS_SET(obj->extra_flags, ITEM_REMORT))
					{
						obj_from_char( obj );
						extract_obj( obj );
					}
  				  }
				}
				save_char_obj(d->character);
				free_descriptor(d);
			}

			free_string(chamber_list[j]->name);
			free_mem(chamber_list[j], sizeof(REMORT_TABLE));
			chamber_list[j]=NULL;
			pArea=get_vnum_area(chambers[j].vnum);
			purge_area(pArea);
			reset_area(pArea);
			remort_save();
		}

	for (i=wait_list; i!=NULL; i=i->next)
	{
		if (i->limit && i->limit<current_time)
		{
			if (i==wait_list)
				wait_list=i->next;
			else
				prev->next=i->next;
				
			free_string(i->name);
			free_mem(i, sizeof(REMORT_TABLE));
			break;
		}
		
		prev=i;
	}

	for (j=0; chambers[j].name!=NULL; j++)
		used[j]= (chamber_list[j]!=NULL);

	for (i=wait_list; i!=NULL; i=i->next)
	{
		for (j=0; chambers[j].name!=NULL; j++)
			if ((chambers[j].availability&power2(i->remorts))
				&& used[j]==FALSE && chambers[j].speed==FALSE)
			{
				used[j]=TRUE;
				if (i->limit==0)
					i->limit=current_time+345600;
				break;
			}
	}

	remort_save();
} 


void remort_remove(CHAR_DATA *ch)
{
	int j;
	AREA_DATA *pArea;

	for (j=0; chambers[j].name!=NULL; j++)
		if (chamber_list[j]!=NULL)
		if (!str_cmp(ch->name, chamber_list[j]->name))
		{
			free_string(chamber_list[j]->name);
			free_mem(chamber_list[j], sizeof(REMORT_TABLE));
			chamber_list[j]=NULL;
			pArea=get_vnum_area(chambers[j].vnum);
			purge_area(pArea);
			reset_area(pArea);
			remort_save();
			return;
		}
}


void remort_load()
{
	FILE *fp;
	REMORT_TABLE *p, *q = NULL;
	char *s;
	int i;
   
	wait_list = NULL;
	
	fp = fopen (REMORT_FILE, "r");
	
	if (!fp)
	{
		for (i=0; chambers[i].name!=NULL; i++)
			chamber_list[i]=NULL;

		return;
	}

	for (i=0; chambers[i].name!=NULL; i++)
	{
		s = str_dup(fread_word (fp));
		if (str_cmp(s, "X"))
		{
			p=alloc_mem(sizeof(REMORT_TABLE));
			p->name = s;
			p->remorts = fread_number(fp);
			p->signup = fread_number(fp);
			p->limit = fread_number(fp);
			chamber_list[i]=p;
		}
		else
		{
			chamber_list[i]=NULL;
			free_string(s);
		}
	}
	
	s = str_dup(fread_word (fp));
	while (s && str_cmp(s, "END"))
	{
		p=alloc_mem(sizeof(REMORT_TABLE));
		if (wait_list==NULL)
			wait_list=p;
		else
			q->next=p;
		q=p;
		p->name = s;
		p->next = NULL;
		p->remorts = fread_number(fp);
		p->signup = fread_number(fp);
		p->limit = fread_number(fp);

		s= str_dup(fread_word (fp));
	}
	free_string(s);

	fclose (fp);		
}


void remort_save()
{
	FILE *fp;
	REMORT_TABLE *p;
	int i;
	
	fp = fopen (REMORT_FILE, "w");
	
	if (!fp)
	{
		bug ("Could not open " REMORT_FILE " for writing",0);
		return;
	}
	
	for (i=0; chambers[i].name!=NULL; i++)
		if (chamber_list[i]==NULL)
			fprintf (fp, "X\n");
		else
			fprintf (fp, "%s %d %ld %ld\n", 
				chamber_list[i]->name,
				chamber_list[i]->remorts,
				chamber_list[i]->signup,
				chamber_list[i]->limit);

	for (p = wait_list; p ; p = p->next)
		fprintf (fp, "%s %d %ld %ld\n", 
			p->name, p->remorts, p->signup, p->limit);

	fprintf (fp, "END\n");
		
	fclose (fp);
}


void remort_begin(CHAR_DATA *ch)
{
	int i;	

	save_char_obj(ch);

	if (ch->desc!=NULL)
		ch->desc->connected = CREATION_REMORT*MAX_CON_STATE + CON_GET_NEW_RACE;
	else
	{
		do_quit(ch, "");
		return;
	}


	if ( ch == char_list )
	{
	   char_list = ch->next;
	}
	else
	{
	CHAR_DATA *prev;

	for ( prev = char_list; prev != NULL; prev = prev->next )
		if ( prev->next == ch )
		{
		prev->next = ch->next;
		break;
		}
	}
	
	char_from_room( ch );
	ch->pcdata->remorts++;
	
	for (i=0; i<MAX_STATS; i++)
		ch->pcdata->history_stats[i]+=ch->pcdata->original_stats[i];

	send_to_char("{CYou are gently pulled upward into a column of brilliant\n\r", ch);
	send_to_char("light.  As you look down, you can see your physical body\n\r", ch);
	send_to_char("lying prone, lifeless.  As if alive beneath the divine\n\r", ch);
	send_to_char("radiance, the earth absorbs your spent body into itself.\n\r", ch);
	send_to_char("Unencumbered by a physical anchor, your spirit is free\n\r", ch);
	send_to_char("to roam all creation until you next choose to take on a\n\r", ch);
	send_to_char("material form.{g{ \n\r\n\r", ch);
	send_to_char("              (Press Enter to Continue)\n\r", ch);

}


void remort_complete(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *obj_next;

	remort_remove(ch);

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
		obj_next = obj->next_content;
		extract_obj( obj );
	}
	
	ch->level   = 1;
	ch->gold = 0;
	ch->silver =0;
	ch->pcdata->points = 50;
	ch->exp = exp_per_level(ch,ch->pcdata->points);
	ch->hit = 10*(9+ch->pcdata->remorts);
	ch->mana = 20*(9+ch->pcdata->remorts);
	ch->move = 20*(9+ch->pcdata->remorts);
	ch->max_hit = ch->hit;
	ch->max_mana    = ch->mana;
	ch->max_move    = ch->move;
	ch->pcdata->perm_hit=ch->hit;
	ch->pcdata->perm_mana=ch->mana;
	ch->pcdata->perm_move=ch->move;
	ch->pcdata->field = 0;
	ch->train    = 8;
	ch->practice = 12;
	ch->pcdata->condition[0]=0;
	ch->pcdata->condition[1]=0;
	ch->pcdata->condition[2]=0;
	ch->pcdata->condition[3]=0;
	ch->pcdata->condition[4]=0;
	ch->pcdata->condition[5]=0;

	if (! IS_SET(ch->act, PLR_TITLE))
	{
		sprintf( buf, "the %s",
	  title_table [ch->class] [1]);
		set_title( ch, buf );
	}

	do_outfit(ch,"");
	obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

	ch->next    = char_list;
	char_list   = ch;

	ch->desc->connected = CON_PLAYING;
	char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	send_to_char("\n\r",ch);

	sprintf(buf, "After much struggle, %s has made it to level 1!", ch->name);
      do_info_message(ch, buf, FALSE);

	save_char_obj(ch);
}

