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

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#if defined( WIN32 )
#include <sys/timeb.h> /*for _ftime(), uses _timeb struct*/
#else
#include <sys/time.h>
#endif
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "merc.h"
#include "recycle.h"
#include "tables.h"

#if defined(macintosh) || defined(MSDOS)
extern const   char    echo_off_str	[];
extern const   char    echo_on_str	[];
extern const   char    go_ahead_str	[];
#endif

#if     defined(unix) || defined(WIN32)
#include "telnet.h"
extern const   char    echo_off_str    [];
extern const   char    echo_on_str     [];
extern const   char    go_ahead_str    [];
#endif


/* command procedures needed */
DECLARE_DO_FUN(do_help      );
DECLARE_DO_FUN(do_look      );
DECLARE_DO_FUN(do_skills    );
DECLARE_DO_FUN(do_outfit    );
DECLARE_DO_FUN(do_unread    );

extern bool            wizlock;        /* Game is wizlocked        */
extern bool            newlock;        /* Game is newlocked        */

bool    check_reconnect     args( ( DESCRIPTOR_DATA *d, char *name,
					bool fConn ) );
bool    check_playing       args( ( DESCRIPTOR_DATA *d, char *name ) );
bool    check_parse_name    args( ( char *name ) );

void enter_game args((DESCRIPTOR_DATA *d));
void take_rom_basics args((DESCRIPTOR_DATA *d));
void take_class_defaults args((DESCRIPTOR_DATA *d));
void take_default_weapon args((DESCRIPTOR_DATA *d));
void newbie_alert args((DESCRIPTOR_DATA *d));
void take_default_stats args((CHAR_DATA *ch));
void get_random_stats args((CHAR_DATA *ch));
bool parse_roll_stats args((CHAR_DATA *ch,char *argument));
bool parse_stat_priority args((CHAR_DATA *ch, char *argument));
void do_stats args((CHAR_DATA *ch, char *argument));
void do_etls args((CHAR_DATA *ch, char *argument));

bool	get_name			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_old_password		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	confirm_new_name		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_new_password		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	confirm_new_password	args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_new_race		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_new_sex			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_new_class		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_alignment		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	default_choice		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	gen_groups			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	pick_weapon			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	read_imotd			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	read_motd			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	break_connect		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_creation_mode		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	roll_stats			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	get_stat_priority		args( ( DESCRIPTOR_DATA *d, char *argument ) );

