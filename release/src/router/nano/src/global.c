/* $Id: global.c 4520 2010-11-12 06:23:14Z astyanax $ */
/**************************************************************************
 *   global.c  --  This file is part of GNU nano.                         *
 *                                                                        *
 *   Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,  *
<<<<<<< HEAD
 *   2008, 2009 Free Software Foundation, Inc.                            *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 3, or (at your option)  *
 *   any later version.                                                   *
=======
 *   2008, 2009, 2010, 2011, 2013, 2014 Free Software Foundation, Inc.    *
 *   Copyright (C) 2014, 2015, 2016 Benno Schulenberg                     *
>>>>>>> origin/tomato-shibby-RT-AC
 *                                                                        *
 *   GNU nano is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published    *
 *   by the Free Software Foundation, either version 3 of the License,    *
 *   or (at your option) any later version.                               *
 *                                                                        *
 *   GNU nano is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#include "proto.h"

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include "assert.h"

/* Global variables. */
#ifndef NANO_TINY
<<<<<<< HEAD
sigjmp_buf jump_buf;
	/* Used to return to either main() or the unjustify routine in
	 * do_justify() after a SIGWINCH. */
bool jump_buf_main = FALSE;
	/* Have we set jump_buf so that we return to main() after a
	 * SIGWINCH? */
=======
volatile sig_atomic_t the_window_resized = FALSE;
	/* Set to TRUE by the handler whenever a SIGWINCH occurs. */
#endif

#ifdef __linux__
bool console;
	/* Whether we're running on a Linux VC (TRUE) or under X (FALSE). */
#endif

bool meta_key;
	/* Whether the current keystroke is a Meta key. */
bool shift_held;
	/* Whether Shift was being held together with a movement key. */
bool focusing = TRUE;
	/* Whether an update of the edit window should center the cursor. */

bool as_an_at = TRUE;
	/* Whether a 0x0A byte should be shown as a ^@ instead of a ^J. */

int margin = 0;
	/* The amount of space reserved at the left for line numbers. */
int editwincols = -1;
	/* The number of usable columns in the edit window: COLS - margin. */

message_type lastmessage = HUSH;
	/* Messages of type HUSH should not overwrite type MILD nor ALERT. */

filestruct *pletion_line = NULL;
	/* The line where the last completion was found, if any. */

int controlleft, controlright, controlup, controldown;
#ifndef NANO_TINY
int shiftcontrolleft, shiftcontrolright, shiftcontrolup, shiftcontroldown;
int shiftaltleft, shiftaltright, shiftaltup, shiftaltdown;
>>>>>>> origin/tomato-shibby-RT-AC
#endif

#ifndef DISABLE_WRAPJUSTIFY
ssize_t fill = 0;
	/* The column where we will wrap lines. */
ssize_t wrap_at = -CHARS_FROM_EOL;
	/* The position where we will wrap lines.  fill is equal to this
	 * if it's greater than zero, and equal to (COLS + this) if it
	 * isn't. */
#endif

char *last_search = NULL;
	/* The last string we searched for. */
char *last_replace = NULL;
	/* The last replacement string we searched for. */

unsigned flags[4] = {0, 0, 0, 0};
	/* Our flag containing the states of all global options. */
WINDOW *topwin;
	/* The top portion of the window, where we display the version
	 * number of nano, the name of the current file, and whether the
	 * current file has been modified. */
WINDOW *edit;
	/* The middle portion of the window, i.e. the edit window, where
	 * we display the current file we're editing. */
WINDOW *bottomwin;
	/* The bottom portion of the window, where we display statusbar
	 * messages, the statusbar prompt, and a list of shortcuts. */
int editwinrows = 0;
	/* How many rows does the edit window take up? */
<<<<<<< HEAD
int maxrows = 0;
	/* How many usable lines are there (due to soft wrapping) */
=======
>>>>>>> origin/tomato-shibby-RT-AC

filestruct *cutbuffer = NULL;
	/* The buffer where we store cut text. */
filestruct *cutbottom = NULL;
	/* The last line in the cutbuffer. */
partition *filepart = NULL;
	/* The "partition" where we store a portion of the current file. */
openfilestruct *openfile = NULL;
	/* The list of all open file buffers. */

#ifndef NANO_TINY
char *matchbrackets = NULL;
	/* The opening and closing brackets that can be found by bracket
	 * searches. */
#endif

#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
char *whitespace = NULL;
	/* The characters used when displaying the first characters of
	 * tabs and spaces. */
int whitespace_len[2];
	/* The length of these characters. */
#endif

#ifndef DISABLE_JUSTIFY
char *punct = NULL;
	/* The closing punctuation that can end sentences. */
char *brackets = NULL;
	/* The closing brackets that can follow closing punctuation and
	 * can end sentences. */
char *quotestr = NULL;
	/* The quoting string.  The default value is set in main(). */
regex_t quotereg;
	/* The compiled regular expression from the quoting string. */
int quoterc;
	/* Whether it was compiled successfully. */
char *quoteerr = NULL;
	/* The error message, if it didn't. */
#endif /* !DISABLE_JUSTIFY */

char *word_chars = NULL;
	/* Nonalphanumeric characters that also form words. */

bool nodelay_mode = FALSE;
	/* Are we in nodelay mode (checking for a cancel wile doing something */

char *answer = NULL;
	/* The answer string used by the statusbar prompt. */

ssize_t tabsize = -1;
	/* The width of a tab in spaces.  The default is set in main(). */

#ifndef NANO_TINY
char *backup_dir = NULL;
	/* The directory where we store backup files. */
#endif
#ifndef DISABLE_OPERATINGDIR
char *operating_dir = NULL;
	/* The relative path to the operating directory, which we can't
	 * move outside of. */
char *full_operating_dir = NULL;
	/* The full path to it. */
#endif

#ifndef DISABLE_SPELLER
char *alt_speller = NULL;
	/* The command to use for the alternate spell checker. */
#endif

#ifdef ENABLE_COLOR
syntaxtype *syntaxes = NULL;
	/* The global list of color syntaxes. */
char *syntaxstr = NULL;
	/* The color syntax name specified on the command line. */

#endif

bool edit_refresh_needed = NULL;
	/* Did a command mangle enough of the buffer refresh that we 
	   should repaint the screen */

const shortcut *currshortcut;
	/* The current shortcut list we're using. */
int currmenu;
	/* The currently loaded menu */

sc *sclist = NULL;
<<<<<<< HEAD
	/* New shortcut key struct */
subnfunc *allfuncs = NULL;
	/* New struct for the function list */

#ifndef NANO_TINY
=======
	/* The start of the shortcuts list. */
subnfunc *allfuncs = NULL;
	/* The start of the functions list. */
subnfunc *tailfunc;
	/* The last function in the list. */
subnfunc *exitfunc;
	/* A pointer to the special Exit/Close item. */
subnfunc *uncutfunc;
	/* A pointer to the special Uncut/Unjustify item. */

#ifndef DISABLE_HISTORIES
>>>>>>> origin/tomato-shibby-RT-AC
filestruct *search_history = NULL;
	/* The search string history list. */
filestruct *searchage = NULL;
	/* The top of the search string history list. */
filestruct *searchbot = NULL;
	/* The bottom of the search string history list. */
filestruct *replace_history = NULL;
	/* The replace string history list. */
filestruct *replaceage = NULL;
	/* The top of the replace string history list. */
filestruct *replacebot = NULL;
	/* The bottom of the replace string history list. */
#endif

regex_t search_regexp;
	/* The compiled regular expression to use in searches. */
regmatch_t regmatches[10];
	/* The match positions for parenthetical subexpressions, 10
	 * maximum, used in regular expression searches. */

<<<<<<< HEAD
int reverse_attr = A_REVERSE;
	/* The curses attribute we use for reverse video. */
=======
int hilite_attribute = A_REVERSE;
	/* The curses attribute we use to highlight something. */
#ifndef DISABLE_COLOR
char* specified_color_combo[] = {};
	/* The color combinations as specified in the rcfile. */
#endif
int interface_color_pair[] = {};
	/* The processed color pairs for the interface elements. */
>>>>>>> origin/tomato-shibby-RT-AC

char *homedir = NULL;
	/* The user's home directory, from $HOME or /etc/passwd. */

<<<<<<< HEAD
/* Return the number of entries in the shortcut list s for a given menu. */
=======