bool	smith_welcome		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_type			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_subtype		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_purchase		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_material		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_quality		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_color			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_personal		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_level			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_keywords		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_select		args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	smith_inventory		args( ( DESCRIPTOR_DATA *d, char *argument ) );

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
	switch (creation_mode(d) )
	{

	default:
		switch ( con_state(d) )
		{

		default:
		bug( "Nanny: bad d->connected %d.", d->connected );
		close_socket( d );
		return;
        
        case CON_GET_NAME:
            get_name(d, argument);
			break;

		case CON_GET_OLD_PASSWORD:
			if (get_old_password(d, argument)) read_imotd(d, argument);
			break;

		case CON_BREAK_CONNECT:
			if (break_connect(d, argument)) get_name(d, argument);
			break;

		case CON_CONFIRM_NEW_NAME:
			if (confirm_new_name(d, argument))
			{
				newbie_alert(d);
				get_new_password(d, argument);
			}
			break;

		case CON_GET_NEW_PASSWORD:
			if (get_new_password(d, argument)) confirm_new_password(d, argument);
			break;

		case CON_CONFIRM_NEW_PASSWORD:
			if (confirm_new_password(d, argument)) get_creation_mode(d, argument);
			break;

		case CON_GET_CREATION_MODE:
			if (get_creation_mode(d, argument)) get_new_race(d, argument);
			break;

		case CON_READ_IMOTD:
			if (read_imotd(d, argument)) read_motd(d, argument);
			break;

		case CON_READ_MOTD:
			if (read_motd(d, argument)) enter_game(d);
			break;
		}	
		break;

	case CREATION_INSTANT:
		switch ( con_state(d) )
		{
		
		default:
		bug( "Nanny: bad d->connected %d.", d->connected );
		close_socket( d );
		return;
		
		case CON_GET_NEW_RACE:
			if (get_new_race(d, argument)) get_new_sex(d, argument);
			break;

		case CON_GET_NEW_SEX:
			if (get_new_sex(d, argument)) get_new_class(d, argument);
			break;

		case CON_GET_NEW_CLASS:
			if (get_new_class(d, argument)) get_alignment(d, argument);
			break;

		case CON_GET_ALIGNMENT:
			if (get_alignment(d, argument))
			{
				take_rom_basics(d);
				take_class_defaults(d);
				take_default_weapon(d);
				take_default_stats(d->original ? d->original : d->character);
				set_creation_state(d, CREATION_UNKNOWN);
				read_imotd(d, argument);
			}
			break;

		}
		break;

	case CREATION_QUICK:
		switch ( con_state(d) )
		{
		
		default:
		bug( "Nanny: bad d->connected %d.", d->connected );
		close_socket( d );
		return;
		
		case CON_GET_NEW_RACE:
			if (get_new_race(d, argument)) get_new_sex(d, argument);
			break;

		case CON_GET_NEW_SEX:
			if (get_new_sex(d, argument)) get_new_class(d, argument);
			break;

		case CON_GET_NEW_CLASS:
			if (get_new_class(d, argument)) get_alignment(d, argument);
			break;

		case CON_GET_ALIGNMENT:
			if (get_alignment(d, argument))
			{
				take_rom_basics(d);
				default_choice(d, argument);
			}
			break;

		case CON_DEFAULT_CHOICE:
			if (default_choice(d, argument)) pick_weapon(d, argument);
			break;

		case CON_GEN_GROUPS:
			if (gen_groups(d, argument)) pick_weapon(d, argument);
			break;

		case CON_PICK_WEAPON:
			if (pick_weapon(d, argument)) get_stat_priority(d, argument);
			break;

		case CON_GET_STAT_PRIORITY:
			if (get_stat_priority(d, argument))
			{
				set_creation_state(d, CREATION_UNKNOWN);
				read_imotd(d, argument);
			}
			break;

		}
		break;

	case CREATION_NORMAL:
		switch ( con_state(d) )
		{
		
		default:
		bug( "Nanny: bad d->connected %d.", d->connected );
		close_socket( d );
		return;
		
		case CON_GET_NEW_RACE:
			if (get_new_race(d, argument)) get_new_sex(d, argument);
			break;

		case CON_GET_NEW_SEX:
			if (get_new_sex(d, argument)) get_new_class(d, argument);
			break;

		case CON_GET_NEW_CLASS:
			if (get_new_class(d, argument)) get_alignment(d, argument);
			break;

		case CON_GET_ALIGNMENT:
			if (get_alignment(d, argument))
			{
				take_rom_basics(d);
				gen_groups(d, argument);
			}
			break;

		case CON_GEN_GROUPS:
			if (gen_groups(d, argument)) pick_weapon(d, argument);
			break;

		case CON_PICK_WEAPON:
			if (pick_weapon(d, argument)) roll_stats(d, argument);
			break;

		case CON_ROLL_STATS:
			if (roll_stats(d, argument))
			{
				set_creation_state(d, CREATION_UNKNOWN);
				read_imotd(d, argument);
			}
			break;

		}
		break;

	case CREATION_REMORT:
		switch ( con_state(d) )
		{
		
		default:
		bug( "Nanny: bad d->connected %d.", d->connected );
		close_socket( d );
		return;
		
		case CON_GET_NEW_RACE:
			if (get_new_race(d, argument)) roll_stats(d, argument);
			break;

		case CON_ROLL_STATS:
			if (roll_stats(d, argument))
			{
				set_creation_state(d, CREATION_UNKNOWN);
				remort_complete(d->character);
			}
			break;

		}
		break;

	case CREATION_BLACKSMITH:
		switch ( con_state(d) )
		{

		default:
		bug( "Nanny: bad d->connected %d.", d->connected );
		close_socket( d );
		return;

		case CON_SMITH_WELCOME:
			if (smith_welcome(d, argument)) smith_level(d, "");
			break;

		case CON_SMITH_LEVEL:
			if (smith_level(d, argument)) smith_type(d, "");
			break;

		case CON_SMITH_TYPE:
			if (smith_type(d, argument)) smith_subtype(d, "");
			break;

		case CON_SMITH_SUBTYPE:
			if (smith_subtype(d, argument)) smith_quality(d, "");
			break;

		case CON_SMITH_QUALITY:
			if (smith_quality(d, argument)) smith_material(d, "");
			break;

		case CON_SMITH_MATERIAL:
			if (smith_material(d, argument)) smith_purchase(d, "");
			break;

		case CON_SMITH_PURCHASE:
			if (smith_purchase(d, argument)) d->connected=CON_PLAYING;
			break;

		case CON_SMITH_COLOR:
			if (smith_color(d, argument)) smith_purchase(d, "");
			break;

		case CON_SMITH_PERSONAL:
			if (smith_personal(d, argument)) smith_keywords(d, "");
			break;
	
		case CON_SMITH_KEYWORDS:
			if (smith_keywords(d, argument)) smith_purchase(d, "");
			break;

		case CON_SMITH_SELECT:
			if (smith_select(d, argument)) smith_inventory(d, "");
			break;

		case CON_SMITH_INVENTORY:
			if (smith_inventory(d, argument)) d->connected=CON_PLAYING;
			break;
	
		}
		break;		

   }
   return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
	/*
	 * Reserved words.
	 */
	if ( is_name( name, 
   "all auto immortal self someone something the you demise balance circle loner honor none questors") )

	return FALSE;
	
	/*
	 * Length restrictions.
	 */
	 
	if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
	if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
	if ( strlen(name) > 12 )
	return FALSE;
#endif

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
		if ( !isalpha(*pc) )
		return FALSE;

		if ( isupper(*pc)) /* ugly anti-caps hack */
		{
		if (adjcaps)
			cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
		}
		else
		adjcaps = FALSE;

		if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
		return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
		return FALSE;
	}

	/*
	 * Prevent players from naming themselves after mobs.
	 */
	{
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
		{
		if ( is_name( name, pMobIndex->player_name ) )
			return FALSE;
		}
	}
	}

	return TRUE;
}


int con_state(DESCRIPTOR_DATA *d)
{
	return ( d->connected % MAX_CON_STATE );
}

int creation_mode(DESCRIPTOR_DATA *d)
{
	return ( (d->connected - (d->connected%MAX_CON_STATE)) / MAX_CON_STATE );
}

void set_con_state(DESCRIPTOR_DATA *d, int cstate)
{
	d->connected += cstate - d->connected%MAX_CON_STATE;
	return;
}

void set_creation_state(DESCRIPTOR_DATA *d, int cmode)
{
	d->connected = d->connected%MAX_CON_STATE + cmode*MAX_CON_STATE;
	return;
}


bool	get_name ( DESCRIPTOR_DATA *d, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	bool fOld;
	
	if (con_state(d)!=CON_GET_NAME)
	{
		set_con_state(d, CON_GET_NAME);
		write_to_buffer(d,"Name: ",0);
		return FALSE;
	}

	if ( argument[0] == '\0' )
	{
		close_socket( d );
		return FALSE;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
		write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
		return FALSE;
	}

	fOld = load_char_obj( d, argument );

	if (IS_SET(d->character->act, PLR_DENY))
	{
		sprintf( buf, "Denying access to %s@%s.", argument, d->host );
		log_string( buf );
		write_to_buffer( d, "You are denied access.\n\r", 0 );
		close_socket( d );
		return FALSE;
	}

	if (check_ban(d->host,BAN_PERMIT) && !IS_SET(d->character->act,PLR_PERMIT))
	{
		write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
		close_socket(d);
		return FALSE;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
		fOld = TRUE;
	}
	else
	{
		if ( wizlock && !IS_IMMORTAL(d->character)) 
		{
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return FALSE;
		}
	}

	if ( fOld )
	{
		/* Old player */
		return get_old_password(d, argument);
	}
	else
	{
		/* New player */
		if (newlock)
		{
				write_to_buffer( d, "The game is newlocked.\n\r", 0 );
				close_socket( d );
				return FALSE;
			}

		if (check_ban(d->host,BAN_NEWBIES))
		{
		write_to_buffer(d,
			"New players are not allowed from your site.\n\r",0);
		close_socket(d);
		return FALSE;
		}
	
		return confirm_new_name(d, argument);
	}

	return FALSE;
}




bool	get_old_password ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];

	if (con_state(d) != CON_GET_OLD_PASSWORD)
	{
		set_con_state(d, CON_GET_OLD_PASSWORD);
		sprintf( buf, "Welcome back, %s.  What is your password? ", ch->name );
		write_to_buffer( d, buf, 0 );
		write_to_buffer( d, echo_off_str, 0 );

		return FALSE;
	}
	else
	{
#if defined(unix)
		write_to_buffer( d, "\n\r", 2 );
#endif
		if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
		{
			write_to_buffer( d, "Wrong Password.\n\r", 0 );
			close_socket( d );
			return FALSE;
		}
 
		write_to_buffer( d, echo_on_str, 0 );

		if (check_playing(d,ch->name))
			return FALSE;

		if ( check_reconnect( d, ch->name, TRUE ) )
			return FALSE;

		sprintf( buf, "%s@%s has connected.", ch->name, d->host );
		log_string( buf );
		wiznet(buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

		return TRUE;
	}
	
	return FALSE;
}



bool	confirm_new_name ( DESCRIPTOR_DATA *d, char *argument )
{
	char buf[MAX_STRING_LENGTH];

	if (con_state(d)!=CON_CONFIRM_NEW_NAME)
	{
		sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
		write_to_buffer( d, buf, 0 );
		set_con_state(d, CON_CONFIRM_NEW_NAME);
		return FALSE;
	}

	switch ( *argument )
	{
	case 'y': case 'Y':
		return TRUE;

	case 'n': case 'N':
		free_char( d->character );
		d->character = NULL;
		return get_name(d, argument);

	default:
		write_to_buffer( d, "Please type Yes or No? ", 0 );
		break;
	}
	
	return FALSE;
}