/* Return the number of entries in the shortcut list for a given menu. */
>>>>>>> origin/tomato-shibby-RT-AC
size_t length_of_list(int menu)
{
    subnfunc *f;
    size_t i = 0;

    for (f = allfuncs; f != NULL; f = f->next)
<<<<<<< HEAD
        if ((f->menus & menu) != 0
#ifndef DISABLE_HELP
	    && strlen(f->help) > 0
#endif
	                          ) {
=======
	if ((f->menus & menu) && first_sc_for(menu, f->scfunc) != NULL)
>>>>>>> origin/tomato-shibby-RT-AC
	    i++;

    return i;
}

/* Set type of function based on the string */
function_type strtokeytype(const char *str)
{
    if (str[0] ==  'M' || str[0] == 'm')
        return META;
    else if (str[0] == '^')
        return CONTROL;
    else if (str[0] ==  'F' || str[0] == 'F')
        return FKEY;
    else
	return RAW;
}

/* Add a string to the new function list strict.
   Does not allow updates, yet anyway */
void add_to_funcs(short func, int menus, const char *desc, const char *help,
    bool blank_after, bool viewok)
{
    subnfunc *f;

    if (allfuncs == NULL) {
	allfuncs = (subnfunc *) nmalloc(sizeof(subnfunc));
	f = allfuncs;
    } else {
	for (f = allfuncs; f->next != NULL; f = f->next)
		;
        f->next = (subnfunc *)nmalloc(sizeof(subnfunc));
        f = f->next;
    }
    f->next = NULL;
    f->scfunc = func;
    f->menus = menus;
    f->desc = desc;
    f->viewok = viewok;
#ifndef DISABLE_HELP
    f->help = help;
    f->blank_after = blank_after;
#endif

#ifdef DEBUG
    fprintf(stderr, "Added func \"%s\"", f->desc);
#endif
}

<<<<<<< HEAD
const sc *first_sc_for(int menu, short func) {
    const sc *s;
    const sc *metasc = NULL;
=======
/* Add a key combo to the shortcut list. */
void add_to_sclist(int menus, const char *scstring, const int keycode,
			void (*func)(void), int toggle)
{
    static sc *tailsc;
#ifndef NANO_TINY
    static int counter = 0;
#endif
    sc *s = (sc *)nmalloc(sizeof(sc));
>>>>>>> origin/tomato-shibby-RT-AC

    for (s = sclist; s != NULL; s = s->next) {
	if ((s->menu & menu) && s->scfunc == func) {
	    /* try to use a meta sequence as a last resort.  Otherwise
	       we will run into problems when we try and handle things like
	       the arrow keys, home, etc, if for some reason the user bound
	       them to a meta sequence first *shrug* */
	    if (s->type == META) {
		metasc = s;
		continue;
	    } /* otherwise it was something else, use it */
	    return s;
	}
    }

<<<<<<< HEAD
    /* If we're here we may have found only meta sequences, if so use one */
    if (metasc)
	return metasc;

#ifdef DEBUG
    fprintf(stderr, "Whoops, returning null given func %ld in menu %d\n", (long) func, menu);
=======
    /* Fill in the data. */
    s->menus = menus;
    s->scfunc = func;
#ifndef NANO_TINY
    s->toggle = toggle;
    if (toggle)
	s->ordinal = ++counter;
#endif
    assign_keyinfo(s, scstring, keycode);

#ifdef DEBUG
    fprintf(stderr, "Setting keycode to %d for shortcut \"%s\" in menus %x\n", s->keycode, scstring, s->menus);
>>>>>>> origin/tomato-shibby-RT-AC
#endif
    /* Otherwise... */
    return NULL;
}


/* Add a string to the new shortcut list implementation
   Allows updates to existing entries in the list */
void add_to_sclist(int menu, const char *scstring, short func, int toggle, int execute)
{
    sc *s;

    if (sclist == NULL) {
	sclist = (sc *) nmalloc(sizeof(sc));
	s = sclist;
        s->next = NULL;
    } else {
	for (s = sclist; s->next != NULL; s = s->next)
            if (s->menu == menu && s->keystr == scstring)
		break;

        if (s->menu != menu || s->keystr != scstring) { /* i.e. this is not a replace... */
#ifdef DEBUG
            fprintf(stderr, "No match found...\n");
#endif
	    s->next = (sc *)nmalloc(sizeof(sc));
	    s = s->next;
            s->next = NULL;
        }
    }

    s->type = strtokeytype(scstring);
    s->menu = menu;
    s->toggle = toggle;
    s->keystr = (char *) scstring;
    s->scfunc = func;
    s->execute = execute;
    assign_keyinfo(s);

#ifdef DEBUG
    fprintf(stderr, "list val = %d\n", (int) s->menu);
    fprintf(stderr, "Hey, set sequence to %d for shortcut \"%s\"\n", s->seq, scstring);
#endif
}

<<<<<<< HEAD
/* Return the given menu's first shortcut sequence, or the default value
  (2nd arg).  Assumes currmenu for the menu to check*/
int sc_seq_or (short func, int defaultval) 
{
    const sc *s = first_sc_for(currmenu, func);

    if (s)
        return s->seq;
    /* else */
    return defaultval;

}

/* Assign the info to the shortcut struct 
   Assumes keystr is already assigned, naturally */
void assign_keyinfo(sc *s)
{
    if (s->type == CONTROL) {
        assert(strlen(s->keystr) > 1);
        s->seq = s->keystr[1] - 64;
    } else if (s->type == META) {
        assert(strlen(s->keystr) > 2);
        s->seq = tolower((int) s->keystr[2]);
    } else if (s->type == FKEY) {
        assert(strlen(s->keystr) > 1);
        s->seq = KEY_F0 + atoi(&s->keystr[1]);
    } else /* raw */
        s->seq = (int) s->keystr[0];

    /* Override some keys which don't bind as nicely as we'd like */
    if (s->type == CONTROL && (!strcasecmp(&s->keystr[1], "space")))
	s->seq = 0;
    else if (s->type == META && (!strcasecmp(&s->keystr[2], "space")))
	s->seq = (int) ' ';
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kup")))
	s->seq = KEY_UP;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kdown")))
	s->seq = KEY_DOWN;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kleft")))
	s->seq = KEY_LEFT;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kright")))
	s->seq = KEY_RIGHT;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kinsert")))
	s->seq = KEY_IC;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kdel")))
	s->seq = KEY_DC;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kbsp")))
	s->seq = KEY_BACKSPACE;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kenter")))
	s->seq = KEY_ENTER;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kpup")))
	s->seq = KEY_PPAGE;
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kpdown")))
	s->seq = KEY_NPAGE;
#ifdef KEY_HOME
    else if (s->type == RAW && (!strcasecmp(s->keystr, "khome")))
	s->seq = KEY_HOME;
#endif
#ifdef KEY_END
    else if (s->type == RAW && (!strcasecmp(s->keystr, "kend")))
	s->seq = KEY_END;
#endif

=======
/* Return the first keycode that is bound to the given function in the
 * current menu, if any; otherwise, return the given default value. */
int the_code_for(void (*func)(void), int defaultval)
{
    const sc *s = first_sc_for(currmenu, func);

    if (s == NULL)
	return defaultval;

    meta_key = s->meta;
    return s->keycode;
}

/* Return a pointer to the function that is bound to the given key. */
functionptrtype func_from_key(int *kbinput)
{
    const sc *s = get_shortcut(kbinput);

    if (s)
	return s->scfunc;
    else
	return NULL;
}

/* Set the string and its corresponding keycode for the given shortcut s. */
void assign_keyinfo(sc *s, const char *keystring, const int keycode)
{
    s->keystr = keystring;
    s->meta = (keystring[0] == 'M');

    assert(strlen(keystring) > 1 && (!s->meta || strlen(keystring) > 2));

    if (keycode)
	s->keycode = keycode;
    else if (keystring[0] == '^') {
	if (strcasecmp(keystring, "^Space") == 0)
	    s->keycode = 0;
	else
	    s->keycode = keystring[1] - 64;
    } else if (s->meta) {
	if (strcasecmp(keystring, "M-Space") == 0)
	    s->keycode = (int)' ';
	else
	    s->keycode = tolower((unsigned char)keystring[2]);
    } else if (keystring[0] == 'F')
	s->keycode = KEY_F0 + atoi(&keystring[1]);
    else if (!strcasecmp(keystring, "Ins"))
	s->keycode = KEY_IC;
    else if (!strcasecmp(keystring, "Del"))
	s->keycode = KEY_DC;
>>>>>>> origin/tomato-shibby-RT-AC
}

#ifdef DEBUG

void print_sclist(void)
{
    sc *s;
    const subnfunc *f;

    for (s = sclist; s->next != NULL; s = s->next) {
	f = sctofunc(s);
        if (f)
	    fprintf(stderr, "Shortcut \"%s\", function: %s, menus %d\n",  s->keystr, f->desc, f->menus);
	else
	    fprintf(stderr, "Hmm, didnt find a func for \"%s\"\n", s->keystr);
    }

}
#endif


/* Stuff we need to make at least static here so we can access it below */
/* TRANSLATORS: Try to keep the next five strings at most 10 characters. */
const char *cancel_msg = N_("Cancel");
const char *replace_msg = N_("Replace");
const char *no_replace_msg = N_("No Replace");

#ifndef NANO_TINY
const char *case_sens_msg = N_("Case Sens");
const char *backwards_msg = N_("Backwards");
#endif

#ifdef HAVE_REGEX_H
const char *regexp_msg = N_("Regexp");
#endif

/* Stuff we want to just stun out if we're in TINY mode */
#ifdef NANO_TINY
const char *gototext_msg = "";
const char *do_para_begin_msg = "";
const char *do_para_end_msg = "";
const char *case_sens_msg = "";
const char *backwards_msg = "";
const char *do_cut_till_end = "";
const char *dos_format_msg = "";
const char *mac_format_msg = "";
const char *append_msg = "";
const char *prepend_msg = "";
const char *backup_file_msg = "";
const char *to_files_msg = "";
const char *first_file_msg = "";
const char *whereis_next_msg = "";
const char *last_file_msg = "";
const char *new_buffer_msg = "";
const char *goto_dir_msg;
const char *ext_cmd_msg = "";

#else
/* TRANSLATORS: Try to keep the next five strings at most 10 characters. */
const char *prev_history_msg = N_("PrevHstory");
const char *next_history_msg = N_("NextHstory");
const char *gototext_msg = N_("Go To Text");
/* TRANSLATORS: Try to keep the next three strings at most 12 characters. */
const char *whereis_next_msg = N_("WhereIs Next");
#ifndef DISABLE_BROWSER
const char *first_file_msg = N_("First File");
const char *last_file_msg = N_("Last File");
/* TRANSLATORS: Try to keep the next nine strings at most 16 characters. */
const char *to_files_msg = N_("To Files");
#endif
const char *dos_format_msg = N_("DOS Format");
const char *mac_format_msg = N_("Mac Format");
const char *append_msg = N_("Append");
const char *prepend_msg = N_("Prepend");
const char *backup_file_msg = N_("Backup File");
const char *ext_cmd_msg = N_("Execute Command");
#ifdef ENABLE_MULTIBUFFER
const char *new_buffer_msg = N_("New Buffer");
#endif
const char *goto_dir_msg = N_("Go To Dir");

#endif /* NANO_TINY */

/* Initialize all shortcut lists.  If unjustify is TRUE, replace the
 * Uncut shortcut in the main shortcut list with UnJustify. */
void shortcut_init(bool unjustify)
{
    /* TRANSLATORS: Try to keep the following strings at most 10 characters. */
    const char *get_help_msg = N_("Get Help");
    const char *exit_msg = N_("Exit");
    const char *whereis_msg = N_("Where Is");
    const char *prev_page_msg = N_("Prev Page");
    const char *next_page_msg = N_("Next Page");
    const char *first_line_msg = N_("First Line");
    const char *last_line_msg = N_("Last Line");
    const char *suspend_msg = N_("Suspend");
#ifndef DISABLE_JUSTIFY
    const char *beg_of_par_msg = N_("Beg of Par");
    const char *end_of_par_msg = N_("End of Par");
    const char *fulljstify_msg = N_("FullJstify");
#endif
<<<<<<< HEAD
    const char *refresh_msg = N_("Refresh");
#ifndef NANO_TINY
    const char *insert_file_msg =  N_("Insert File");
#endif
    const char *go_to_line_msg = N_("Go To Line");
=======
    const char *refresh_tag = N_("Refresh");
    /* TRANSLATORS: Try to keep this string at most 12 characters. */
    const char *whereis_next_tag = N_("WhereIs Next");
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef DISABLE_JUSTIFY
    const char *nano_justify_msg = N_("Justify the current paragraph");
#endif
#ifndef DISABLE_HELP
    /* TRANSLATORS: The next long series of strings are shortcut descriptions;
     * they are best kept shorter than 56 characters, but may be longer. */
    const char *nano_cancel_msg = N_("Cancel the current function");
    const char *nano_help_msg = N_("Display this help text");
    const char *nano_exit_msg =
#ifdef ENABLE_MULTIBUFFER
	N_("Close the current file buffer / Exit from nano")
#else
   	N_("Exit from nano")
#endif
	;
    const char *nano_writeout_msg =
	N_("Write the current file to disk");
    const char *nano_insert_msg =
	N_("Insert another file into the current one");
    const char *nano_whereis_msg =
	N_("Search for a string or a regular expression");
<<<<<<< HEAD
    const char *nano_prevpage_msg = N_("Go to previous screen");
    const char *nano_nextpage_msg = N_("Go to next screen");
=======
#ifndef DISABLE_BROWSER
    const char *nano_browser_whereis_msg = N_("Search for a string");
    const char *nano_browser_refresh_msg = N_("Refresh the file list");
#ifndef NANO_TINY
    const char *nano_browser_lefthand_msg = N_("Go to lefthand column");
    const char *nano_browser_righthand_msg = N_("Go to righthand column");
#endif
#endif
    const char *nano_prevpage_msg = N_("Go one screenful up");
    const char *nano_nextpage_msg = N_("Go one screenful down");
>>>>>>> origin/tomato-shibby-RT-AC
    const char *nano_cut_msg =
	N_("Cut the current line and store it in the cutbuffer");
    const char *nano_uncut_msg =
	N_("Uncut from the cutbuffer into the current line");
<<<<<<< HEAD
    const char *nano_cursorpos_msg =
	N_("Display the position of the cursor");
    const char *nano_spell_msg =
	N_("Invoke the spell checker, if available");
    const char *nano_replace_msg =
	N_("Replace a string or a regular expression");
     const char *nano_gotoline_msg = N_("Go to line and column number");
#ifndef NANO_TINY
    const char *nano_mark_msg = N_("Mark text at the cursor position");
    const char *nano_whereis_next_msg = N_("Repeat last search");
=======
    const char *nano_cursorpos_msg = N_("Display the position of the cursor");
#ifndef DISABLE_SPELLER
    const char *nano_spell_msg = N_("Invoke the spell checker, if available");
#endif
    const char *nano_replace_msg = N_("Replace a string or a regular expression");
    const char *nano_gotoline_msg = N_("Go to line and column number");
    const char *nano_whereis_next_msg = N_("Repeat the last search");
#ifndef NANO_TINY
    const char *nano_mark_msg = N_("Mark text starting from the cursor position");
>>>>>>> origin/tomato-shibby-RT-AC
    const char *nano_copy_msg =
	N_("Copy the current line and store it in the cutbuffer");
    const char *nano_indent_msg = N_("Indent the current line");
    const char *nano_unindent_msg = N_("Unindent the current line");
    const char *nano_undo_msg = N_("Undo the last operation");
    const char *nano_redo_msg = N_("Redo the last undone operation");
#endif
    const char *nano_forward_msg = N_("Go forward one character");
<<<<<<< HEAD
    const char *nano_back_msg = N_("Go back one character");
#ifndef NANO_TINY
    const char *nano_nextword_msg = N_("Go forward one word");
    const char *nano_prevword_msg = N_("Go back one word");
#endif
=======
    const char *nano_prevword_msg = N_("Go back one word");
    const char *nano_nextword_msg = N_("Go forward one word");
>>>>>>> origin/tomato-shibby-RT-AC
    const char *nano_prevline_msg = N_("Go to previous line");
    const char *nano_nextline_msg = N_("Go to next line");
    const char *nano_home_msg = N_("Go to beginning of current line");
    const char *nano_end_msg = N_("Go to end of current line");
    const char *nano_prevblock_msg = N_("Go to previous block of text");
    const char *nano_nextblock_msg = N_("Go to next block of text");
#ifndef DISABLE_JUSTIFY
    const char *nano_parabegin_msg =
	N_("Go to beginning of paragraph; then of previous paragraph");
    const char *nano_paraend_msg =
	N_("Go just beyond end of paragraph; then of next paragraph");
#endif
    const char *nano_firstline_msg =
	N_("Go to the first line of the file");
    const char *nano_lastline_msg =
	N_("Go to the last line of the file");
#ifndef NANO_TINY
    const char *nano_bracket_msg = N_("Go to the matching bracket");
    const char *nano_scrollup_msg =
	N_("Scroll up one line without scrolling the cursor");
    const char *nano_scrolldown_msg =
	N_("Scroll down one line without scrolling the cursor");
#endif
#ifdef ENABLE_MULTIBUFFER
    const char *nano_prevfile_msg =
	N_("Switch to the previous file buffer");
    const char *nano_nextfile_msg =
	N_("Switch to the next file buffer");
#endif
    const char *nano_verbatim_msg =
	N_("Insert the next keystroke verbatim");
    const char *nano_tab_msg =
	N_("Insert a tab at the cursor position");
    const char *nano_enter_msg =
	N_("Insert a newline at the cursor position");
    const char *nano_delete_msg =
	N_("Delete the character under the cursor");
    const char *nano_backspace_msg =
	N_("Delete the character to the left of the cursor");
#ifndef NANO_TINY
    const char *nano_cut_till_end_msg =
	N_("Cut from the cursor position to the end of the file");
#endif
#ifndef DISABLE_JUSTIFY
    const char *nano_fulljustify_msg = N_("Justify the entire file");
#endif
#ifndef NANO_TINY
    const char *nano_wordcount_msg =
	N_("Count the number of words, lines, and characters");
#endif
    const char *nano_refresh_msg =
	N_("Refresh (redraw) the current screen");
    const char *nano_suspend_msg =
<<<<<<< HEAD
	N_("Suspend the editor (if suspend is enabled)");
#ifndef NANO_TINY
=======
	N_("Suspend the editor (if suspension is enabled)");
#ifdef ENABLE_WORDCOMPLETION
    const char *nano_completion_msg = N_("Try and complete the current word");
#endif
#ifdef ENABLE_COMMENT
    const char *nano_comment_msg =
	N_("Comment/uncomment the current line or marked lines");
#endif
#ifndef NANO_TINY
    const char *nano_savefile_msg = N_("Save file without prompting");
    const char *nano_findprev_msg = N_("Search next occurrence backward");
    const char *nano_findnext_msg = N_("Search next occurrence forward");
#endif
>>>>>>> origin/tomato-shibby-RT-AC
    const char *nano_case_msg =
	N_("Toggle the case sensitivity of the search");
    const char *nano_reverse_msg =
	N_("Reverse the direction of the search");
    const char *nano_regexp_msg =
	N_("Toggle the use of regular expressions");
<<<<<<< HEAD
#endif
#ifndef NANO_TINY
=======
#ifndef DISABLE_HISTORIES
>>>>>>> origin/tomato-shibby-RT-AC
    const char *nano_prev_history_msg =
	N_("Recall the previous search/replace string");
    const char *nano_next_history_msg =
	N_("Recall the next search/replace string");
#endif
#ifndef DISABLE_BROWSER
    const char *nano_tofiles_msg = N_("Go to file browser");
#endif
#ifndef NANO_TINY
    const char *nano_dos_msg = N_("Toggle the use of DOS format");
    const char *nano_mac_msg = N_("Toggle the use of Mac format");
#endif
    const char *nano_append_msg = N_("Toggle appending");
    const char *nano_prepend_msg = N_("Toggle prepending");
#ifndef NANO_TINY
    const char *nano_backup_msg =
	N_("Toggle backing up of the original file");
    const char *nano_execute_msg = N_("Execute external command");
#endif
#if !defined(NANO_TINY) && defined(ENABLE_MULTIBUFFER)
    const char *nano_multibuffer_msg =
	N_("Toggle the use of a new buffer");
#endif
#ifndef DISABLE_BROWSER
    const char *nano_exitbrowser_msg = N_("Exit from the file browser");
    const char *nano_firstfile_msg =
	N_("Go to the first file in the list");
    const char *nano_lastfile_msg =
	N_("Go to the last file in the list");
    const char *nano_forwardfile_msg = N_("Go to the next file in the list");
    const char *nano_backfile_msg = N_("Go to the previous file in the list");
    const char *nano_gotodir_msg = N_("Go to directory");
#endif
#endif /* !DISABLE_HELP */

#ifndef DISABLE_HELP
#define IFSCHELP(help) help
#else
#define IFSCHELP(help) ""
#endif

    while (allfuncs != NULL) {
        subnfunc *f = allfuncs;
        allfuncs = (allfuncs)->next;
        free(f);
    }

    add_to_funcs(DO_HELP_VOID,
	(MMAIN|MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MWRITEFILE|MINSERTFILE|MEXTCMD|MSPELL|MBROWSER|MWHEREISFILE|MGOTODIR),
	get_help_msg, IFSCHELP(nano_help_msg), FALSE, VIEW);

    add_to_funcs( CANCEL_MSG,
	(MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MWRITEFILE|MINSERTFILE|MEXTCMD|MSPELL|MWHEREISFILE|MGOTODIR|MYESNO),
	cancel_msg, IFSCHELP(nano_cancel_msg), FALSE, VIEW);

    add_to_funcs(DO_EXIT, MMAIN,
#ifdef ENABLE_MULTIBUFFER
	/* TRANSLATORS: Try to keep this at most 10 characters. */
	openfile != NULL && openfile != openfile->next ? N_("Close") :
#endif
	exit_msg, IFSCHELP(nano_exit_msg), FALSE, VIEW);

#ifndef DISABLE_BROWSER
    add_to_funcs(DO_EXIT, MBROWSER, exit_msg, IFSCHELP(nano_exitbrowser_msg), FALSE, VIEW);
#endif

    /* TRANSLATORS: Try to keep this at most 10 characters. */
    add_to_funcs(DO_WRITEOUT_VOID, MMAIN, N_("WriteOut"),
	IFSCHELP(nano_writeout_msg), FALSE, NOVIEW);

#ifndef DISABLE_JUSTIFY
    /* TRANSLATORS: Try to keep this at most 10 characters. */
    add_to_funcs(DO_JUSTIFY_VOID, MMAIN, N_("Justify"),
	nano_justify_msg, TRUE, NOVIEW);
#endif

    /* We allow inserting files in view mode if multibuffers are
     * available, so that we can view multiple files.  If we're using
     * restricted mode, inserting files is disabled, since it allows
     * reading from or writing to files not specified on the command
     * line. */

    add_to_funcs(DO_INSERTFILE_VOID,
	/* TRANSLATORS: Try to keep this at most 10 characters. */
	MMAIN, N_("Read File"), IFSCHELP(nano_insert_msg), FALSE,
#ifdef ENABLE_MULTIBUFFER
	VIEW);
#else
	NOVIEW);
#endif

    add_to_funcs(DO_SEARCH, MMAIN|MBROWSER, whereis_msg,
	IFSCHELP(nano_whereis_msg), FALSE, VIEW);

    add_to_funcs(DO_PAGE_UP, MMAIN|MHELP|MBROWSER,
	prev_page_msg, IFSCHELP(nano_prevpage_msg), FALSE, VIEW);
    add_to_funcs(DO_PAGE_DOWN, MMAIN|MHELP|MBROWSER,
	next_page_msg, IFSCHELP(nano_nextpage_msg), TRUE, VIEW);


    /* TRANSLATORS: Try to keep this at most 10 characters. */
    add_to_funcs(DO_CUT_TEXT_VOID, MMAIN, N_("Cut Text"), IFSCHELP(nano_cut_msg),
	FALSE, NOVIEW);

    if (unjustify)
	/* TRANSLATORS: Try to keep this at most 10 characters. */
	add_to_funcs(DO_UNCUT_TEXT, MMAIN, N_("UnJustify"), "",
	    FALSE, NOVIEW);

    else
	/* TRANSLATORS: Try to keep this at most 10 characters. */
	add_to_funcs(DO_UNCUT_TEXT, MMAIN, N_("UnCut Text"), IFSCHELP(nano_uncut_msg),
	    FALSE, NOVIEW);

#ifndef NANO_TINY
    /* TRANSLATORS: Try to keep this at most 10 characters. */
    add_to_funcs(DO_CURSORPOS_VOID, MMAIN, N_("Cur Pos"), IFSCHELP(nano_cursorpos_msg),
	FALSE, VIEW);
#endif

    /* If we're using restricted mode, spell checking is disabled
     * because it allows reading from or writing to files not specified
     * on the command line. */
#ifndef DISABLE_SPELLER
	/* TRANSLATORS: Try to keep this at most 10 characters. */
	add_to_funcs(DO_SPELL, MMAIN, N_("To Spell"), IFSCHELP(nano_spell_msg),
	    TRUE, NOVIEW);
#endif

    add_to_funcs(DO_FIRST_LINE,
	(MMAIN|MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE),
	first_line_msg, IFSCHELP(nano_firstline_msg), FALSE, VIEW);

    add_to_funcs(DO_LAST_LINE,
	(MMAIN|MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE),
	last_line_msg, IFSCHELP(nano_lastline_msg), TRUE, VIEW);


    add_to_funcs(DO_GOTOLINECOLUMN_VOID, (MMAIN|MWHEREIS),
	go_to_line_msg, IFSCHELP(nano_gotoline_msg), FALSE, VIEW);

<<<<<<< HEAD
#ifdef NANO_TINY
    /* TRANSLATORS: Try to keep this at most 10 characters. */
    add_to_funcs(DO_CURSORPOS_VOID, MMAIN, N_("Cur Pos"), IFSCHELP(nano_cursorpos_msg),
	FALSE, VIEW);
#endif


    add_to_funcs(DO_REPLACE, (MMAIN|MWHEREIS), replace_msg, IFSCHELP(nano_replace_msg),

#ifndef NANO_TINY
	FALSE,
#else
	TRUE,
#endif
	NOVIEW);
=======
    add_to_funcs(do_cursorpos_void, MMAIN,
	N_("Cur Pos"), IFSCHELP(nano_cursorpos_msg), TOGETHER, VIEW);

#if (!defined(DISABLE_JUSTIFY) && (!defined(DISABLE_SPELLER) || !defined(DISABLE_COLOR)) || \
	defined(DISABLE_JUSTIFY) && defined(DISABLE_SPELLER) && defined(DISABLE_COLOR))
    /* Conditionally placing this one here or further on, to keep the
     * help items nicely paired in most conditions. */
    add_to_funcs(do_gotolinecolumn_void, MMAIN,
	gotoline_tag, IFSCHELP(nano_gotoline_msg), BLANKAFTER, VIEW);
#endif

#ifndef NANO_TINY
    add_to_funcs(do_undo, MMAIN,
	N_("Undo"), IFSCHELP(nano_undo_msg), TOGETHER, NOVIEW);
    add_to_funcs(do_redo, MMAIN,
	N_("Redo"), IFSCHELP(nano_redo_msg), BLANKAFTER, NOVIEW);

    add_to_funcs(do_mark, MMAIN,
	N_("Mark Text"), IFSCHELP(nano_mark_msg), TOGETHER, VIEW);
    add_to_funcs(do_copy_text, MMAIN,
	N_("Copy Text"), IFSCHELP(nano_copy_msg), BLANKAFTER, NOVIEW);
#endif

    add_to_funcs(case_sens_void, MWHEREIS|MREPLACE,
	N_("Case Sens"), IFSCHELP(nano_case_msg), TOGETHER, VIEW);
    add_to_funcs(regexp_void, MWHEREIS|MREPLACE,
	N_("Regexp"), IFSCHELP(nano_regexp_msg), TOGETHER, VIEW);
    add_to_funcs(backwards_void, MWHEREIS|MREPLACE,
	N_("Backwards"), IFSCHELP(nano_reverse_msg), TOGETHER, VIEW);
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef NANO_TINY

<<<<<<< HEAD
    add_to_funcs(DO_MARK, MMAIN, N_("Mark Text"), 
	IFSCHELP(nano_mark_msg), FALSE, VIEW);

    add_to_funcs(DO_RESEARCH, (MMAIN|MBROWSER), whereis_next_msg,
	IFSCHELP(nano_whereis_next_msg), TRUE, VIEW);

    add_to_funcs(DO_COPY_TEXT, MMAIN, N_("Copy Text"),
	IFSCHELP(nano_copy_msg), FALSE, NOVIEW);

    add_to_funcs(DO_INDENT_VOID, MMAIN, N_("Indent Text"),
	IFSCHELP(nano_indent_msg), FALSE, NOVIEW);

    add_to_funcs(DO_UNINDENT, MMAIN, N_("Unindent Text"),
	IFSCHELP(nano_unindent_msg), FALSE, NOVIEW);

    if (ISSET(UNDOABLE)) {
	add_to_funcs(DO_UNDO, MMAIN, N_("Undo"),
	    IFSCHELP(nano_undo_msg), FALSE, NOVIEW);

	add_to_funcs(DO_REDO, MMAIN, N_("Redo"),
	    IFSCHELP(nano_redo_msg), TRUE, NOVIEW);
    }

#endif

    add_to_funcs(DO_RIGHT, MMAIN, N_("Forward"), IFSCHELP(nano_forward_msg),
	FALSE, VIEW);

#ifndef DISABLE_BROWSER
    add_to_funcs(DO_RIGHT, MBROWSER, N_("Forward"), IFSCHELP(nano_forwardfile_msg),
	FALSE, VIEW);
#endif

    add_to_funcs(DO_RIGHT, MALL, "", "", FALSE, VIEW);

    add_to_funcs(DO_LEFT, MMAIN, N_("Back"), IFSCHELP(nano_back_msg),
	FALSE, VIEW);

=======
#ifndef DISABLE_JUSTIFY
    add_to_funcs(do_full_justify, MWHEREIS,
	fulljustify_tag, IFSCHELP(nano_fulljustify_msg), TOGETHER, NOVIEW);

    add_to_funcs(do_gotolinecolumn_void, MWHEREIS,
	gotoline_tag, IFSCHELP(nano_gotoline_msg), BLANKAFTER, VIEW);
#endif

#ifndef NANO_TINY
    add_to_funcs(do_find_bracket, MMAIN,
	N_("To Bracket"), IFSCHELP(nano_bracket_msg), BLANKAFTER, VIEW);

    add_to_funcs(do_research, MMAIN,
	whereis_next_tag, IFSCHELP(nano_whereis_next_msg), TOGETHER, VIEW);

    add_to_funcs(do_findprevious, MMAIN,
	N_("Previous"), IFSCHELP(nano_findprev_msg), TOGETHER, VIEW);
    add_to_funcs(do_findnext, MMAIN,
	N_("Next"), IFSCHELP(nano_findnext_msg), BLANKAFTER, VIEW);
#endif /* !NANO_TINY */

    add_to_funcs(do_left, MMAIN,
	N_("Back"), IFSCHELP(nano_back_msg), TOGETHER, VIEW);
    add_to_funcs(do_right, MMAIN,
	N_("Forward"), IFSCHELP(nano_forward_msg), TOGETHER, VIEW);
>>>>>>> origin/tomato-shibby-RT-AC
#ifndef DISABLE_BROWSER
    add_to_funcs(DO_LEFT, MBROWSER, N_("Back"), IFSCHELP(nano_backfile_msg),
	FALSE, VIEW);
#endif

<<<<<<< HEAD
    add_to_funcs(DO_LEFT, MALL, "", "", FALSE, VIEW);

#ifndef NANO_TINY
    add_to_funcs(DO_NEXT_WORD_VOID, MMAIN, N_("Next Word"),
	IFSCHELP(nano_nextword_msg), FALSE, VIEW);

    add_to_funcs(DO_PREV_WORD_VOID, MMAIN, N_("Prev Word"),
	IFSCHELP(nano_prevword_msg), FALSE, VIEW);
#endif

    add_to_funcs(DO_UP_VOID, (MMAIN|MHELP|MBROWSER), N_("Prev Line"),
	IFSCHELP(nano_prevline_msg), FALSE, VIEW);

    add_to_funcs(DO_DOWN_VOID, (MMAIN|MHELP|MBROWSER), N_("Next Line"),
	IFSCHELP(nano_nextline_msg), TRUE, VIEW);

    add_to_funcs(DO_HOME, MMAIN, N_("Home"), IFSCHELP(nano_home_msg),
	FALSE, VIEW);

    add_to_funcs(DO_END, MMAIN, N_("End"), IFSCHELP(nano_end_msg),
	FALSE, VIEW);
=======
    add_to_funcs(do_prev_word_void, MMAIN,
	N_("Prev Word"), IFSCHELP(nano_prevword_msg), TOGETHER, VIEW);
    add_to_funcs(do_next_word_void, MMAIN,
	N_("Next Word"), IFSCHELP(nano_nextword_msg), TOGETHER, VIEW);

    add_to_funcs(do_home_void, MMAIN,
	N_("Home"), IFSCHELP(nano_home_msg), TOGETHER, VIEW);
    add_to_funcs(do_end_void, MMAIN,
	N_("End"), IFSCHELP(nano_end_msg), BLANKAFTER, VIEW);

    add_to_funcs(do_up_void, MMAIN|MBROWSER,
	prev_line_tag, IFSCHELP(nano_prevline_msg), TOGETHER, VIEW);
    add_to_funcs(do_down_void, MMAIN|MBROWSER,
	next_line_tag, IFSCHELP(nano_nextline_msg), TOGETHER, VIEW);
#ifndef NANO_TINY
    add_to_funcs(do_scroll_up, MMAIN,
	N_("Scroll Up"), IFSCHELP(nano_scrollup_msg), TOGETHER, VIEW);
    add_to_funcs(do_scroll_down, MMAIN,
	N_("Scroll Down"), IFSCHELP(nano_scrolldown_msg), BLANKAFTER, VIEW);
#endif

    add_to_funcs(do_prev_block, MMAIN,
	N_("Prev Block"), IFSCHELP(nano_prevblock_msg), TOGETHER, VIEW);
    add_to_funcs(do_next_block, MMAIN,
	N_("Next Block"), IFSCHELP(nano_nextblock_msg), TOGETHER, VIEW);
#ifndef DISABLE_JUSTIFY
    add_to_funcs(do_para_begin_void, MMAIN|MWHEREIS,
	N_("Beg of Par"), IFSCHELP(nano_parabegin_msg), TOGETHER, VIEW);
    add_to_funcs(do_para_end_void, MMAIN|MWHEREIS,
	N_("End of Par"), IFSCHELP(nano_paraend_msg), BLANKAFTER, VIEW);
#endif

    add_to_funcs(do_page_up, MMAIN|MHELP,
	prev_page_tag, IFSCHELP(nano_prevpage_msg), TOGETHER, VIEW);
    add_to_funcs(do_page_down, MMAIN|MHELP,
	next_page_tag, IFSCHELP(nano_nextpage_msg), TOGETHER, VIEW);

    add_to_funcs(do_first_line, MMAIN|MHELP|MWHEREIS|MREPLACE|MREPLACEWITH|MGOTOLINE,
	N_("First Line"), IFSCHELP(nano_firstline_msg), TOGETHER, VIEW);
    add_to_funcs(do_last_line, MMAIN|MHELP|MWHEREIS|MREPLACE|MREPLACEWITH|MGOTOLINE,
	N_("Last Line"), IFSCHELP(nano_lastline_msg), BLANKAFTER, VIEW);
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef DISABLE_JUSTIFY
    add_to_funcs(DO_PARA_BEGIN_VOID, (MMAIN|MWHEREIS), beg_of_par_msg,
	IFSCHELP(nano_parabegin_msg), FALSE, VIEW);

<<<<<<< HEAD
    add_to_funcs(DO_PARA_END_VOID, (MMAIN|MWHEREIS), end_of_par_msg,
	IFSCHELP(nano_paraend_msg), FALSE, VIEW);
#endif

#ifndef NANO_TINY
    add_to_funcs(DO_FIND_BRACKET, MMAIN, _("Find Other Bracket"),
	IFSCHELP(nano_bracket_msg), FALSE, VIEW);

    add_to_funcs(DO_SCROLL_UP, MMAIN, N_("Scroll Up"),
	IFSCHELP(nano_scrollup_msg), FALSE, VIEW);

    add_to_funcs(DO_SCROLL_DOWN, MMAIN, N_("Scroll Down"),
	IFSCHELP(nano_scrolldown_msg), FALSE, VIEW);
#endif

#ifdef ENABLE_MULTIBUFFER
    add_to_funcs(SWITCH_TO_PREV_BUFFER_VOID, MMAIN, _("Previous File"),
	IFSCHELP(nano_prevfile_msg), FALSE, VIEW);
    add_to_funcs(SWITCH_TO_NEXT_BUFFER_VOID, MMAIN, N_("Next File"),
	IFSCHELP(nano_nextfile_msg), TRUE, VIEW);
#endif

    add_to_funcs(DO_VERBATIM_INPUT, MMAIN, N_("Verbatim Input"),
	IFSCHELP(nano_verbatim_msg), FALSE, NOVIEW);
    add_to_funcs(DO_VERBATIM_INPUT, MWHEREIS|MREPLACE|MREPLACE2|MEXTCMD|MSPELL,
	"", "", FALSE, NOVIEW);

    add_to_funcs(DO_TAB, MMAIN, N_("Tab"), IFSCHELP(nano_tab_msg),
	FALSE, NOVIEW);
    add_to_funcs(DO_TAB, MALL, "", "", FALSE, NOVIEW);
    add_to_funcs(DO_ENTER, MMAIN, N_("Enter"), IFSCHELP(nano_enter_msg),
	FALSE, NOVIEW);
    add_to_funcs(DO_ENTER, MALL, "", "", FALSE, NOVIEW);
    add_to_funcs(DO_DELETE, MMAIN, N_("Delete"), IFSCHELP(nano_delete_msg),
	FALSE, NOVIEW);
    add_to_funcs(DO_DELETE, MALL, "", "", FALSE, NOVIEW);
    add_to_funcs(DO_BACKSPACE, MMAIN, N_("Backspace"), IFSCHELP(nano_backspace_msg),
#ifndef NANO_TINY
	FALSE,
#else
	TRUE,
#endif
	NOVIEW);
=======
#if (defined(DISABLE_JUSTIFY) && (!defined(DISABLE_SPELLER) || !defined(DISABLE_COLOR)) || \
	!defined(DISABLE_JUSTIFY) && defined(DISABLE_SPELLER) && defined(DISABLE_COLOR))
    add_to_funcs(do_gotolinecolumn_void, MMAIN,
	gotoline_tag, IFSCHELP(nano_gotoline_msg), BLANKAFTER, VIEW);
#endif

#ifdef NANO_TINY
    /* Place this one here only in the tiny version; otherwise further up. */
    add_to_funcs(do_research, MMAIN,
	whereis_next_tag, IFSCHELP(nano_whereis_next_msg), TOGETHER, VIEW);
#endif

    add_to_funcs(do_verbatim_input, MMAIN,
	N_("Verbatim"), IFSCHELP(nano_verbatim_msg), TOGETHER, NOVIEW);

    add_to_funcs(do_tab, MMAIN,
	N_("Tab"), IFSCHELP(nano_tab_msg), TOGETHER, NOVIEW);
    add_to_funcs(do_enter, MMAIN,
	N_("Enter"), IFSCHELP(nano_enter_msg), BLANKAFTER, NOVIEW);
>>>>>>> origin/tomato-shibby-RT-AC

    add_to_funcs(DO_BACKSPACE, MALL, "", "",
#ifndef NANO_TINY
	FALSE,
#else
	TRUE,
#endif
	NOVIEW);

#ifndef NANO_TINY
    add_to_funcs(DO_CUT_TILL_END, MMAIN, N_("CutTillEnd"),
	IFSCHELP(nano_cut_till_end_msg), TRUE, NOVIEW);
#endif

    add_to_funcs(XON_COMPLAINT, MMAIN, "", "", FALSE, VIEW);
    add_to_funcs(XOFF_COMPLAINT, MMAIN, "", "", FALSE, VIEW);

#ifndef DISABLE_JUSTIFY
    add_to_funcs(DO_FULL_JUSTIFY, (MMAIN|MWHEREIS), fulljstify_msg,
	IFSCHELP(nano_fulljustify_msg), FALSE, NOVIEW);
#endif

#ifndef NANO_TINY
    add_to_funcs(DO_WORDLINECHAR_COUNT, MMAIN, N_("Word Count"),
	IFSCHELP(nano_wordcount_msg), FALSE, VIEW);
#endif

    add_to_funcs(TOTAL_REFRESH, (MMAIN|MHELP), refresh_msg, 
	IFSCHELP(nano_refresh_msg), FALSE, VIEW);

    add_to_funcs(DO_SUSPEND_VOID, MMAIN, suspend_msg,
	IFSCHELP(nano_suspend_msg), TRUE, VIEW);

<<<<<<< HEAD
#ifndef NANO_TINY
    add_to_funcs( CASE_SENS_MSG,
	(MWHEREIS|MREPLACE|MWHEREISFILE),
	case_sens_msg, IFSCHELP(nano_case_msg), FALSE, VIEW);

    add_to_funcs( BACKWARDS_MSG,
	(MWHEREIS|MREPLACE|MWHEREISFILE),
	backwards_msg, IFSCHELP(nano_reverse_msg), FALSE, VIEW);
=======
#ifndef NANO_TINY
    add_to_funcs(do_indent_void, MMAIN,
	N_("Indent Text"), IFSCHELP(nano_indent_msg), TOGETHER, NOVIEW);
    add_to_funcs(do_unindent, MMAIN,
	N_("Unindent Text"), IFSCHELP(nano_unindent_msg), BLANKAFTER, NOVIEW);
#endif
#ifdef ENABLE_WORDCOMPLETION
    add_to_funcs(complete_a_word, MMAIN,
	N_("Complete"), IFSCHELP(nano_completion_msg), TOGETHER, NOVIEW);
#endif
#ifdef ENABLE_COMMENT
    add_to_funcs(do_comment, MMAIN,
	N_("Comment Lines"), IFSCHELP(nano_comment_msg), BLANKAFTER, NOVIEW);
#endif
#ifndef NANO_TINY
    add_to_funcs(do_savefile, MMAIN,
	N_("Save"), IFSCHELP(nano_savefile_msg), BLANKAFTER, NOVIEW);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

#ifdef HAVE_REGEX_H
    add_to_funcs( REGEXP_MSG,
	(MWHEREIS|MREPLACE|MWHEREISFILE),
	regexp_msg, IFSCHELP(nano_regexp_msg), FALSE, VIEW);
#endif

<<<<<<< HEAD
#ifndef NANO_TINY
    add_to_funcs( PREV_HISTORY_MSG,
	(MWHEREIS|MREPLACE|MREPLACE2|MWHEREISFILE),
	prev_history_msg, IFSCHELP(nano_prev_history_msg), FALSE, VIEW);

    add_to_funcs( NEXT_HISTORY_MSG,
	(MWHEREIS|MREPLACE|MREPLACE2|MWHEREISFILE),
	next_history_msg, IFSCHELP(nano_next_history_msg), FALSE, VIEW);
=======
#ifdef DISABLE_JUSTIFY
    add_to_funcs(do_gotolinecolumn_void, MWHEREIS,
	gotoline_tag, IFSCHELP(nano_gotoline_msg), BLANKAFTER, VIEW);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    add_to_funcs( NO_REPLACE_MSG, MREPLACE,
	no_replace_msg, IFSCHELP(nano_whereis_msg), FALSE, VIEW);

    add_to_funcs( GOTOTEXT_MSG, MGOTOLINE,
	gototext_msg, IFSCHELP(nano_whereis_msg), TRUE, VIEW);

#ifndef DISABLE_BROWSER
    if (!ISSET(RESTRICTED))
	add_to_funcs( TO_FILES_MSG,
	    (MGOTOLINE|MINSERTFILE),
	    to_files_msg, IFSCHELP(nano_tofiles_msg), FALSE, VIEW);
#endif

#ifndef NANO_TINY
    /* If we're using restricted mode, the DOS format, Mac format,
     * append, prepend, and backup toggles are disabled.  The first and
     * second are useless since inserting files is disabled, the third
     * and fourth are disabled because they allow writing to files not
     * specified on the command line, and the fifth is useless since
     * backups are disabled. */
    if (!ISSET(RESTRICTED))
        add_to_funcs( DOS_FORMAT_MSG, MWRITEFILE,
            dos_format_msg, IFSCHELP(nano_dos_msg), FALSE, NOVIEW);

    if (!ISSET(RESTRICTED))
        add_to_funcs( MAC_FORMAT_MSG, MWRITEFILE,
            mac_format_msg, IFSCHELP(nano_mac_msg), FALSE, NOVIEW);

    if (!ISSET(RESTRICTED))
        add_to_funcs( APPEND_MSG, MWRITEFILE,
            append_msg, IFSCHELP(nano_append_msg), FALSE, NOVIEW);

    if (!ISSET(RESTRICTED))
        add_to_funcs( PREPEND_MSG, MWRITEFILE,
            prepend_msg, IFSCHELP(nano_prepend_msg), FALSE, NOVIEW);

    if (!ISSET(RESTRICTED))
        add_to_funcs( BACKUP_FILE_MSG, MWRITEFILE,
            backup_file_msg, IFSCHELP(nano_backup_msg), FALSE, NOVIEW);
#endif

#ifndef NANO_TINY
    /* If we're using restricted mode, command execution is disabled.
     * It's useless since inserting files is disabled. */
    if (!ISSET(RESTRICTED))
        add_to_funcs( EXT_CMD_MSG, MINSERTFILE,
	    ext_cmd_msg, IFSCHELP(nano_execute_msg), FALSE, NOVIEW);

#ifdef ENABLE_MULTIBUFFER
    /* If we're using restricted mode, the multibuffer toggle is
     * disabled.  It's useless since inserting files is disabled. */
    if (!ISSET(RESTRICTED))
	add_to_funcs( NEW_BUFFER_MSG, MINSERTFILE,
	new_buffer_msg, IFSCHELP(nano_multibuffer_msg), FALSE, NOVIEW);
#endif

    add_to_funcs(  INSERT_FILE_MSG, MEXTCMD,
	insert_file_msg, IFSCHELP(nano_insert_msg), FALSE, VIEW);

#ifdef ENABLE_MULTIBUFFER
     add_to_funcs( NEW_BUFFER_MSG, MEXTCMD,
	new_buffer_msg, IFSCHELP(nano_multibuffer_msg), FALSE, NOVIEW);
#endif
#endif

#ifndef DISABLE_HELP
    add_to_funcs( REFRESH_MSG, MHELP,
	refresh_msg, nano_refresh_msg, FALSE, VIEW);

    add_to_funcs(DO_EXIT, MHELP, exit_msg, IFSCHELP(nano_exit_msg), FALSE, VIEW);


<<<<<<< HEAD
#endif

#ifndef DISABLE_BROWSER

    add_to_funcs( FIRST_FILE_MSG,
	(MBROWSER|MWHEREISFILE),
	first_file_msg, IFSCHELP(nano_firstfile_msg), FALSE, VIEW);

    add_to_funcs( LAST_FILE_MSG,
	(MBROWSER|MWHEREISFILE),
	last_file_msg, IFSCHELP(nano_lastfile_msg), FALSE, VIEW);

    add_to_funcs( GOTO_DIR_MSG, MBROWSER,
	goto_dir_msg, IFSCHELP(nano_gotodir_msg), FALSE, VIEW);
#endif

    currmenu = MMAIN;

    add_to_sclist(MMAIN|MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MWRITEFILE|MINSERTFILE|MEXTCMD|MSPELL|MBROWSER|MWHEREISFILE|MGOTODIR,
	"^G", DO_HELP_VOID, 0, TRUE);
    add_to_sclist(MMAIN|MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MWRITEFILE|MINSERTFILE|MEXTCMD|MSPELL|MBROWSER|MWHEREISFILE|MGOTODIR,
	"F1", DO_HELP_VOID, 0, TRUE);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "^X", DO_EXIT, 0, TRUE);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "F2", DO_EXIT, 0, TRUE);
    add_to_sclist(MMAIN, "^_", DO_GOTOLINECOLUMN_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "F13", DO_GOTOLINECOLUMN_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "M-G", DO_GOTOLINECOLUMN_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "^O", DO_WRITEOUT_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "F3", DO_WRITEOUT_VOID, 0, TRUE);
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MMAIN, "^J", DO_JUSTIFY_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "F4", DO_JUSTIFY_VOID, 0, TRUE);
#endif
    add_to_sclist(MMAIN, "^R", DO_INSERTFILE_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "F5", DO_INSERTFILE_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "kinsert", DO_INSERTFILE_VOID, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER, "^W", DO_SEARCH, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER, "F6", DO_SEARCH, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER|MHELP|MWHEREISFILE, "^Y", DO_PAGE_UP, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER|MHELP|MWHEREISFILE, "F7", DO_PAGE_UP, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER|MHELP|MWHEREISFILE, "kpup", DO_PAGE_UP, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER|MHELP|MWHEREISFILE, "^V", DO_PAGE_DOWN, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER|MHELP|MWHEREISFILE, "F8", DO_PAGE_DOWN, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER|MHELP|MWHEREISFILE, "kpdown", DO_PAGE_DOWN, 0, TRUE);
    add_to_sclist(MMAIN, "^K", DO_CUT_TEXT_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "F9", DO_CUT_TEXT_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "^U", DO_UNCUT_TEXT, 0, TRUE);
    add_to_sclist(MMAIN, "F10", DO_UNCUT_TEXT, 0, TRUE);
    add_to_sclist(MMAIN, "^C", DO_CURSORPOS_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "F11", DO_CURSORPOS_VOID, 0, TRUE);