bool	get_new_password ( DESCRIPTOR_DATA *d, char *argument )
{
	char *pwdnew;
	char *p;
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];

	if (con_state(d) != CON_GET_NEW_PASSWORD)
	{
		sprintf( buf, "Ah, a new soul.  Welcome to your new home, %s.\n\rPlease enter a password for your new character: %s",
		d->character->name, echo_off_str );
		write_to_buffer( d, buf, 0 );
		set_con_state(d, CON_GET_NEW_PASSWORD);
		return FALSE;
	}

#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
		write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
		return FALSE;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
		if ( *p == '~' )
		{
		write_to_buffer( d,
			"New password not acceptable, try again.\n\rPassword: ",
			0 );
		return FALSE;
		}
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );

	return TRUE;
}




bool	confirm_new_password ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch=d->character;

	if (con_state(d) != CON_CONFIRM_NEW_PASSWORD)
	{
		write_to_buffer( d, "Please retype password: ", 0 );
		set_con_state(d, CON_CONFIRM_NEW_PASSWORD);
		return FALSE;
	}

#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
		write_to_buffer( d, "Passwords don't match, please retype it.\n\r", 0 );
		return (get_new_password(d, argument));
	}

	write_to_buffer( d, echo_on_str, 0 );

	return TRUE;
}




bool get_creation_mode(DESCRIPTOR_DATA *d, char *argument)
{
	char arg[MAX_STRING_LENGTH];

	if (con_state(d) != CON_GET_CREATION_MODE)
	{
		do_help(d->character,"creation header");
		write_to_buffer(d,"Which option do you choose(instant, quick, normal)? ",0);
		set_con_state(d, CON_GET_CREATION_MODE);
		return FALSE;
	}

	one_argument(argument,arg);

	if (!strcmp(arg, "instant"))
	{
		set_creation_state(d, CREATION_INSTANT);
		return TRUE;
	}

	if (!strcmp(arg, "quick"))
	{
		set_creation_state(d, CREATION_QUICK);
		return TRUE;
	}

	if (!strcmp(arg, "normal"))
	{
		set_creation_state(d, CREATION_NORMAL);
		return TRUE;
	}

	write_to_buffer(d,"That isn't a valid choice.\n\r",0);
	write_to_buffer(d,"Which creation option do you choose(instant, quick, normal)? ",0);
	return FALSE;
}
 



bool	get_new_race ( DESCRIPTOR_DATA *d, char *argument )
{
	char arg[MAX_STRING_LENGTH];
	CHAR_DATA *ch = d->character;
	int race, i;
	
	if (con_state(d) != CON_GET_NEW_RACE)
	{
		do_help(d->character, "race help");
			write_to_buffer(d,"The following races are available:\n\r  ",0);
			for ( race = 1; race_table[race].name != NULL; race++ )
			{
				if (!race_table[race].pc_race)
					break;
				if (pc_race_table[race].remorts > ch->pcdata->remorts)
					break;
				write_to_buffer(d,race_table[race].name,0);
				write_to_buffer(d," ",1);
			}
			write_to_buffer(d,"\n\r",0);
		if (ch->pcdata->remorts>0)
			write_to_buffer(d, "Type HELP REMORTRACE for information on remort races.\n\r ",0);
		write_to_buffer(d,"What is your race (for more information type HELP, STATS, or ETLS)? ",0);
		set_con_state(d, CON_GET_NEW_RACE);
		return FALSE;
	}

	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
		argument = one_argument(argument,arg);
		if (argument[0] == '\0')
		do_help(ch,"race help");
		else
		do_help(ch,argument);
		if (ch->pcdata->remorts>0)
			write_to_buffer(d, "Type HELP REMORTRACE for information on remort races.\n\r ",0);
		write_to_buffer(d,
		"What is your race (for more information type HELP, STATS, or ETLS)? ",0);
		return FALSE;
	}

	if (!strcmp(arg,"stats"))
	{
		do_stats(ch,"");
		if (ch->pcdata->remorts>0)
			write_to_buffer(d, "Type HELP REMORTRACE for information on remort races.\n\r ",0);
		write_to_buffer(d,
		"What is your race? (for more information type HELP, STATS, or ETLS) ",0);
		return FALSE;
	}

	if (!strcmp(arg,"etls"))
	{
		do_etls(ch,"");
		if (ch->pcdata->remorts>0)
			write_to_buffer(d, "Type HELP REMORTRACE for information on remort races.\n\r ",0);
		write_to_buffer(d,
		"What is your race? (for more information type HELP, STATS, or ETLS) ",0);
		return FALSE;
	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race ||
		pc_race_table[race].remorts > ch->pcdata->remorts)
	{
		write_to_buffer(d,"That is not a valid race.\n\r",0);
		write_to_buffer(d,"The following races are available:\n\r  ",0);
		for ( race = 1; race_table[race].name != NULL; race++ )
		{
			if (!race_table[race].pc_race)
				break;
			if (pc_race_table[race].remorts > ch->pcdata->remorts)
				break;
			write_to_buffer(d,race_table[race].name,0);
			write_to_buffer(d," ",1);
		}
		write_to_buffer(d,"\n\r",0);
		if (ch->pcdata->remorts>0)
			write_to_buffer(d, "Type HELP REMORTRACE for information on remort races.\n\r ",0);
		write_to_buffer(d,
		"What is your race? (for more information type HELP, STATS, or ETLS) ",0);
		return FALSE;
	}

	if (pc_race_table[race].gender == SEX_NEUTRAL)
	{
		ch->sex = SEX_NEUTRAL;
	}
	else if (pc_race_table[race].gender != SEX_BOTH
		&& ch->pcdata->true_sex!=pc_race_table[race].gender)
	{
		write_to_buffer(d,"You are the wrong gender for that race.\n\r",0);
		write_to_buffer(d,"The following races are available:\n\r  ",0);
		for ( race = 1; race_table[race].name != NULL; race++ )
		{
			if (!race_table[race].pc_race)
				break;
			if (pc_race_table[race].remorts > ch->pcdata->remorts)
				break;
			write_to_buffer(d,race_table[race].name,0);
			write_to_buffer(d," ",1);
		}
		write_to_buffer(d,"\n\r",0);
		if (ch->pcdata->remorts>0)
			write_to_buffer(d, "Type HELP REMORTRACE for information on remort races.\n\r ",0);
		write_to_buffer(d,
		"What is your race? (for more information type HELP, STATS, or ETLS) ",0);
		return FALSE;
	}
	

	ch->race = race;
	/* initialize stats */
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->imm_flags   = ch->imm_flags|race_table[race].imm;
	ch->res_flags   = ch->res_flags|race_table[race].res;
	ch->vuln_flags  = ch->vuln_flags|race_table[race].vuln;
	ch->form    = race_table[race].form;
	ch->parts   = race_table[race].parts;

	/* add skills */
	for (i = 0; i < pc_race_table[race].num_skills; i++)
	{
		group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	/* add cost */
	ch->pcdata->points =0;
	ch->size = pc_race_table[race].size;

	return TRUE;
}