#ifndef DISABLE_SPELLER
    add_to_sclist(MMAIN, "^T", DO_SPELL, 0, TRUE);
    add_to_sclist(MMAIN, "F12", DO_SPELL, 0, TRUE);
#endif
    add_to_sclist(MMAIN, "^\\", DO_REPLACE, 0, TRUE);
    add_to_sclist(MMAIN, "F14", DO_REPLACE, 0, TRUE);
    add_to_sclist(MMAIN, "M-R", DO_REPLACE, 0, TRUE);
    add_to_sclist(MWHEREIS, "^R", DO_REPLACE, 0, FALSE);
    add_to_sclist(MREPLACE, "^R", NO_REPLACE_MSG, 0, FALSE);
    add_to_sclist(MWHEREIS, "^T", DO_GOTOLINECOLUMN_VOID, 0, FALSE);
#ifndef NANO_TINY
    add_to_sclist(MMAIN, "^^", DO_MARK, 0, TRUE);
    add_to_sclist(MMAIN, "F15", DO_MARK, 0, TRUE);
    add_to_sclist(MMAIN, "M-A", DO_MARK, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER, "M-W", DO_RESEARCH, 0, TRUE);
    add_to_sclist(MMAIN|MBROWSER, "F16", DO_RESEARCH, 0, TRUE);
    add_to_sclist(MMAIN, "M-^", DO_COPY_TEXT, 0, TRUE);
    add_to_sclist(MMAIN, "M-6", DO_COPY_TEXT, 0, TRUE);
    add_to_sclist(MMAIN, "M-}", DO_INDENT_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "M-{", DO_UNINDENT, 0, TRUE);
    if (ISSET(UNDOABLE)) {
	add_to_sclist(MMAIN, "M-U", DO_UNDO, 0, TRUE);
 	add_to_sclist(MMAIN, "M-E", DO_REDO, 0, TRUE);
    }
    add_to_sclist(MALL, "^F", DO_RIGHT, 0, TRUE);
    add_to_sclist(MALL, "^B", DO_LEFT, 0, TRUE);
    add_to_sclist(MMAIN, "^Space", DO_NEXT_WORD_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "M-Space", DO_PREV_WORD_VOID, 0, TRUE);
#endif
    add_to_sclist(MALL, "kright", DO_RIGHT, 0, TRUE);
    add_to_sclist(MALL, "kleft", DO_LEFT, 0, TRUE);
    add_to_sclist(MMAIN, "^Q", XON_COMPLAINT, 0, TRUE);
    add_to_sclist(MMAIN, "^S", XOFF_COMPLAINT, 0, TRUE);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "^P", DO_UP_VOID, 0, TRUE);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "kup", DO_UP_VOID, 0, TRUE);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "^N", DO_DOWN_VOID, 0, TRUE);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "kdown", DO_DOWN_VOID, 0, TRUE);
    add_to_sclist(MALL, "^A", DO_HOME, 0, TRUE);
    add_to_sclist(MALL, "khome", DO_HOME, 0, TRUE);
    add_to_sclist(MALL, "^E", DO_END, 0, TRUE);
    add_to_sclist(MALL, "kend", DO_END, 0, TRUE);
#ifndef NANO_TINY
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2|MWHEREISFILE, "^P",  PREV_HISTORY_MSG, 0, FALSE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2|MWHEREISFILE, "kup",  PREV_HISTORY_MSG, 0, FALSE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2|MWHEREISFILE, "^N",  NEXT_HISTORY_MSG, 0, FALSE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2|MWHEREISFILE, "kdown",  NEXT_HISTORY_MSG, 0, FALSE);
#endif
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2,
	"^W", DO_PARA_BEGIN_VOID, 0, TRUE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2,
	"^O", DO_PARA_END_VOID, 0, TRUE);
    add_to_sclist(MALL, "M-(", DO_PARA_BEGIN_VOID, 0, TRUE);
    add_to_sclist(MALL, "M-9", DO_PARA_BEGIN_VOID, 0, TRUE);
    add_to_sclist(MALL, "M-)", DO_PARA_END_VOID, 0, TRUE);
    add_to_sclist(MALL, "M-0", DO_PARA_END_VOID, 0, TRUE);