bool	get_new_sex ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch = d->character;

	if (con_state(d)!=CON_GET_NEW_SEX)
	{
		write_to_buffer( d, "What is your sex (M/F)? ", 0 );
		set_con_state(d, CON_GET_NEW_SEX);
		return FALSE;
	}

	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
				ch->pcdata->true_sex = SEX_MALE;
				return TRUE;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
				ch->pcdata->true_sex = SEX_FEMALE;
				return TRUE;
	default:
		write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
		break;
	}

	return FALSE;
}




bool	get_new_class ( DESCRIPTOR_DATA *d, char *argument )
{
	int i;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
	CHAR_DATA *ch = d->character;

	if (con_state(d)!=CON_GET_NEW_CLASS)
	{
		do_help(ch, "classes");
		strcpy( buf, "Available classes: [" );
		for ( i = 0; i < MAX_CLASS; i++ )
		{
			if ( i > 0 )
			strcat( buf, " " );
			strcat( buf, class_table[i].name );
		}
		strcat( buf, "]\n\rChoose a class(for more information type HELP, STATS, or ETLS): " );
		write_to_buffer( d, buf, 0 );
		set_con_state(d, CON_GET_NEW_CLASS);

		return FALSE;
	}

        argument = one_argument(argument,arg);
	if (!strcmp(arg,"help"))
	{
		if (argument[0] == '\0')
		do_help(ch,"class help");
		else
		do_help(ch,argument);
			write_to_buffer(d,
		"What is your class (for more information type HELP, STATS, or ETLS)? ",0);
		return FALSE;
	}

	if (!strcmp(arg,"stats"))
	{
		do_stats(ch,"");
			write_to_buffer(d,
		"What is your class (for more information type HELP, STATS, or ETLS)? ",0);
		return FALSE;
	}

	if (!strcmp(arg,"etls"))
	{
		do_etls(ch,"");
			write_to_buffer(d,
		"What is your class (for more information type HELP, STATS, or ETLS)? ",0);
		return FALSE;
	}

	i = class_lookup(arg);

	if ( i == -1 )
	{
		write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
		return FALSE;
	}

	ch->class = i;

	write_to_buffer( d, "\n\r", 2 );

	return TRUE;
}




bool	get_alignment ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch=d->character;

	if (con_state(d)!= CON_GET_ALIGNMENT)
	{
		write_to_buffer( d, "You may be holy, good, moral, nice, indifferent, cruel, wicked, evil, or demonic.\n\r",0);
		write_to_buffer( d, "Which alignment (H/G/M/N/I/C/W/E/D)? ",0);
		set_con_state(d, CON_GET_ALIGNMENT);
		return FALSE;
	}

	switch( argument[0])
	{
		case 'h' : case 'H' : ch->alignment = 1000; break;
		case 'g' : case 'G' : ch->alignment = 750;  break;
		case 'm' : case 'M' : ch->alignment = 500;  break;
		case 'n' : case 'N' : ch->alignment = 250;  break;
		case 'i' : case 'I' : ch->alignment = 0;    break;
		case 'c' : case 'C' : ch->alignment = -250;  break;
		case 'w' : case 'W' : ch->alignment = -500; break;
		case 'e' : case 'E' : ch->alignment = -750; break;
		case 'd' : case 'D' : ch->alignment = -1000; break;
		default:
		write_to_buffer(d,"That's not a valid alignment.\n\r",0);
		write_to_buffer(d,"Which alignment (H/G/M/N/I/C/W/E/D)? ",0);
		return FALSE;
	}

	write_to_buffer(d,"\n\r",0);

	return TRUE;
}


void take_rom_basics(DESCRIPTOR_DATA *d)
{
	CHAR_DATA *ch=d->character;

	group_add(ch,"rom basics",FALSE);
	group_add(ch,class_table[ch->class].base_group,FALSE);
	ch->pcdata->learned[gsn_recall] = 50;
	
	return;
}	


void take_class_defaults(DESCRIPTOR_DATA *d)
{
	group_add(d->character,class_table[d->character->class].default_group,TRUE);
	return;
}


void take_default_weapon(DESCRIPTOR_DATA *d)
{
	int i;
	for(i=0; weapon_table[i].name!=NULL; i++)
		if (weapon_table[i].vnum == class_table[d->character->class].weapon)
			break;

	if (d->character->class == class_lookup("monk"))
		d->character->pcdata->learned[gsn_hand_to_hand] = 40;
	else
		d->character->pcdata->learned[*weapon_table[i].gsn] = 40;
}


void newbie_alert(DESCRIPTOR_DATA *d)
{
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "%s@%s new player.", d->character->name, d->host );
	log_string( buf );
	wiznet("Newbie alert!  $N sighted.",d->character,NULL,WIZ_NEWBIE,0,0);
	wiznet(buf,NULL,NULL,WIZ_SITES,0,0);
	return;
}


bool	default_choice ( DESCRIPTOR_DATA *d, char *argument )
{

	if (con_state(d) != CON_DEFAULT_CHOICE)
	{
		write_to_buffer(d,"Do you wish to customize this character?\n\r",0);
		write_to_buffer(d,"Customization takes time, but allows a wider range of skills and abilities.\n\r",0);
		write_to_buffer(d,"Customize (Y/N)? ",0);
		set_con_state(d, CON_DEFAULT_CHOICE);
		return FALSE;		
	}

	write_to_buffer(d,"\n\r",2);
	switch ( argument[0] )
	{
	case 'y': case 'Y': 
		return gen_groups(d, argument);
	case 'n': case 'N':
		take_class_defaults(d); 
		return TRUE;
	default:
		write_to_buffer( d, "Please answer (Y/N)? ", 0 );
		break;
	}
	
	return FALSE;
}




bool	gen_groups ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch=d->character;
	char buf[MAX_STRING_LENGTH];

	if (con_state(d) != CON_GEN_GROUPS)
	{
		ch->gen_data = new_gen_data();
		ch->gen_data->points_chosen = ch->pcdata->points;
		do_help(ch,"group header");
		list_group_costs(ch);
		write_to_buffer(d,"You already have the following skills:\n\r",0);
		do_skills(ch,"");
		send_to_char("List, learned, premise, add, drop, info, help, or done? ",ch);
		set_con_state(d, CON_GEN_GROUPS);
		return FALSE;		
	}

	send_to_char("\n\r",ch);
	if (!str_cmp(argument,"done"))
	{
		sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
		send_to_char(buf,ch);
		sprintf(buf,"Experience per level: %d\n\r",
				exp_per_level(ch,ch->gen_data->points_chosen));
		free_gen_data(ch->gen_data);
		ch->gen_data = NULL;
		send_to_char(buf,ch);
		return TRUE;
	}

	if (!parse_gen_groups(ch,argument))
		send_to_char("Thats not a valid choice.\n\r",ch);

	send_to_char("List, learned, premise, add, drop, info, help, or done? ",ch);
	
	return FALSE;
}