#endif
    add_to_sclist(MWHEREIS,
	"M-C", CASE_SENS_MSG, 0, FALSE);
    add_to_sclist(MREPLACE,
	"M-C", CASE_SENS_MSG, 0, FALSE);
    add_to_sclist(MREPLACE2,
	"M-C", CASE_SENS_MSG, 0, FALSE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2,
	"M-B", BACKWARDS_MSG, 0, FALSE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2,
	"M-R", REGEXP_MSG, 0, FALSE);

    add_to_sclist(MMAIN, "M-\\", DO_FIRST_LINE, 0, TRUE);
    add_to_sclist(MMAIN, "M-|", DO_FIRST_LINE, 0, TRUE);
    add_to_sclist(MMAIN, "M-/", DO_LAST_LINE, 0, TRUE);
    add_to_sclist(MMAIN, "M-?", DO_LAST_LINE, 0, TRUE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MHELP,
	"^Y", DO_FIRST_LINE, 0, TRUE);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MHELP,
	"^V", DO_LAST_LINE, 0, TRUE);

    add_to_sclist(MBROWSER|MWHEREISFILE, "M-\\",  FIRST_FILE_MSG, 0, TRUE);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-|",  FIRST_FILE_MSG, 0, TRUE);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-/",  LAST_FILE_MSG, 0, TRUE);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-?",  LAST_FILE_MSG, 0, TRUE);
    add_to_sclist(MBROWSER|MWHEREISFILE, "^_",  GOTO_DIR_MSG, 0, TRUE);
    add_to_sclist(MBROWSER|MWHEREISFILE, "F13",  GOTO_DIR_MSG, 0, TRUE);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-G",  GOTO_DIR_MSG, 0, TRUE);
#ifndef NANO_TINY
    add_to_sclist(MMAIN, "M-]", DO_FIND_BRACKET, 0, TRUE);
    add_to_sclist(MMAIN, "M--", DO_SCROLL_UP, 0, TRUE);
    add_to_sclist(MMAIN, "M-_", DO_SCROLL_UP, 0, TRUE);
    add_to_sclist(MMAIN, "M-+", DO_SCROLL_DOWN, 0, TRUE);
    add_to_sclist(MMAIN, "M-=", DO_SCROLL_DOWN, 0, TRUE);
#endif

#ifdef ENABLE_MULTIBUFFER
    add_to_sclist(MMAIN, "M-<", SWITCH_TO_PREV_BUFFER_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "M-,", SWITCH_TO_PREV_BUFFER_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "M->", SWITCH_TO_NEXT_BUFFER_VOID, 0, TRUE);
    add_to_sclist(MMAIN, "M-.", SWITCH_TO_NEXT_BUFFER_VOID, 0, TRUE);
#endif
    add_to_sclist(MALL, "M-V", DO_VERBATIM_INPUT, 0, TRUE);
#ifndef NANO_TINY
    add_to_sclist(MALL, "M-T", DO_CUT_TILL_END, 0, TRUE);
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MALL, "M-J", DO_FULL_JUSTIFY, 0, TRUE);
#endif
    add_to_sclist(MMAIN, "M-D", DO_WORDLINECHAR_COUNT, 0, TRUE);
    add_to_sclist(MMAIN, "M-X", DO_TOGGLE, NO_HELP, TRUE);
    add_to_sclist(MMAIN, "M-C", DO_TOGGLE, CONST_UPDATE, TRUE);
    add_to_sclist(MMAIN, "M-O", DO_TOGGLE, MORE_SPACE, TRUE);
    add_to_sclist(MMAIN, "M-S", DO_TOGGLE, SMOOTH_SCROLL, TRUE);
    add_to_sclist(MMAIN, "M-P", DO_TOGGLE, WHITESPACE_DISPLAY, TRUE);
    add_to_sclist(MMAIN, "M-Y", DO_TOGGLE, NO_COLOR_SYNTAX, TRUE);
    add_to_sclist(MMAIN, "M-H", DO_TOGGLE, SMART_HOME, TRUE);
    add_to_sclist(MMAIN, "M-I", DO_TOGGLE, AUTOINDENT, TRUE);
    add_to_sclist(MMAIN, "M-K", DO_TOGGLE, CUT_TO_END, TRUE);
    add_to_sclist(MMAIN, "M-L", DO_TOGGLE, NO_WRAP, TRUE);
    add_to_sclist(MMAIN, "M-Q", DO_TOGGLE, TABS_TO_SPACES, TRUE);
    add_to_sclist(MMAIN, "M-B", DO_TOGGLE, BACKUP_FILE, TRUE);
    add_to_sclist(MMAIN, "M-F", DO_TOGGLE, MULTIBUFFER, TRUE);
    add_to_sclist(MMAIN, "M-M", DO_TOGGLE, USE_MOUSE, TRUE);
    add_to_sclist(MMAIN, "M-N", DO_TOGGLE, NO_CONVERT, TRUE);
    add_to_sclist(MMAIN, "M-Z", DO_TOGGLE, SUSPEND, TRUE);
    add_to_sclist(MMAIN, "M-$", DO_TOGGLE, SOFTWRAP, TRUE);
#endif
    add_to_sclist(MGOTOLINE, "^T",  GOTOTEXT_MSG, 0, FALSE);
    add_to_sclist(MINSERTFILE|MEXTCMD, "M-F",  NEW_BUFFER_MSG, 0, FALSE);
    add_to_sclist((MWHEREIS|MREPLACE|MREPLACE2|MGOTOLINE|MWRITEFILE|MINSERTFILE|MEXTCMD|MSPELL|MWHEREISFILE|MGOTODIR|MYESNO),
	"^C", CANCEL_MSG, 0, FALSE);
    add_to_sclist(MHELP, "^X", DO_EXIT, 0, TRUE);
    add_to_sclist(MHELP, "F2", DO_EXIT, 0, TRUE);
    add_to_sclist(MWRITEFILE, "M-D",  DOS_FORMAT_MSG, 0, FALSE);
    add_to_sclist(MWRITEFILE, "M-M",  MAC_FORMAT_MSG, 0, FALSE);
    add_to_sclist(MWRITEFILE, "M-A",  APPEND_MSG, 0, FALSE);
    add_to_sclist(MWRITEFILE, "M-P",  PREPEND_MSG, 0, FALSE);
    add_to_sclist(MWRITEFILE, "M-B",  BACKUP_FILE_MSG, 0, FALSE);
    add_to_sclist(MWRITEFILE, "^T",  TO_FILES_MSG, 0, FALSE);
    add_to_sclist(MINSERTFILE, "^T",  TO_FILES_MSG, 0, FALSE);
    add_to_sclist(MINSERTFILE, "^X",  EXT_CMD_MSG, 0, FALSE);
    add_to_sclist(MMAIN, "^Z", DO_SUSPEND_VOID, 0, FALSE);
    add_to_sclist(MMAIN, "^L", TOTAL_REFRESH, 0, TRUE);
    add_to_sclist(MALL, "^I", DO_TAB, 0, TRUE);
    add_to_sclist(MALL, "^M", DO_ENTER, 0, TRUE);
    add_to_sclist(MALL, "kenter", DO_ENTER, 0, TRUE);
    add_to_sclist(MALL, "^D", DO_DELETE, 0, TRUE);
    add_to_sclist(MALL, "kdel", DO_DELETE, 0, TRUE);
    add_to_sclist(MALL, "^H", DO_BACKSPACE, 0, TRUE);
    add_to_sclist(MALL, "kbsp", DO_BACKSPACE, 0, TRUE);
=======
#ifndef DISABLE_BROWSER
    add_to_funcs(do_research, MBROWSER,
	whereis_next_tag, IFSCHELP(nano_whereis_next_msg), TOGETHER, VIEW);
    add_to_funcs(total_refresh, MBROWSER,
	refresh_tag, IFSCHELP(nano_browser_refresh_msg), BLANKAFTER, VIEW);
#ifndef NANO_TINY
    add_to_funcs(do_prev_word_void, MBROWSER,
	N_("Left Column"), IFSCHELP(nano_browser_lefthand_msg), TOGETHER, VIEW);
    add_to_funcs(do_next_word_void, MBROWSER,
	N_("Right Column"), IFSCHELP(nano_browser_righthand_msg), BLANKAFTER, VIEW);
#endif
#endif

#ifndef DISABLE_COLOR
    add_to_funcs(do_page_up, MLINTER,
	/* TRANSLATORS: Try to keep the next two strings at most 20 characters. */
	N_("Prev Lint Msg"), IFSCHELP(nano_prevlint_msg), TOGETHER, VIEW);
    add_to_funcs(do_page_down, MLINTER,
	N_("Next Lint Msg"), IFSCHELP(nano_nextlint_msg), TOGETHER, VIEW);
#endif

    /* Start associating key combos with functions in specific menus. */

    add_to_sclist(MMOST, "^G", 0, do_help_void, 0);
    add_to_sclist(MMOST, "F1", 0, do_help_void, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "^X", 0, do_exit, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "F2", 0, do_exit, 0);
    add_to_sclist(MMAIN, "^O", 0, do_writeout_void, 0);
    add_to_sclist(MMAIN, "F3", 0, do_writeout_void, 0);
    add_to_sclist(MMAIN, "^R", 0, do_insertfile_void, 0);
    add_to_sclist(MMAIN, "F5", 0, do_insertfile_void, 0);
    add_to_sclist(MMAIN, "Ins", 0, do_insertfile_void, 0);
    add_to_sclist(MMAIN|MBROWSER, "^W", 0, do_search, 0);
    add_to_sclist(MMAIN|MBROWSER, "F6", 0, do_search, 0);
    add_to_sclist(MMAIN, "^\\", 0, do_replace, 0);
    add_to_sclist(MMAIN, "M-R", 0, do_replace, 0);
    add_to_sclist(MMAIN, "F14", 0, do_replace, 0);
    add_to_sclist(MMOST, "^K", 0, do_cut_text_void, 0);
    add_to_sclist(MMOST, "F9", 0, do_cut_text_void, 0);
    add_to_sclist(MMAIN, "^U", 0, do_uncut_text, 0);
    add_to_sclist(MMAIN, "F10", 0, do_uncut_text, 0);
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MMAIN, "^J", 0, do_justify_void, 0);
    add_to_sclist(MMAIN, "F4", 0, do_justify_void, 0);
#endif
#ifndef DISABLE_SPELLER
    add_to_sclist(MMAIN, "^T", 0, do_spell, 0);
    add_to_sclist(MMAIN, "F12", 0, do_spell, 0);
#else
#ifndef DISABLE_COLOR
    add_to_sclist(MMAIN, "^T", 0, do_linter, 0);
    add_to_sclist(MMAIN, "F12", 0, do_linter, 0);
#endif
#endif
    add_to_sclist(MMAIN, "^C", 0, do_cursorpos_void, 0);
    add_to_sclist(MMAIN, "F11", 0, do_cursorpos_void, 0);
    add_to_sclist(MMAIN, "^_", 0, do_gotolinecolumn_void, 0);
    add_to_sclist(MMAIN, "M-G", 0, do_gotolinecolumn_void, 0);
    add_to_sclist(MMAIN, "F13", 0, do_gotolinecolumn_void, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER|MLINTER, "^Y", 0, do_page_up, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER|MLINTER, "F7", 0, do_page_up, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER|MLINTER, "PgUp", KEY_PPAGE, do_page_up, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER|MLINTER, "^V", 0, do_page_down, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER|MLINTER, "F8", 0, do_page_down, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER|MLINTER, "PgDn", KEY_NPAGE, do_page_down, 0);
    add_to_sclist(MMAIN|MHELP, "M-\\", 0, do_first_line, 0);
    add_to_sclist(MMAIN|MHELP, "M-|", 0, do_first_line, 0);
    add_to_sclist(MMAIN|MHELP, "M-/", 0, do_last_line, 0);
    add_to_sclist(MMAIN|MHELP, "M-?", 0, do_last_line, 0);
    add_to_sclist(MMAIN|MBROWSER, "M-W", 0, do_research, 0);
    add_to_sclist(MMAIN|MBROWSER, "F16", 0, do_research, 0);
#ifndef NANO_TINY
    add_to_sclist(MMAIN, "M-]", 0, do_find_bracket, 0);
    add_to_sclist(MMAIN, "^^", 0, do_mark, 0);
    add_to_sclist(MMAIN, "M-A", 0, do_mark, 0);
    add_to_sclist(MMAIN, "F15", 0, do_mark, 0);
    add_to_sclist(MMAIN, "M-^", 0, do_copy_text, 0);
    add_to_sclist(MMAIN, "M-6", 0, do_copy_text, 0);
    add_to_sclist(MMAIN, "M-}", 0, do_indent_void, 0);
    add_to_sclist(MMAIN, "M-{", 0, do_unindent, 0);
    add_to_sclist(MMAIN, "M-U", 0, do_undo, 0);
    add_to_sclist(MMAIN, "M-E", 0, do_redo, 0);
#endif
#ifdef ENABLE_WORDCOMPLETION
    add_to_sclist(MMAIN, "^]", 0, complete_a_word, 0);
#endif
#ifdef ENABLE_COMMENT
    add_to_sclist(MMAIN, "M-3", 0, do_comment, 0);
#endif
    add_to_sclist(MMOST, "^B", 0, do_left, 0);
    add_to_sclist(MMOST, "Left", KEY_LEFT, do_left, 0);
    add_to_sclist(MMOST, "^F", 0, do_right, 0);
    add_to_sclist(MMOST, "Right", KEY_RIGHT, do_right, 0);
#ifdef ENABLE_UTF8
    if (using_utf8()) {
	add_to_sclist(MMOST, "^\xE2\x86\x90", CONTROL_LEFT, do_prev_word_void, 0);
	add_to_sclist(MMOST, "^\xE2\x86\x92", CONTROL_RIGHT, do_next_word_void, 0);
    } else
#endif
    {
	add_to_sclist(MMOST, "^Left", CONTROL_LEFT, do_prev_word_void, 0);
	add_to_sclist(MMOST, "^Right", CONTROL_RIGHT, do_next_word_void, 0);
    }
    add_to_sclist(MMOST, "M-Space", 0, do_prev_word_void, 0);
    add_to_sclist(MMOST, "^Space", 0, do_next_word_void, 0);
    add_to_sclist((MMOST & ~MBROWSER), "^A", 0, do_home_void, 0);
    add_to_sclist((MMOST & ~MBROWSER), "Home", KEY_HOME, do_home_void, 0);
    add_to_sclist((MMOST & ~MBROWSER), "^E", 0, do_end_void, 0);
    add_to_sclist((MMOST & ~MBROWSER), "End", KEY_END, do_end_void, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "^P", 0, do_up_void, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "Up", KEY_UP, do_up_void, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "^N", 0, do_down_void, 0);
    add_to_sclist(MMAIN|MHELP|MBROWSER, "Down", KEY_DOWN, do_down_void, 0);
#ifdef ENABLE_UTF8
    if (using_utf8()) {
	add_to_sclist(MMAIN, "^\xE2\x86\x91", CONTROL_UP, do_prev_block, 0);
	add_to_sclist(MMAIN, "^\xE2\x86\x93", CONTROL_DOWN, do_next_block, 0);
    } else
#endif
    {
	add_to_sclist(MMAIN, "^Up", CONTROL_UP, do_prev_block, 0);
	add_to_sclist(MMAIN, "^Down", CONTROL_DOWN, do_next_block, 0);
    }
    add_to_sclist(MMAIN, "M-7", 0, do_prev_block, 0);
    add_to_sclist(MMAIN, "M-8", 0, do_next_block, 0);
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MMAIN, "M-(", 0, do_para_begin_void, 0);
    add_to_sclist(MMAIN, "M-9", 0, do_para_begin_void, 0);
    add_to_sclist(MMAIN, "M-)", 0, do_para_end_void, 0);
    add_to_sclist(MMAIN, "M-0", 0, do_para_end_void, 0);
#endif
#ifndef NANO_TINY
    add_to_sclist(MMAIN, "M--", 0, do_scroll_up, 0);
    add_to_sclist(MMAIN, "M-_", 0, do_scroll_up, 0);
    add_to_sclist(MMAIN, "M-+", 0, do_scroll_down, 0);
    add_to_sclist(MMAIN, "M-=", 0, do_scroll_down, 0);
#endif
#ifndef DISABLE_MULTIBUFFER
    add_to_sclist(MMAIN, "M-<", 0, switch_to_prev_buffer_void, 0);
    add_to_sclist(MMAIN, "M-,", 0, switch_to_prev_buffer_void, 0);
    add_to_sclist(MMAIN, "M->", 0, switch_to_next_buffer_void, 0);
    add_to_sclist(MMAIN, "M-.", 0, switch_to_next_buffer_void, 0);
#endif
    add_to_sclist(MMOST, "M-V", 0, do_verbatim_input, 0);
#ifndef NANO_TINY
    add_to_sclist(MMAIN, "M-T", 0, do_cut_till_eof, 0);
    add_to_sclist(MMAIN, "M-D", 0, do_wordlinechar_count, 0);
#endif
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MMAIN|MWHEREIS, "M-J", 0, do_full_justify, 0);
#endif
    add_to_sclist(MMAIN|MHELP, "^L", 0, total_refresh, 0);
    add_to_sclist(MMAIN, "^Z", 0, do_suspend_void, 0);

#ifndef NANO_TINY
    /* Group of "Appearance" toggles. */
    add_to_sclist(MMAIN, "M-X", 0, do_toggle_void, NO_HELP);
    add_to_sclist(MMAIN, "M-C", 0, do_toggle_void, CONST_UPDATE);
    add_to_sclist(MMAIN, "M-O", 0, do_toggle_void, MORE_SPACE);
    add_to_sclist(MMAIN, "M-S", 0, do_toggle_void, SMOOTH_SCROLL);
    add_to_sclist(MMAIN, "M-$", 0, do_toggle_void, SOFTWRAP);
#ifdef ENABLE_LINENUMBERS
    add_to_sclist(MMAIN, "M-#", 0, do_toggle_void, LINE_NUMBERS);
#endif
    add_to_sclist(MMAIN, "M-P", 0, do_toggle_void, WHITESPACE_DISPLAY);
#ifndef DISABLE_COLOR
    add_to_sclist(MMAIN, "M-Y", 0, do_toggle_void, NO_COLOR_SYNTAX);
#endif

    /* Group of "Editing-behavior" toggles. */
    add_to_sclist(MMAIN, "M-H", 0, do_toggle_void, SMART_HOME);
    add_to_sclist(MMAIN, "M-I", 0, do_toggle_void, AUTOINDENT);
    add_to_sclist(MMAIN, "M-K", 0, do_toggle_void, CUT_TO_END);
#ifndef DISABLE_WRAPPING
    add_to_sclist(MMAIN, "M-L", 0, do_toggle_void, NO_WRAP);
#endif
    add_to_sclist(MMAIN, "M-Q", 0, do_toggle_void, TABS_TO_SPACES);

    /* Group of "Peripheral-feature" toggles. */
    add_to_sclist(MMAIN, "M-B", 0, do_toggle_void, BACKUP_FILE);
#ifndef DISABLE_MULTIBUFFER
    add_to_sclist(MMAIN, "M-F", 0, do_toggle_void, MULTIBUFFER);
#endif
#ifndef DISABLE_MOUSE
    add_to_sclist(MMAIN, "M-M", 0, do_toggle_void, USE_MOUSE);
#endif
    add_to_sclist(MMAIN, "M-N", 0, do_toggle_void, NO_CONVERT);
    add_to_sclist(MMAIN, "M-Z", 0, do_toggle_void, SUSPEND);
#endif /* !NANO_TINY */

    add_to_sclist(MMAIN, "^Q", 0, xon_complaint, 0);
    add_to_sclist(MMAIN, "^S", 0, xoff_complaint, 0);

    add_to_sclist(((MMOST & ~MMAIN & ~MBROWSER) | MYESNO), "^C", 0, do_cancel, 0);

    add_to_sclist(MWHEREIS|MREPLACE, "M-C", 0, case_sens_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE, "M-R", 0, regexp_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE, "M-B", 0, backwards_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE, "^R", 0, flip_replace_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MGOTOLINE, "^Y", 0, do_first_line, 0);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MGOTOLINE, "^V", 0, do_last_line, 0);
#ifndef DISABLE_JUSTIFY
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH, "^W", 0, do_para_begin_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH, "^O", 0, do_para_end_void, 0);
#endif
    add_to_sclist(MWHEREIS, "^T", 0, do_gotolinecolumn_void, 0);
    add_to_sclist(MGOTOLINE, "^T", 0, gototext_void, 0);
#ifndef DISABLE_HISTORIES
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE, "^P", 0, get_history_older_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE, "Up", KEY_UP, get_history_older_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE, "^N", 0, get_history_newer_void, 0);
    add_to_sclist(MWHEREIS|MREPLACE|MREPLACEWITH|MWHEREISFILE, "Down", KEY_DOWN, get_history_newer_void, 0);
#endif
#ifndef DISABLE_BROWSER
    add_to_sclist(MWHEREISFILE, "^Y", 0, do_first_file, 0);
    add_to_sclist(MWHEREISFILE, "^V", 0, do_last_file, 0);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-\\", 0, do_first_file, 0);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-|", 0, do_first_file, 0);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-/", 0, do_last_file, 0);
    add_to_sclist(MBROWSER|MWHEREISFILE, "M-?", 0, do_last_file, 0);
    add_to_sclist(MBROWSER, "Home", KEY_HOME, do_first_file, 0);
    add_to_sclist(MBROWSER, "End", KEY_END, do_last_file, 0);
    add_to_sclist(MBROWSER, "^_", 0, goto_dir_void, 0);
    add_to_sclist(MBROWSER, "M-G", 0, goto_dir_void, 0);
    add_to_sclist(MBROWSER, "F13", 0, goto_dir_void, 0);
    add_to_sclist(MBROWSER, "^L", 0, total_refresh, 0);
#endif
    if (ISSET(TEMP_FILE))
	add_to_sclist(MWRITEFILE, "^Q", 0, discard_buffer, 0);
    add_to_sclist(MWRITEFILE, "M-D", 0, dos_format_void, 0);
    add_to_sclist(MWRITEFILE, "M-M", 0, mac_format_void, 0);
    if (!ISSET(RESTRICTED)) {
	/* Don't allow Appending, Prepending, nor Backups in restricted mode. */
	add_to_sclist(MWRITEFILE, "M-A", 0, append_void, 0);
	add_to_sclist(MWRITEFILE, "M-P", 0, prepend_void, 0);
	add_to_sclist(MWRITEFILE, "M-B", 0, backup_file_void, 0);
#ifndef DISABLE_BROWSER
	add_to_sclist(MWRITEFILE|MINSERTFILE, "^T", 0, to_files_void, 0);
#endif
	add_to_sclist(MINSERTFILE|MEXTCMD, "^X", 0, flip_execute_void, 0);
	add_to_sclist(MINSERTFILE|MEXTCMD, "M-F", 0, new_buffer_void, 0);
    }
    add_to_sclist(MHELP|MBROWSER, "^C", 0, do_exit, 0);
    /* Allow exiting from the file browser and the help viewer with
     * the same key as they were entered. */
#ifndef DISABLE_BROWSER
    add_to_sclist(MBROWSER, "^T", 0, do_exit, 0);
#endif
#ifndef DISABLE_HELP
    add_to_sclist(MHELP, "^G", 0, do_exit, 0);
    add_to_sclist(MHELP, "Home", KEY_HOME, do_first_line, 0);
    add_to_sclist(MHELP, "End", KEY_END, do_last_line, 0);