bool	pick_weapon ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch=d->character;
	int i, weapon;
	char buf[MAX_STRING_LENGTH];	

	if (con_state(d) != CON_PICK_WEAPON)
	{
		write_to_buffer( d, "\n\r", 2 );
		write_to_buffer(d,
		"Please pick a weapon from the following choices:\n\r",0);
		buf[0] = '\0';
		for ( i = 0; weapon_table[i].name != NULL; i++)
			if (ch->pcdata->learned[*weapon_table[i].gsn] > 0 
			&& skill_table[*weapon_table[i].gsn].skill_level[ch->class] == 1)
			{
				strcat(buf,weapon_table[i].name);
				strcat(buf," ");
			}
		if (ch->pcdata->learned[gsn_hand_to_hand]>0)
			strcat(buf,"unarmed");
		strcat(buf,"\n\rYour choice(press enter to take your class default)? ");
		write_to_buffer(d,buf,0);
		set_con_state(d, CON_PICK_WEAPON);
		return FALSE;
	}

	write_to_buffer(d,"\n\r",2);

	if (argument[0]=='\0')
	{
		take_default_weapon(d);
		return TRUE;
	}		

	weapon = weapon_lookup(argument);
	if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
	{
		if (strcmp(argument, "unarmed")||
			ch->pcdata->learned[gsn_hand_to_hand]<=0)
		{
			write_to_buffer(d, "That's not a valid selection. Choices are:\n\r",0);
			buf[0] = '\0';
			for ( i = 0; weapon_table[i].name != NULL; i++)
				if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
				{
					strcat(buf,weapon_table[i].name);
					strcat(buf," ");
				}
			if (ch->pcdata->learned[gsn_hand_to_hand]>0)
				strcat(buf,"unarmed");
			strcat(buf,"\n\rYour choice(press enter to take your class default)? ");
			write_to_buffer(d,buf,0);
			return FALSE;
		}
	}

	if (weapon==-1)
		ch->pcdata->learned[gsn_hand_to_hand] = 40;
	else
		ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;

	write_to_buffer(d,"\n\r",2);
	return TRUE;
}



bool	roll_stats ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch=d->character;

	if (con_state(d) != CON_ROLL_STATS)
	{
		ch->gen_data = new_gen_data();
		do_help(ch,"rollstat header");
		parse_roll_stats(ch, "reroll");
		send_to_char("Show, reroll, assign, unassign, default, help, or done? ",ch);
		set_con_state(d, CON_ROLL_STATS);
		return FALSE;
	}

	send_to_char("\n\r",ch);
	if (!str_cmp(argument,"done"))
	{
		if (ch->gen_data->unused_die[0]!=-1)
		{
			send_to_char("You haven't finished setting your priorities.  Finish assigning them or type default.\n\r",ch);
			return FALSE;
		}
		free_gen_data(ch->gen_data);
		ch->gen_data = NULL;
		return TRUE;
	}

	if (!str_cmp(argument,"default"))
	{
		free_gen_data(ch->gen_data);
		ch->gen_data=NULL;
		send_to_char("The game will select stats for you based on your class.\n\r",ch);
		take_default_stats(ch);
		return TRUE;
	}	

	if (!parse_roll_stats(ch,argument))
		send_to_char("Thats not a valid choice.\n\r",ch);

	send_to_char("Show, reroll, assign, unassign, default, help, or done? ",ch);
	
	return FALSE;
}




bool	get_stat_priority ( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA *ch=d->character;
	int i;

	if (con_state(d) != CON_GET_STAT_PRIORITY)
	{
		ch->gen_data = new_gen_data();
		for (i = 0; i<MAX_STATS; i++)
		{
			ch->gen_data->stat_priority[i]=-1;
			ch->gen_data->unused_die[i]=TRUE;
		}
		parse_stat_priority(ch, "help");
		send_to_char("Type a statistic or type default, undo, help, or done? ",ch);
		set_con_state(d, CON_GET_STAT_PRIORITY);
		return FALSE;
	}

	send_to_char("\n\r",ch);
	if (!str_cmp(argument,"done"))
	{
		if (ch->gen_data->stat_priority[MAX_STATS-1]==-1)
		{
			send_to_char("You haven't finished setting your priorities.  Finish assigning them or type default.\n\r",ch);
			return FALSE;
		}
		get_random_stats(ch);
		free_gen_data(ch->gen_data);
		ch->gen_data = NULL;
		return TRUE;
	}

	if (!str_cmp(argument,"default"))
	{
		free_gen_data(ch->gen_data);
		ch->gen_data=NULL;
		send_to_char("The game will select stats for you based on your class.\n\r",ch);
		take_default_stats(ch);
		return TRUE;
	}	

	if (!parse_stat_priority(ch,argument))
		send_to_char("Thats not a valid choice.\n\r",ch);

	send_to_char("Type a statistic or type default, undo, help, or done? ",ch);
	
	return FALSE;
}




bool	read_imotd ( DESCRIPTOR_DATA *d, char *argument )
{
	if (!IS_IMMORTAL(d->character))
		return read_motd(d, argument);
	
	if (con_state(d) != CON_READ_IMOTD)
	{
		set_con_state(d, CON_READ_IMOTD);
		do_help( d->character, "imotd" );
		return FALSE;
	}

	return TRUE;
}




bool	read_motd ( DESCRIPTOR_DATA *d, char *argument )
{
	if (con_state(d) != CON_READ_MOTD)
	{
		set_con_state(d, CON_READ_MOTD);
		do_help( d->character, "motd" );
		return FALSE;
	}

	return TRUE;
}