#endif
    add_to_sclist(MMOST, "^I", 0, do_tab, 0);
    add_to_sclist(MMOST, "Tab", TAB_CODE, do_tab, 0);
    add_to_sclist(MMOST, "^M", 0, do_enter, 0);
    add_to_sclist(MMOST, "Enter", KEY_ENTER, do_enter, 0);
    add_to_sclist(MMOST, "^D", 0, do_delete, 0);
    add_to_sclist(MMOST, "Del", 0, do_delete, 0);
    add_to_sclist(MMOST, "^H", 0, do_backspace, 0);
    add_to_sclist(MMOST, "Bsp", KEY_BACKSPACE, do_backspace, 0);
>>>>>>> origin/tomato-shibby-RT-AC

#ifdef DEBUG
    print_sclist();
#endif

}

/* Given a function alias, execute the proper
   function, and then me */
void iso_me_harder_funcmap(short func)
{
    if (func == TOTAL_REFRESH)
	total_refresh();
    else if (func == DO_HELP_VOID)
	do_help_void();
    else if (func == DO_SEARCH)
	do_search();
    else if (func == DO_PAGE_UP)
	do_page_up();
    else if (func == DO_PAGE_DOWN)
	do_page_down();
    else if (func == DO_UP_VOID)
	do_up_void();
    else if (func == DO_LEFT)
	do_left();
    else if (func == DO_DOWN_VOID)
	do_down_void();
    else if (func == DO_RIGHT)
	do_right();
    else if (func == DO_ENTER)
	do_enter(FALSE);
    else if (func == DO_EXIT)
	do_exit();
    else if (func == DO_FIRST_LINE)
	do_first_line();
    else if (func == DO_LAST_LINE)
	do_last_line();
    else if (func == DO_BACKSPACE)
	do_backspace();
    else if (func == DO_DELETE)
	do_delete();
    else if (func == DO_TAB)
	do_tab();
    else if (func == DO_VERBATIM_INPUT)
	do_verbatim_input();
#ifdef ENABLE_MULTIBUFFER
    else if (func == SWITCH_TO_NEXT_BUFFER_VOID)
	switch_to_next_buffer_void();
    else if (func == SWITCH_TO_PREV_BUFFER_VOID)
	switch_to_prev_buffer_void();
#endif
    else if (func == DO_END)
	do_end();
    else if (func == DO_HOME)
	do_home();
    else if (func == DO_SUSPEND_VOID)
	do_suspend_void();
    else if (func == DO_WRITEOUT_VOID)
	do_writeout_void();
    else if (func == DO_INSERTFILE_VOID)
	do_insertfile_void();
    else if (func == DO_CUT_TEXT_VOID)
	do_cut_text_void();
    else if (func == DO_UNCUT_TEXT)
	do_uncut_text();
    else if (func == DO_CURSORPOS_VOID)
	do_cursorpos_void();
    else if (func == DO_GOTOLINECOLUMN_VOID)
	do_gotolinecolumn_void();
    else if (func == DO_REPLACE)
	do_replace();
    else if (func == XOFF_COMPLAINT)
	xoff_complaint();
    else if (func == XON_COMPLAINT)
	xon_complaint();
    else if (func == DO_CUT_TEXT)
	do_cut_text_void();
#ifndef NANO_TINY
    else if (func == DO_CUT_TILL_END)
	do_cut_till_end();
    else if (func == DO_REDO)
	do_redo();
    else if (func == DO_UNDO)
	do_undo();
    else if (func == DO_WORDLINECHAR_COUNT)
	do_wordlinechar_count();
    else if (func == DO_FIND_BRACKET)
	do_find_bracket();
    else if (func == DO_PREV_WORD_VOID)
	do_prev_word_void();
#ifndef DISABLE_JUSTIFY
    else if (func == DO_JUSTIFY_VOID)
	do_justify_void();
    else if (func == DO_PARA_BEGIN_VOID)
	do_para_begin_void();
    else if (func == DO_PARA_END_VOID)
	do_para_end_void();
    else if (func == DO_FULL_JUSTIFY)
	do_full_justify();
#endif
    else if (func == DO_MARK)
	do_mark();
    else if (func == DO_RESEARCH)
	do_research();
    else if (func == DO_COPY_TEXT)
	do_copy_text();
    else if (func == DO_INDENT_VOID)
	do_indent_void();
    else if (func == DO_UNINDENT)
	do_unindent();
    else if (func == DO_SCROLL_UP)
	do_scroll_up();
    else if (func == DO_SCROLL_DOWN)
	do_scroll_down();
    else if (func == DO_NEXT_WORD_VOID)
	do_next_word_void();
#ifndef DISABLE_SPELLER
    else if (func == DO_SPELL)
	do_spell();
#endif
    else if (func == DO_NEXT_WORD)
	do_next_word_void();
    else if (func == DO_PREV_WORD)
	do_prev_word_void();
#endif
}


/* Free the given shortcut. */
void free_shortcutage(shortcut **shortcutage)
{
    assert(shortcutage != NULL);

    while (*shortcutage != NULL) {
	shortcut *ps = *shortcutage;
	*shortcutage = (*shortcutage)->next;
	free(ps);
    }
}

const subnfunc *sctofunc(sc *s)
{
    subnfunc *f;

    for (f = allfuncs; f != NULL && s->scfunc != f->scfunc; f = f->next)
	;

    return f;
}

#ifndef NANO_TINY
/* Now lets come up with a single (hopefully)
   function to get a string for each flag */
const char *flagtostr(int flag)
{
<<<<<<< HEAD
   switch (flag) {
        case NO_HELP:
            return N_("Help mode");
        case CONST_UPDATE:
            return N_("Constant cursor position display");
        case MORE_SPACE:
            return N_("Use of one more line for editing");
        case SMOOTH_SCROLL:
            return N_("Smooth scrolling");
        case WHITESPACE_DISPLAY:
            return N_("Whitespace display");
        case NO_COLOR_SYNTAX:
            return N_("Color syntax highlighting");
        case SMART_HOME:
            return N_("Smart home key");
        case AUTOINDENT:
            return N_("Auto indent");
        case CUT_TO_END:
            return N_("Cut to end");
        case NO_WRAP:
            return N_("Long line wrapping");
        case TABS_TO_SPACES:
            return N_("Conversion of typed tabs to spaces");
        case BACKUP_FILE:
            return N_("Backup files");
        case MULTIBUFFER:
            return N_("Multiple file buffers");
        case USE_MOUSE:
            return N_("Mouse support");
        case NO_CONVERT:
            return N_("No conversion from DOS/Mac format");
        case SUSPEND:
            return N_("Suspension");
        case SOFTWRAP:
            return N_("Soft line wrapping");
        default:
            return "?????";
=======
    switch (flag) {
	case NO_HELP:
	    /* TRANSLATORS: The next seventeen strings are toggle descriptions;
	     * they are best kept shorter than 40 characters, but may be longer. */
	    return N_("Help mode");
	case CONST_UPDATE:
	    return N_("Constant cursor position display");
	case MORE_SPACE:
	    return N_("Use of one more line for editing");
	case SMOOTH_SCROLL:
	    return N_("Smooth scrolling");
	case SOFTWRAP:
	    return N_("Soft wrapping of overlong lines");
	case WHITESPACE_DISPLAY:
	    return N_("Whitespace display");
	case NO_COLOR_SYNTAX:
	    return N_("Color syntax highlighting");
	case SMART_HOME:
	    return N_("Smart home key");
	case AUTOINDENT:
	    return N_("Auto indent");
	case CUT_TO_END:
	    return N_("Cut to end");
	case NO_WRAP:
	    return N_("Hard wrapping of overlong lines");
	case TABS_TO_SPACES:
	    return N_("Conversion of typed tabs to spaces");
	case BACKUP_FILE:
	    return N_("Backup files");
	case MULTIBUFFER:
	    return N_("Reading file into separate buffer");
	case USE_MOUSE:
	    return N_("Mouse support");
	case NO_CONVERT:
	    return N_("No conversion from DOS/Mac format");
	case SUSPEND:
	    return N_("Suspension");
	case LINE_NUMBERS:
	    return N_("Line numbering");
	default:
	    return "?????";
>>>>>>> origin/tomato-shibby-RT-AC
    }
}
#endif /* NANO_TINY */

/* Interpret the string given by the rc file and return a
    shortcut struct, complete with proper value for execute */
sc *strtosc(int menu, char *input)
{
<<<<<<< HEAD
     sc *s;

    s = (sc *)nmalloc(sizeof(sc));
    s->execute = TRUE; /* overridden as needed below */

=======
    sc *s = nmalloc(sizeof(sc));

#ifndef NANO_TINY
    s->toggle = 0;
#endif
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef DISABLE_HELP
    if (!strcasecmp(input, "help"))
	s->scfunc = DO_HELP_VOID;
    else 
#endif
    if (!strcasecmp(input, "cancel")) {
	s->scfunc =  CANCEL_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "exit"))
	s->scfunc = DO_EXIT;
    else if (!strcasecmp(input, "writeout"))
	s->scfunc = DO_WRITEOUT_VOID;
    else if (!strcasecmp(input, "insert"))
	s->scfunc = DO_INSERTFILE_VOID;
    else if (!strcasecmp(input, "whereis"))
<<<<<<< HEAD
	s->scfunc = DO_SEARCH;
    else if (!strcasecmp(input, "up"))
	s->scfunc = DO_UP_VOID;
    else if (!strcasecmp(input, "down"))
	s->scfunc = DO_DOWN_VOID;
    else if (!strcasecmp(input, "pageup")
	|| !strcasecmp(input, "prevpage"))
	s->scfunc = DO_PAGE_UP;
    else if (!strcasecmp(input, "pagedown")
	|| !strcasecmp(input, "nextpage"))
	s->scfunc = DO_PAGE_DOWN;
=======
	s->scfunc = do_search;
    else if (!strcasecmp(input, "searchagain") ||
	     !strcasecmp(input, "research"))  /* Deprecated.  Remove in 2018. */
	s->scfunc = do_research;
#ifndef NANO_TINY
    else if (!strcasecmp(input, "findprevious"))
	s->scfunc = do_findprevious;
    else if (!strcasecmp(input, "findnext"))
	s->scfunc = do_findnext;
#endif
    else if (!strcasecmp(input, "replace"))
	s->scfunc = do_replace;
>>>>>>> origin/tomato-shibby-RT-AC
    else if (!strcasecmp(input, "cut"))
	s->scfunc = DO_CUT_TEXT_VOID;
    else if (!strcasecmp(input, "uncut"))
	s->scfunc = DO_UNCUT_TEXT;
    else if (!strcasecmp(input, "curpos") ||
<<<<<<< HEAD
	!strcasecmp(input, "cursorpos"))
	s->scfunc = DO_CURSORPOS_VOID;
    else if (!strcasecmp(input, "firstline"))
	s->scfunc = DO_FIRST_LINE;
    else if (!strcasecmp(input, "lastline"))
	s->scfunc = DO_LAST_LINE;
=======
	     !strcasecmp(input, "cursorpos"))  /* Deprecated.  Remove in 2018. */
	s->scfunc = do_cursorpos_void;
>>>>>>> origin/tomato-shibby-RT-AC
    else if (!strcasecmp(input, "gotoline"))
	s->scfunc = DO_GOTOLINECOLUMN_VOID;
    else if (!strcasecmp(input, "replace"))
	s->scfunc = DO_REPLACE;
#ifndef DISABLE_JUSTIFY
    else if (!strcasecmp(input, "justify"))
	s->scfunc = DO_JUSTIFY_VOID;
    else if (!strcasecmp(input, "beginpara"))
	s->scfunc = DO_PARA_BEGIN_VOID;
    else if (!strcasecmp(input, "endpara"))
	s->scfunc = DO_PARA_END_VOID;
    else if (!strcasecmp(input, "fulljustify"))
	s->scfunc = DO_FULL_JUSTIFY;
#endif
#ifdef ENABLE_WORDCOMPLETION
    else if (!strcasecmp(input, "complete"))
	s->scfunc = complete_a_word;
#endif
#ifndef NANO_TINY
    else if (!strcasecmp(input, "mark"))
	s->scfunc = DO_MARK;
    else if (!strcasecmp(input, "searchagain") ||
		!strcasecmp(input, "research"))
	s->scfunc = DO_RESEARCH;
    else if (!strcasecmp(input, "copytext"))
	s->scfunc = DO_COPY_TEXT;
    else if (!strcasecmp(input, "indent"))
	s->scfunc = DO_INDENT_VOID;
    else if (!strcasecmp(input, "unindent"))
	s->scfunc = DO_UNINDENT;
    else if (!strcasecmp(input, "scrollup"))
	s->scfunc = DO_SCROLL_UP;
    else if (!strcasecmp(input, "scrolldown"))
	s->scfunc = DO_SCROLL_DOWN;
    else if (!strcasecmp(input, "nextword"))
<<<<<<< HEAD
	s->scfunc = DO_NEXT_WORD_VOID;
    else if (!strcasecmp(input, "suspend"))
	s->scfunc = DO_SUSPEND_VOID;
    else if (!strcasecmp(input, "prevword"))
	s->scfunc = DO_PREV_WORD_VOID;
=======
	s->scfunc = do_next_word_void;
    else if (!strcasecmp(input, "cutwordleft"))
	s->scfunc = do_cut_prev_word;
    else if (!strcasecmp(input, "cutwordright"))
	s->scfunc = do_cut_next_word;
    else if (!strcasecmp(input, "prevblock"))
	s->scfunc = do_prev_block;
    else if (!strcasecmp(input, "nextblock"))
	s->scfunc = do_next_block;
>>>>>>> origin/tomato-shibby-RT-AC
    else if (!strcasecmp(input, "findbracket"))
	s->scfunc = DO_FIND_BRACKET;
    else if (!strcasecmp(input, "wordcount"))
	s->scfunc = DO_WORDLINECHAR_COUNT;
    else if (!strcasecmp(input, "undo"))
	s->scfunc = DO_UNDO;
    else if (!strcasecmp(input, "redo"))
	s->scfunc = DO_REDO;
    else if (!strcasecmp(input, "prevhistory")) {
	s->scfunc =  PREV_HISTORY_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "nexthistory")) {
	s->scfunc =  NEXT_HISTORY_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "nohelp") ||
		!strcasecmp(input, "nohelp")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = NO_HELP;
    } else if (!strcasecmp(input, "constupdate")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = CONST_UPDATE;
    } else if (!strcasecmp(input, "morespace")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = MORE_SPACE;
    } else if (!strcasecmp(input, "smoothscroll")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = SMOOTH_SCROLL;
    } else if (!strcasecmp(input, "whitespacedisplay")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = WHITESPACE_DISPLAY;
    } else if (!strcasecmp(input, "nosyntax")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = NO_COLOR_SYNTAX;
    } else if (!strcasecmp(input, "smarthome")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = SMART_HOME;
    } else if (!strcasecmp(input, "autoindent")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = AUTOINDENT;
    } else if (!strcasecmp(input, "cuttoend")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = CUT_TO_END;
    } else if (!strcasecmp(input, "nowrap")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = NO_WRAP;
    } else if (!strcasecmp(input, "tabstospaces")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = TABS_TO_SPACES;
    } else if (!strcasecmp(input, "backupfile")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = BACKUP_FILE;
    } else if (!strcasecmp(input, "mutlibuffer")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = MULTIBUFFER;
    } else if (!strcasecmp(input, "mouse")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = USE_MOUSE;
    } else if (!strcasecmp(input, "noconvert")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = NO_CONVERT;
    } else if (!strcasecmp(input, "suspendenable")) {
	s->scfunc =  DO_TOGGLE;
	s->execute = FALSE;
	s->toggle = SUSPEND;
    }
#endif /* NANO_TINY */
    else if (!strcasecmp(input, "right") ||
		!strcasecmp(input, "forward"))
	s->scfunc = DO_RIGHT;
    else if (!strcasecmp(input, "left") ||
		!strcasecmp(input, "back"))
	s->scfunc = DO_LEFT;
    else if (!strcasecmp(input, "up") ||
		!strcasecmp(input, "prevline"))
	s->scfunc = DO_UP_VOID;
    else if (!strcasecmp(input, "down") ||
		!strcasecmp(input, "nextline"))
	s->scfunc = DO_DOWN_VOID;
    else if (!strcasecmp(input, "home"))
<<<<<<< HEAD
	s->scfunc = DO_HOME;
    else if (!strcasecmp(input, "end"))
	s->scfunc = DO_END;
#ifdef ENABLE_MULTIBUFFER
=======
	s->scfunc = do_home_void;
    else if (!strcasecmp(input, "end"))
	s->scfunc = do_end_void;
    else if (!strcasecmp(input, "pageup") ||
	     !strcasecmp(input, "prevpage"))
	s->scfunc = do_page_up;
    else if (!strcasecmp(input, "pagedown") ||
	     !strcasecmp(input, "nextpage"))
	s->scfunc = do_page_down;
    else if (!strcasecmp(input, "firstline"))
	s->scfunc = do_first_line;
    else if (!strcasecmp(input, "lastline"))
	s->scfunc = do_last_line;
#ifndef DISABLE_MULTIBUFFER
>>>>>>> origin/tomato-shibby-RT-AC
    else if (!strcasecmp(input, "prevbuf"))
	s->scfunc = SWITCH_TO_PREV_BUFFER_VOID;
    else if (!strcasecmp(input, "nextbuf"))
	s->scfunc = SWITCH_TO_NEXT_BUFFER_VOID;
#endif
    else if (!strcasecmp(input, "verbatim"))
	s->scfunc = DO_VERBATIM_INPUT;
    else if (!strcasecmp(input, "tab"))
	s->scfunc = DO_TAB;
    else if (!strcasecmp(input, "enter"))
	s->scfunc = DO_ENTER;
    else if (!strcasecmp(input, "delete"))
	s->scfunc = DO_DELETE;
    else if (!strcasecmp(input, "backspace"))
	s->scfunc = DO_BACKSPACE;
    else if (!strcasecmp(input, "refresh"))
<<<<<<< HEAD
	s->scfunc = TOTAL_REFRESH;
    else if (!strcasecmp(input, "casesens")) {
	s->scfunc =  CASE_SENS_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "regexp") ||
		!strcasecmp(input, "regex")) {
	s->scfunc =  REGEXP_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "dontreplace")) {
	s->scfunc =  NO_REPLACE_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "gototext")) {
	s->scfunc =  GOTOTEXT_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "browser") ||
		!strcasecmp(input, "tofiles")) {
	s->scfunc =  TO_FILES_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "dosformat")) {
	s->scfunc =  DOS_FORMAT_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "macformat")) {
	s->scfunc =  MAC_FORMAT_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "append")) {
	s->scfunc =  APPEND_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "prepend")) {
	s->scfunc =  PREPEND_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "backup")) {
	s->scfunc =  BACKUP_FILE_MSG;
	s->execute = FALSE;
#ifdef ENABLE_MULTIBUFFER
    } else if (!strcasecmp(input, "newbuffer")) {
	s->scfunc =  NEW_BUFFER_MSG;
	s->execute = FALSE;
#endif
    } else if (!strcasecmp(input, "firstfile")) {
	s->scfunc =  FIRST_FILE_MSG;
	s->execute = FALSE;
    } else if (!strcasecmp(input, "lastfile")) {
	s->scfunc =  LAST_FILE_MSG;
	s->execute = FALSE;
    } else {
	free(s);
	return NULL;
    }

=======
	s->scfunc = total_refresh;
    else if (!strcasecmp(input, "suspend"))
	s->scfunc = do_suspend_void;
    else if (!strcasecmp(input, "casesens"))
	s->scfunc = case_sens_void;
    else if (!strcasecmp(input, "regexp") ||
	     !strcasecmp(input, "regex"))  /* Deprecated.  Remove in 2018. */
	s->scfunc = regexp_void;
    else if (!strcasecmp(input, "backwards"))
	s->scfunc = backwards_void;
    else if (!strcasecmp(input, "flipreplace") ||
	     !strcasecmp(input, "dontreplace"))  /* Deprecated.  Remove in 2018. */
	s->scfunc = flip_replace_void;
    else if (!strcasecmp(input, "gototext"))
	s->scfunc = gototext_void;
#ifndef DISABLE_HISTORIES
    else if (!strcasecmp(input, "prevhistory"))
	s->scfunc = get_history_older_void;
    else if (!strcasecmp(input, "nexthistory"))
	s->scfunc = get_history_newer_void;
#endif
    else if (!strcasecmp(input, "dosformat"))
	s->scfunc = dos_format_void;
    else if (!strcasecmp(input, "macformat"))
	s->scfunc = mac_format_void;
    else if (!strcasecmp(input, "append"))
	s->scfunc = append_void;
    else if (!strcasecmp(input, "prepend"))
	s->scfunc = prepend_void;
    else if (!strcasecmp(input, "backup"))
	s->scfunc = backup_file_void;
#ifndef ENABLE_TINY
    else if (!strcasecmp(input, "flipexecute"))
	s->scfunc = flip_execute_void;
#endif
#ifndef DISABLE_MULTIBUFFER
    else if (!strcasecmp(input, "flipnewbuffer") ||
	     !strcasecmp(input, "newbuffer"))  /* Deprecated.  Remove in 2018. */
	s->scfunc = new_buffer_void;
#endif
#ifndef DISABLE_BROWSER
    else if (!strcasecmp(input, "tofiles") ||
	     !strcasecmp(input, "browser"))
	s->scfunc = to_files_void;
    else if (!strcasecmp(input, "gotodir"))
	s->scfunc = goto_dir_void;
    else if (!strcasecmp(input, "firstfile"))
	s->scfunc = do_first_file;
    else if (!strcasecmp(input, "lastfile"))
	s->scfunc = do_last_file;
#endif
    else {
#ifndef NANO_TINY
	s->scfunc = do_toggle_void;
	if (!strcasecmp(input, "nohelp"))
	    s->toggle = NO_HELP;
	else if (!strcasecmp(input, "constupdate"))
	    s->toggle = CONST_UPDATE;
	else if (!strcasecmp(input, "morespace"))
	    s->toggle = MORE_SPACE;
	else if (!strcasecmp(input, "smoothscroll"))
	    s->toggle = SMOOTH_SCROLL;
	else if (!strcasecmp(input, "softwrap"))
	    s->toggle = SOFTWRAP;
	else if (!strcasecmp(input, "whitespacedisplay"))
	    s->toggle = WHITESPACE_DISPLAY;
#ifndef DISABLE_COLOR
	else if (!strcasecmp(input, "nosyntax"))
	    s->toggle = NO_COLOR_SYNTAX;
#endif
	else if (!strcasecmp(input, "smarthome"))
	    s->toggle = SMART_HOME;
	else if (!strcasecmp(input, "autoindent"))
	    s->toggle = AUTOINDENT;
	else if (!strcasecmp(input, "cuttoend"))
	    s->toggle = CUT_TO_END;
#ifndef DISABLE_WRAPPING
	else if (!strcasecmp(input, "nowrap"))
	    s->toggle = NO_WRAP;
#endif
	else if (!strcasecmp(input, "tabstospaces"))
	    s->toggle = TABS_TO_SPACES;
	else if (!strcasecmp(input, "backupfile"))
	    s->toggle = BACKUP_FILE;
#ifndef DISABLE_MULTIBUFFER
	else if (!strcasecmp(input, "multibuffer"))
	    s->toggle = MULTIBUFFER;
#endif
#ifndef DISABLE_MOUSE
	else if (!strcasecmp(input, "mouse"))
	    s->toggle = USE_MOUSE;
#endif
	else if (!strcasecmp(input, "noconvert"))
	    s->toggle = NO_CONVERT;
	else if (!strcasecmp(input, "suspendenable"))
	    s->toggle = SUSPEND;
	else
#endif /* !NANO_TINY */
	{
	    free(s);
	    return NULL;
	}
    }
>>>>>>> origin/tomato-shibby-RT-AC
    return s;

}

#ifdef ENABLE_NANORC
/* Same thing as abnove but for the menu */
int strtomenu(char *input)
{
    if (!strcasecmp(input, "all"))
	return MALL;
    else if (!strcasecmp(input, "main"))
	return MMAIN;
    else if (!strcasecmp(input, "search"))
	return MWHEREIS;
    else if (!strcasecmp(input, "replace"))
	return MREPLACE;
<<<<<<< HEAD
    else if (!strcasecmp(input, "replace2") ||
		!strcasecmp(input, "replacewith"))
	return MREPLACE2;
=======
    else if (!strcasecmp(input, "replace2") ||  /* Deprecated.  Remove in 2018. */
	     !strcasecmp(input, "replacewith"))
	return MREPLACEWITH;
>>>>>>> origin/tomato-shibby-RT-AC
    else if (!strcasecmp(input, "gotoline"))
	return MGOTOLINE;
    else if (!strcasecmp(input, "writeout"))
	return MWRITEFILE;
    else if (!strcasecmp(input, "insert"))
	return MINSERTFILE;
    else if (!strcasecmp(input, "externalcmd") ||
		!strcasecmp(input, "extcmd"))
	return MEXTCMD;
    else if (!strcasecmp(input, "help"))
	return MHELP;
    else if (!strcasecmp(input, "spell"))
	return MSPELL;
    else if (!strcasecmp(input, "browser"))
	return MBROWSER;
    else if (!strcasecmp(input, "whereisfile"))
	return MWHEREISFILE;
    else if (!strcasecmp(input, "gotodir"))
	return MGOTODIR;

    return -1;
}
#endif


#ifdef DEBUG
/* This function is used to gracefully return all the memory we've used.
 * It should be called just before calling exit().  Practically, the
 * only effect is to cause a segmentation fault if the various data
 * structures got bolloxed earlier.  Thus, we don't bother having this
 * function unless debugging is turned on. */
void thanks_for_all_the_fish(void)
{
    delwin(topwin);
    delwin(edit);
    delwin(bottomwin);

    free(word_chars);
#ifndef DISABLE_JUSTIFY
<<<<<<< HEAD
    if (quotestr != NULL)
	free(quotestr);
#ifdef HAVE_REGEX_H
=======
    free(quotestr);
>>>>>>> origin/tomato-shibby-RT-AC
    regfree(&quotereg);
    if (quoteerr != NULL)
	free(quoteerr);
#endif
#ifndef NANO_TINY
    if (backup_dir != NULL)
        free(backup_dir);
#endif
#ifndef DISABLE_OPERATINGDIR
    if (operating_dir != NULL)
	free(operating_dir);
    if (full_operating_dir != NULL)
	free(full_operating_dir);
#endif
    if (last_search != NULL)
	free(last_search);
    if (last_replace != NULL)
	free(last_replace);
#ifndef DISABLE_SPELLER
    if (alt_speller != NULL)
	free(alt_speller);
#endif
<<<<<<< HEAD
    if (answer != NULL)
	free(answer);
    if (cutbuffer != NULL)
	free_filestruct(cutbuffer);
#ifndef DISABLE_JUSTIFY
    if (jusbuffer != NULL)
	free_filestruct(jusbuffer);
#endif
#ifdef DEBUG
=======
    free_filestruct(cutbuffer);
>>>>>>> origin/tomato-shibby-RT-AC
    /* Free the memory associated with each open file buffer. */
    if (openfile != NULL)
	free_openfilestruct(openfile);
#endif
#ifdef ENABLE_COLOR
    if (syntaxstr != NULL)
	free(syntaxstr);
    while (syntaxes != NULL) {
	syntaxtype *bill = syntaxes;

	free(syntaxes->desc);
	while (syntaxes->extensions != NULL) {
	    exttype *bob = syntaxes->extensions;

	    syntaxes->extensions = bob->next;
	    free(bob->ext_regex);
	    if (bob->ext != NULL) {
		regfree(bob->ext);
		free(bob->ext);
	    }
	    free(bob);
	}
	while (syntaxes->color != NULL) {
	    colortype *bob = syntaxes->color;

	    syntaxes->color = bob->next;
	    free(bob->start_regex);
	    if (bob->start != NULL) {
		regfree(bob->start);
		free(bob->start);
	    }
	    if (bob->end_regex != NULL)
		free(bob->end_regex);
	    if (bob->end != NULL) {
		regfree(bob->end);
		free(bob->end);
	    }
	    free(bob);
	}
	syntaxes = syntaxes->next;
	free(bill);
    }
#endif /* ENABLE_COLOR */
#ifndef NANO_TINY
    /* Free the search and replace history lists. */
    if (searchage != NULL)
	free_filestruct(searchage);
    if (replaceage != NULL)
	free_filestruct(replaceage);
#endif
#ifdef ENABLE_NANORC
    if (homedir != NULL)
	free(homedir);
#endif
}
#endif /* DEBUG */