bool	break_connect ( DESCRIPTOR_DATA *d, char *argument )
{
	DESCRIPTOR_DATA *d_old, *d_next;

	if (con_state(d)!=CON_BREAK_CONNECT)
	{
		write_to_buffer( d, "That character is already playing.\n\r",0);
		write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
		set_con_state(d, CON_BREAK_CONNECT);
		return FALSE;
	}
	else
	switch( *argument )
	{
	case 'y' : case 'Y':
		for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
		{
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
			continue;

		if (str_cmp(d->character->name,d_old->original ?
			d_old->original->name : d_old->character->name))
			continue;

		close_socket(d_old);
		}
		if (check_reconnect(d,d->character->name,TRUE))
			return FALSE;
		write_to_buffer(d,"Reconnect attempt failed.\n\r",0);
			if ( d->character != NULL )
			{
				free_char( d->character );
				d->character = NULL;
			}
		return TRUE;

	case 'n' : case 'N':
			if ( d->character != NULL )
			{
				free_char( d->character );
				d->character = NULL;
			}
		return TRUE;

	default:
		write_to_buffer(d,"Please type Y or N? ",0);
		break;
	}

	return FALSE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
	CHAR_DATA *ch;
	char buf[MAX_STRING_LENGTH];

	for ( ch = char_list; ch != NULL; ch = ch->next )
	{
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
		if ( fConn == FALSE )
		{
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
		}
		else
		{
		free_char( d->character );
		d->character = ch;
		ch->desc     = d;
		ch->timer    = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		if (buf_string(ch->pcdata->buffer)[0] != '\0')
		   send_to_char( "Type replay to see missed tells.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

		sprintf( buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( buf );
		wiznet("$N groks the fullness of $S link.",
			ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
            /* Inform the character of a note in progress and the possibility of continuation */ 
            if (ch->pcdata->in_progress)
               send_to_char ("You have a note in progress. Type NOTE WRITE to continue it.\n\r",ch);
		}
		return TRUE;
	}
	}

	return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
	DESCRIPTOR_DATA *dold;

	for ( dold = descriptor_list; dold; dold = dold->next )
	{
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
			 ? dold->original->name : dold->character->name ) )
	{
		break_connect(d, "");
		return TRUE;
	}
	}

	return FALSE;
}


void enter_game ( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch=d->character;
	char buf[MAX_STRING_LENGTH];

	if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
	{
		write_to_buffer( d, "Warning! Null password!\n\r",0 );
		write_to_buffer( d, "Please report old password with bug.\n\r",0);
		write_to_buffer( d,
			"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, 
		"\n\rWelcome to ROM 2.4.  Please do not feed the mobiles.\n\r",
		0 );
	ch->next    = char_list;
	char_list   = ch;
	d->connected    = CON_PLAYING;
	
	free_string(ch->pcdata->last_host);
	ch->pcdata->last_host = str_dup(d->host);

	reset_char(ch);

	if ( ch->level == 0 )
	{
		ch->level   = 1;
		ch->exp     = exp_per_level(ch,ch->pcdata->points);
		ch->hit     = ch->max_hit;
		ch->mana    = ch->max_mana;
		ch->move    = ch->max_move;
      ch->silver  = 50;

		if (ch->pcdata->points < 50)
		  ch->train = (91 - ch->pcdata->points) / 2;
		else  ch->train    = 20;
		  ch->practice = 5;

      if (ch->class == class_lookup("assassin"))
         SET_BIT(ch->act, PLR_PERM_PKILL);

		SET_BIT(ch->act, PLR_AUTOEXIT);
		SET_BIT(ch->act, PLR_AUTOLOOT);
		SET_BIT(ch->act, PLR_AUTOGOLD);
		SET_BIT(ch->act, PLR_AUTOSPLIT);

		sprintf( buf, "the %s",
		title_table [ch->class] [(ch->level+4-(ch->level+4)%5)/5]);
		set_title( ch, buf );

		do_outfit(ch,"");
		obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

		char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
		send_to_char("\n\r",ch);
		do_help(ch,"NEWBIE INFO");
		send_to_char("\n\r",ch);
	}
	else if ( ch->in_room != NULL )
	{
		char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
		char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
		char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}
	
	update_bounty(ch);

	wiznet("$N has left real life behind.",ch,NULL,
		WIZ_LOGINS,WIZ_SITES,get_trust(ch));

	sprintf(buf, "%s has decided to join us.", ch->name);
      do_info_message(ch, buf, FALSE);

	act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
	if (ch->pet != NULL)
	{
		char_to_room(ch->pet,ch->in_room);
		act("$n appears in the room.",ch->pet,NULL,NULL,TO_ROOM);
	}
      do_board(ch,"");

	return;
}




