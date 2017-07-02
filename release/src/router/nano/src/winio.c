/* $Id: winio.c 4484 2010-03-07 19:35:46Z astyanax $ */
/**************************************************************************
 *   winio.c  --  This file is part of GNU nano.                          *
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
 *   Copyright (C) 2014, 2015, 2016, 2017 Benno Schulenberg               *
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
#include "revision.h"

#ifdef __linux__
#include <sys/ioctl.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#ifdef REVISION
#define BRANDING REVISION
#else
#define BRANDING PACKAGE_STRING
#endif

static int *key_buffer = NULL;
	/* The keystroke buffer, containing all the keystrokes we
	 * haven't handled yet at a given point. */
static size_t key_buffer_len = 0;
	/* The length of the keystroke buffer. */
static bool solitary = FALSE;
	/* Whether an Esc arrived by itself -- not as leader of a sequence. */
static int statusblank = 0;
<<<<<<< HEAD
	/* The number of keystrokes left after we call statusbar(),
	 * before we actually blank the statusbar. */
static bool disable_cursorpos = FALSE;
	/* Should we temporarily disable constant cursor position
	 * display? */
=======
	/* The number of keystrokes left before we blank the statusbar. */
static bool suppress_cursorpos = FALSE;
	/* Should we skip constant position display for one keystroke? */
#ifdef USING_OLD_NCURSES
static bool seen_wide = FALSE;
	/* Whether we've seen a multicolumn character in the current line. */
#endif
>>>>>>> origin/tomato-shibby-RT-AC

/* Control character compatibility:
 *
 * - Ctrl-H is Backspace under ASCII, ANSI, VT100, and VT220.
 * - Ctrl-I is Tab under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-M is Enter under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-Q is XON under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-S is XOFF under ASCII, ANSI, VT100, VT220, and VT320.
 * - Ctrl-8 (Ctrl-?) is Delete under ASCII, ANSI, VT100, and VT220,
 *          but is Backspace under VT320.
 *
 * Note: VT220 and VT320 also generate Esc [ 3 ~ for Delete.  By
 * default, xterm assumes it's running on a VT320 and generates Ctrl-8
 * (Ctrl-?) for Backspace and Esc [ 3 ~ for Delete.  This causes
 * problems for VT100-derived terminals such as the FreeBSD console,
 * which expect Ctrl-H for Backspace and Ctrl-8 (Ctrl-?) for Delete, and
 * on which the VT320 sequences are translated by the keypad to KEY_DC
 * and [nothing].  We work around this conflict via the REBIND_DELETE
 * flag: if it's not set, we assume VT320 compatibility, and if it is,
 * we assume VT100 compatibility.  Thanks to Lee Nelson and Wouter van
 * Hemel for helping work this conflict out.
 *
 * Escape sequence compatibility:
 *
 * We support escape sequences for ANSI, VT100, VT220, VT320, the Linux
 * console, the FreeBSD console, the Mach console, xterm, rxvt, Eterm,
 * and Terminal, and some for iTerm2.  Among these, there are several
 * conflicts and omissions, outlined as follows:
 *
 * - Tab on ANSI == PageUp on FreeBSD console; the former is omitted.
 *   (Ctrl-I is also Tab on ANSI, which we already support.)
 * - PageDown on FreeBSD console == Center (5) on numeric keypad with
 *   NumLock off on Linux console; the latter is omitted.  (The editing
 *   keypad key is more important to have working than the numeric
 *   keypad key, because the latter has no value when NumLock is off.)
 * - F1 on FreeBSD console == the mouse key on xterm/rxvt/Eterm; the
 *   latter is omitted.  (Mouse input will only work properly if the
 *   extended keypad value KEY_MOUSE is generated on mouse events
 *   instead of the escape sequence.)
 * - F9 on FreeBSD console == PageDown on Mach console; the former is
 *   omitted.  (The editing keypad is more important to have working
 *   than the function keys, because the functions of the former are not
 *   arbitrary and the functions of the latter are.)
 * - F10 on FreeBSD console == PageUp on Mach console; the former is
 *   omitted.  (Same as above.)
 * - F13 on FreeBSD console == End on Mach console; the former is
 *   omitted.  (Same as above.)
 * - F15 on FreeBSD console == Shift-Up on rxvt/Eterm; the former is
 *   omitted.  (The arrow keys, with or without modifiers, are more
 *   important to have working than the function keys, because the
 *   functions of the former are not arbitrary and the functions of the
 *   latter are.)
 * - F16 on FreeBSD console == Shift-Down on rxvt/Eterm; the former is
 *   omitted.  (Same as above.) */

/* Read in a sequence of keystrokes from win and save them in the
 * keystroke buffer.  This should only be called when the keystroke
 * buffer is empty. */
void get_key_buffer(WINDOW *win)
{
    int input;
    size_t errcount;

    /* If the keystroke buffer isn't empty, get out. */
    if (key_buffer != NULL)
	return;

    /* Read in the first character using blocking input. */
#ifndef NANO_TINY
<<<<<<< HEAD
    allow_pending_sigwinch(TRUE);
=======
    if (the_window_resized) {
	ungetch(input);
	regenerate_screen();
	input = KEY_WINCH;
    }
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    /* Just before reading in the first character, display any pending
     * screen updates. */
    doupdate();

    errcount = 0;
    if (nodelay_mode) {
	if ((input =  wgetch(win)) == ERR)
           return;
    } else
	while ((input = wgetch(win)) == ERR) {
	    errcount++;

	    /* If we've failed to get a character MAX_BUF_SIZE times in a
	     * row, assume that the input source we were using is gone and
	     * die gracefully.  We could check if errno is set to EIO
	     * ("Input/output error") and die gracefully in that case, but
	     * it's not always set properly.  Argh. */
	    if (errcount == MAX_BUF_SIZE)
		handle_hupterm(0);
	}

#ifndef NANO_TINY
<<<<<<< HEAD
    allow_pending_sigwinch(FALSE);
=======
	if (the_window_resized) {
	    regenerate_screen();
	    input = KEY_WINCH;
	    break;
	}
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    /* Increment the length of the keystroke buffer, and save the value
     * of the keystroke at the end of it. */
    key_buffer_len++;
    key_buffer = (int *)nmalloc(sizeof(int));
    key_buffer[0] = input;

    /* Read in the remaining characters using non-blocking input. */
    nodelay(win, TRUE);

    while (TRUE) {
#ifndef NANO_TINY
	allow_pending_sigwinch(TRUE);
#endif

	input = wgetch(win);

	/* If there aren't any more characters, stop reading. */
	if (input == ERR)
	    break;

	/* Otherwise, increment the length of the keystroke buffer, and
	 * save the value of the keystroke at the end of it. */
	key_buffer_len++;
	key_buffer = (int *)nrealloc(key_buffer, key_buffer_len *
		sizeof(int));
	key_buffer[key_buffer_len - 1] = input;

#ifndef NANO_TINY
	allow_pending_sigwinch(FALSE);
#endif
    }

    /* Switch back to non-blocking input. */
    nodelay(win, FALSE);

#ifdef DEBUG
    fprintf(stderr, "get_key_buffer(): key_buffer_len = %lu\n", (unsigned long)key_buffer_len);
#endif
}

/* Return the length of the keystroke buffer. */
size_t get_key_buffer_len(void)
{
    return key_buffer_len;
}

/* Add the keystrokes in input to the keystroke buffer. */
void unget_input(int *input, size_t input_len)
{
#ifndef NANO_TINY
    allow_pending_sigwinch(TRUE);
    allow_pending_sigwinch(FALSE);
#endif

    /* If input is empty, get out. */
    if (input_len == 0)
	return;

    /* If adding input would put the keystroke buffer beyond maximum
     * capacity, only add enough of input to put it at maximum
     * capacity. */
    if (key_buffer_len + input_len < key_buffer_len)
	input_len = (size_t)-1 - key_buffer_len;

    /* Add the length of input to the length of the keystroke buffer,
     * and reallocate the keystroke buffer so that it has enough room
     * for input. */
    key_buffer_len += input_len;
    key_buffer = (int *)nrealloc(key_buffer, key_buffer_len *
	sizeof(int));

    /* If the keystroke buffer wasn't empty before, move its beginning
     * forward far enough so that we can add input to its beginning. */
    if (key_buffer_len > input_len)
	memmove(key_buffer + input_len, key_buffer,
		(key_buffer_len - input_len) * sizeof(int));

    /* Copy input to the beginning of the keystroke buffer. */
    memcpy(key_buffer, input, input_len * sizeof(int));
}

<<<<<<< HEAD
/* Put back the character stored in kbinput, putting it in byte range
 * beforehand.  If meta_key is TRUE, put back the Escape character after
 * putting back kbinput.  If func_key is TRUE, put back the function key
 * (a value outside byte range) without putting it in byte range. */
void unget_kbinput(int kbinput, bool meta_key, bool func_key)
{
    if (!func_key)
	kbinput = (char)kbinput;

    unget_input(&kbinput, 1);

    if (meta_key) {
	kbinput = NANO_CONTROL_3;
=======
/* Put the character given in kbinput back into the input stream.  If it
 * is a Meta key, also insert an Escape character in front of it. */
void unget_kbinput(int kbinput, bool metakey)
{
    unget_input(&kbinput, 1);

    if (metakey) {
	kbinput = ESC_CODE;
>>>>>>> origin/tomato-shibby-RT-AC
	unget_input(&kbinput, 1);
    }
}

/* Try to read input_len codes from the keystroke buffer.  If the
 * keystroke buffer is empty and win isn't NULL, try to read in more
<<<<<<< HEAD
 * characters from win and add them to the keystroke buffer before doing
 * anything else.  If the keystroke buffer is empty and win is NULL,
 * return NULL. */
=======
 * codes from win and add them to the keystroke buffer before doing
 * anything else.  If the keystroke buffer is (still) empty, return NULL. */
>>>>>>> origin/tomato-shibby-RT-AC
int *get_input(WINDOW *win, size_t input_len)
{
    int *input;

#ifndef NANO_TINY
    allow_pending_sigwinch(TRUE);
    allow_pending_sigwinch(FALSE);
#endif

    if (key_buffer_len == 0) {
	if (win != NULL) {
	    get_key_buffer(win);

	    if (key_buffer_len == 0)
		return NULL;
	} else
	    return NULL;
    }

    /* Limit the request to the number of available codes in the buffer. */
    if (input_len > key_buffer_len)
	input_len = key_buffer_len;

    /* Copy input_len codes from the head of the keystroke buffer. */
    input = (int *)nmalloc(input_len * sizeof(int));
    memcpy(input, key_buffer, input_len * sizeof(int));
    key_buffer_len -= input_len;

    /* If the keystroke buffer is now empty, mark it as such. */
    if (key_buffer_len == 0) {
	free(key_buffer);
	key_buffer = NULL;
    } else {
	/* Trim from the buffer the codes that were copied. */
	memmove(key_buffer, key_buffer + input_len, key_buffer_len *
		sizeof(int));
	key_buffer = (int *)nrealloc(key_buffer, key_buffer_len *
		sizeof(int));
    }

    return input;
}

/* Read in a single character.  If it's ignored, swallow it and go on.
 * Otherwise, try to translate it from ASCII, meta key sequences, escape
 * sequences, and/or extended keypad values.  Set meta_key to TRUE when
 * we get a meta key sequence, and set func_key to TRUE when we get an
 * extended keypad value.  Supported extended keypad values consist of
 * [arrow key], Ctrl-[arrow key], Shift-[arrow key], Enter, Backspace,
 * the editing keypad (Insert, Delete, Home, End, PageUp, and PageDown),
 * the function keypad (F1-F16), and the numeric keypad with NumLock
 * off.  Assume nodelay(win) is FALSE. */
int get_kbinput(WINDOW *win, bool *meta_key, bool *func_key)
{
    int kbinput = ERR;

<<<<<<< HEAD
    /* Read in a character and interpret it.  Continue doing this until
     * we get a recognized value or sequence. */
    while ((kbinput = parse_kbinput(win, meta_key, func_key)) == ERR);
=======
    /* Extract one keystroke from the input stream. */
    while (kbinput == ERR)
	kbinput = parse_kbinput(win);

#ifdef DEBUG
    fprintf(stderr, "after parsing:  kbinput = %d, meta_key = %s\n",
	kbinput, meta_key ? "TRUE" : "FALSE");
#endif
>>>>>>> origin/tomato-shibby-RT-AC

    /* If we read from the edit window, blank the statusbar if we need
     * to. */
    if (win == edit)
	check_statusblank();

    return kbinput;
}

<<<<<<< HEAD
/* Translate ASCII characters, extended keypad values, and escape
 * sequences into their corresponding key values.  Set meta_key to TRUE
 * when we get a meta key sequence, and set func_key to TRUE when we get
 * a function key.  Assume nodelay(win) is FALSE. */
int parse_kbinput(WINDOW *win, bool *meta_key, bool *func_key)
{
    static int escapes = 0, byte_digits = 0;
    int *kbinput, retval = ERR;

    *meta_key = FALSE;
    *func_key = FALSE;
=======
/* Extract a single keystroke from the input stream.  Translate escape
 * sequences and extended keypad codes into their corresponding values.
 * Set meta_key to TRUE when appropriate.  Supported extended keypad values
 * are: [arrow key], Ctrl-[arrow key], Shift-[arrow key], Enter, Backspace,
 * the editing keypad (Insert, Delete, Home, End, PageUp, and PageDown),
 * the function keys (F1-F16), and the numeric keypad with NumLock off. */
int parse_kbinput(WINDOW *win)
{
    static int escapes = 0, byte_digits = 0;
    static bool double_esc = FALSE;
    int *kbinput, keycode, retval = ERR;

    meta_key = FALSE;
    shift_held = FALSE;
>>>>>>> origin/tomato-shibby-RT-AC

    /* Read in a character. */
    if (nodelay_mode) {
	kbinput = get_input(win, 1);
	if (kbinput == 0)
	    return 0;
    } else
	while ((kbinput = get_input(win, 1)) == NULL);

    keycode = *kbinput;
    free(kbinput);

#ifdef DEBUG
    fprintf(stderr, "before parsing:  keycode = %d, escapes = %d, byte_digits = %d\n",
	keycode, escapes, byte_digits);
#endif

    if (keycode == ERR)
	return ERR;

    if (keycode == ESC_CODE) {
	/* Increment the escape counter, but trim an overabundance. */
	escapes++;
	if (escapes > 3)
	    escapes = 1;
	/* Take note when an Esc arrived by itself. */
	solitary = (escapes == 1 && key_buffer_len == 0);
	return ERR;
    }

    switch (escapes) {
	case 0:
	    /* One non-escape: normal input mode. */
	    retval = keycode;
	    break;
<<<<<<< HEAD
	case NANO_CONTROL_3:
	    /* Increment the escape counter. */
	    escapes++;
	    switch (escapes) {
		case 1:
		    /* One escape: wait for more input. */
		case 2:
		    /* Two escapes: wait for more input. */
		case 3:
		    /* Three escapes: wait for more input. */
		    break;
		default:
		    /* More than three escapes: limit the escape counter
		     * to no more than two, and wait for more input. */
		    escapes %= 3;
	    }
	    break;
	default:
	    switch (escapes) {
		case 0:
		    /* One non-escape: normal input mode.  Save the
		     * non-escape character as the result. */
		    retval = *kbinput;
		    break;
		case 1:
		    /* Reset the escape counter. */
		    escapes = 0;
		    if (get_key_buffer_len() == 0) {
			/* One escape followed by a non-escape, and
			 * there aren't any other keystrokes waiting:
			 * meta key sequence mode.  Set meta_key to
			 * TRUE, and save the lowercase version of the
			 * non-escape character as the result. */
			*meta_key = TRUE;
			retval = tolower(*kbinput);
		    } else
			/* One escape followed by a non-escape, and
			 * there are other keystrokes waiting: escape
			 * sequence mode.  Interpret the escape
			 * sequence. */
			retval = parse_escape_seq_kbinput(win,
				*kbinput);
		    break;
		case 2:
		    if (get_key_buffer_len() == 0) {
			if (('0' <= *kbinput && *kbinput <= '2' &&
				byte_digits == 0) || ('0' <= *kbinput &&
				*kbinput <= '9' && byte_digits > 0)) {
			    /* Two escapes followed by one or more
			     * decimal digits, and there aren't any
			     * other keystrokes waiting: byte sequence
			     * mode.  If the byte sequence's range is
			     * limited to 2XX (the first digit is in the
			     * '0' to '2' range and it's the first
			     * digit, or it's in the '0' to '9' range
			     * and it's not the first digit), increment
			     * the byte sequence counter and interpret
			     * the digit.  If the byte sequence's range
			     * is not limited to 2XX, fall through. */
			    int byte;

			    byte_digits++;
			    byte = get_byte_kbinput(*kbinput);

			    if (byte != ERR) {
				char *byte_mb;
				int byte_mb_len, *seq, i;

				/* If we've read in a complete byte
				 * sequence, reset the escape counter
				 * and the byte sequence counter, and
				 * put back the corresponding byte
				 * value. */
				escapes = 0;
				byte_digits = 0;

				/* Put back the multibyte equivalent of
				 * the byte value. */
				byte_mb = make_mbchar((long)byte,
					&byte_mb_len);

				seq = (int *)nmalloc(byte_mb_len *
					sizeof(int));

				for (i = 0; i < byte_mb_len; i++)
				    seq[i] = (unsigned char)byte_mb[i];

				unget_input(seq, byte_mb_len);

				free(byte_mb);
				free(seq);
			    }
			} else {
			    /* Reset the escape counter. */
			    escapes = 0;
			    if (byte_digits == 0)
				/* Two escapes followed by a non-decimal
				 * digit or a decimal digit that would
				 * create a byte sequence greater than
				 * 2XX, we're not in the middle of a
				 * byte sequence, and there aren't any
				 * other keystrokes waiting: control
				 * character sequence mode.  Interpret
				 * the control sequence and save the
				 * corresponding control character as
				 * the result. */
				retval = get_control_kbinput(*kbinput);
			    else {
				/* If we're in the middle of a byte
				 * sequence, reset the byte sequence
				 * counter and save the character we got
				 * as the result. */
				byte_digits = 0;
				retval = *kbinput;
			    }
			}
		    } else {
			/* Two escapes followed by a non-escape, and
			 * there are other keystrokes waiting: combined
			 * meta and escape sequence mode.  Reset the
			 * escape counter, set meta_key to TRUE, and
			 * interpret the escape sequence. */
			escapes = 0;
			*meta_key = TRUE;
			retval = parse_escape_seq_kbinput(win,
				*kbinput);
=======
	case 1:
	    if (keycode >= 0x80)
		retval = keycode;
	    else if ((keycode != 'O' && keycode != 'o' && keycode != '[') ||
			key_buffer_len == 0 || *key_buffer == ESC_CODE) {
		/* One escape followed by a single non-escape:
		 * meta key sequence mode. */
		if (!solitary || (keycode >= 0x20 && keycode < 0x7F))
		    meta_key = TRUE;
		retval = tolower(keycode);
	    } else
		/* One escape followed by a non-escape, and there
		 * are more codes waiting: escape sequence mode. */
		retval = parse_escape_sequence(win, keycode);
	    escapes = 0;
	    break;
	case 2:
	    if (double_esc) {
		/* An "ESC ESC [ X" sequence from Option+arrow, or
		 * an "ESC ESC [ x" sequence from Shift+Alt+arrow. */
		switch (keycode) {
		    case 'A':
			retval = KEY_HOME;
			break;
		    case 'B':
			retval = KEY_END;
			break;
		    case 'C':
			retval = CONTROL_RIGHT;
			break;
		    case 'D':
			retval = CONTROL_LEFT;
			break;
#ifndef NANO_TINY
		    case 'a':
			retval = shiftaltup;
			break;
		    case 'b':
			retval = shiftaltdown;
			break;
		    case 'c':
			retval = shiftaltright;
			break;
		    case 'd':
			retval = shiftaltleft;
			break;
#endif
		}
		double_esc = FALSE;
		escapes = 0;
	    } else if (key_buffer_len == 0) {
		if (('0' <= keycode && keycode <= '2' &&
			byte_digits == 0) || ('0' <= keycode &&
			keycode <= '9' && byte_digits > 0)) {
		    /* Two escapes followed by one or more decimal
		     * digits, and there aren't any other codes
		     * waiting: byte sequence mode.  If the range
		     * of the byte sequence is limited to 2XX (the
		     * first digit is between '0' and '2' and the
		     * others between '0' and '9', interpret it. */
		    int byte;

		    byte_digits++;
		    byte = get_byte_kbinput(keycode);

		    /* If the decimal byte value is complete, convert it and
		     * put the obtained byte(s) back into the input buffer. */
		    if (byte != ERR) {
			char *byte_mb;
			int byte_mb_len, *seq, i;

			/* Convert the decimal code to one or two bytes. */
			byte_mb = make_mbchar((long)byte, &byte_mb_len);

			seq = (int *)nmalloc(byte_mb_len * sizeof(int));

			for (i = 0; i < byte_mb_len; i++)
			    seq[i] = (unsigned char)byte_mb[i];

			/* Insert the byte(s) into the input buffer. */
			unget_input(seq, byte_mb_len);

			free(byte_mb);
			free(seq);

			byte_digits = 0;
			escapes = 0;
>>>>>>> origin/tomato-shibby-RT-AC
		    }
		} else {
		    if (byte_digits == 0)
			/* Two escapes followed by a non-decimal
			 * digit (or a decimal digit that would
			 * create a byte sequence greater than 2XX)
			 * and there aren't any other codes waiting:
			 * control character sequence mode. */
			retval = get_control_kbinput(keycode);
		    else {
			/* An invalid digit in the middle of a byte
			 * sequence: reset the byte sequence counter
			 * and save the code we got as the result. */
			byte_digits = 0;
			retval = keycode;
		    }
		    escapes = 0;
<<<<<<< HEAD
		    if (get_key_buffer_len() == 0)
			/* Three escapes followed by a non-escape, and
			 * there aren't any other keystrokes waiting:
			 * normal input mode.  Save the non-escape
			 * character as the result. */
			retval = *kbinput;
		    else
			/* Three escapes followed by a non-escape, and
			 * there are other keystrokes waiting: combined
			 * control character and escape sequence mode.
			 * Interpret the escape sequence, and interpret
			 * the result as a control sequence. */
			retval = get_control_kbinput(
				parse_escape_seq_kbinput(win,
				*kbinput));
		    break;
=======
		}
	    } else if (keycode == '[' && key_buffer_len > 0 &&
			(('A' <= *key_buffer && *key_buffer <= 'D') ||
			('a' <= *key_buffer && *key_buffer <= 'd'))) {
		/* An iTerm2/Eterm/rxvt sequence: ^[ ^[ [ X. */
		double_esc = TRUE;
	    } else {
		/* Two escapes followed by a non-escape, and there are more
		 * codes waiting: combined meta and escape sequence mode. */
		retval = parse_escape_sequence(win, keycode);
		meta_key = TRUE;
		escapes = 0;
>>>>>>> origin/tomato-shibby-RT-AC
	    }
	    break;
	case 3:
	    if (key_buffer_len == 0)
		/* Three escapes followed by a non-escape, and no
		 * other codes are waiting: normal input mode. */
		retval = keycode;
	    else
		/* Three escapes followed by a non-escape, and more
		 * codes are waiting: combined control character and
		 * escape sequence mode.  First interpret the escape
		 * sequence, then the result as a control sequence. */
		retval = get_control_kbinput(
			parse_escape_sequence(win, keycode));
	    escapes = 0;
	    break;
    }

<<<<<<< HEAD
    if (retval != ERR) {
	switch (retval) {
	    case NANO_CONTROL_8:
		retval = ISSET(REBIND_DELETE) ? sc_seq_or(DO_DELETE, 0) :
			sc_seq_or(DO_BACKSPACE, 0);
		break;
	    case KEY_DOWN:
#ifdef KEY_SDOWN
	    /* ncurses and Slang don't support KEY_SDOWN. */
	    case KEY_SDOWN:
#endif
		retval = sc_seq_or(DO_DOWN_VOID, *kbinput);
		break;
	    case KEY_UP:
#ifdef KEY_SUP
	    /* ncurses and Slang don't support KEY_SUP. */
	    case KEY_SUP:
#endif
		retval = sc_seq_or(DO_UP_VOID, *kbinput);
		break;
	    case KEY_LEFT:
=======
    if (retval == ERR)
	return ERR;

    if (retval == controlleft)
	return CONTROL_LEFT;
    else if (retval == controlright)
	return CONTROL_RIGHT;
    else if (retval == controlup)
	return CONTROL_UP;
    else if (retval == controldown)
	return CONTROL_DOWN;
#ifndef NANO_TINY
    else if (retval == shiftcontrolleft) {
	shift_held = TRUE;
	return CONTROL_LEFT;
    } else if (retval == shiftcontrolright) {
	shift_held = TRUE;
	return CONTROL_RIGHT;
    } else if (retval == shiftcontrolup) {
	shift_held = TRUE;
	return CONTROL_UP;
    } else if (retval == shiftcontroldown) {
	shift_held = TRUE;
	return CONTROL_DOWN;
    } else if (retval == shiftaltleft) {
	shift_held = TRUE;
	return KEY_HOME;
    } else if (retval == shiftaltright) {
	shift_held = TRUE;
	return KEY_END;
    } else if (retval == shiftaltup) {
	shift_held = TRUE;
	return KEY_PPAGE;
    } else if (retval == shiftaltdown) {
	shift_held = TRUE;
	return KEY_NPAGE;
    }
#endif

#ifdef __linux__
    /* When not running under X, check for the bare arrow keys whether
     * Shift/Ctrl/Alt are being held together with them. */
    unsigned char modifiers = 6;

    if (console && ioctl(0, TIOCLINUX, &modifiers) >= 0) {
#ifndef NANO_TINY
	/* Is Shift being held? */
	if (modifiers & 0x01)
	    shift_held = TRUE;
#endif
	/* Is Ctrl being held? */
	if (modifiers & 0x04) {
	    if (retval == KEY_UP)
		return CONTROL_UP;
	    else if (retval == KEY_DOWN)
		return CONTROL_DOWN;
	    else if (retval == KEY_LEFT)
		return CONTROL_LEFT;
	    else if (retval == KEY_RIGHT)
		return CONTROL_RIGHT;
	}
#ifndef NANO_TINY
	/* Are both Shift and Alt being held? */
	if ((modifiers & 0x09) == 0x09) {
	    if (retval == KEY_UP)
		return KEY_PPAGE;
	    else if (retval == KEY_DOWN)
		return KEY_NPAGE;
	    else if (retval == KEY_LEFT)
		return KEY_HOME;
	    else if (retval == KEY_RIGHT)
		return KEY_END;
	}
#endif
    }
#endif /* __linux__ */

    switch (retval) {
>>>>>>> origin/tomato-shibby-RT-AC
#ifdef KEY_SLEFT
	/* Slang doesn't support KEY_SLEFT. */
	case KEY_SLEFT:
	    shift_held = TRUE;
	    return KEY_LEFT;
#endif
<<<<<<< HEAD
		retval = sc_seq_or(DO_LEFT, *kbinput);
		break;
	    case KEY_RIGHT:
=======
>>>>>>> origin/tomato-shibby-RT-AC
#ifdef KEY_SRIGHT
	/* Slang doesn't support KEY_SRIGHT. */
	case KEY_SRIGHT:
	    shift_held = TRUE;
	    return KEY_RIGHT;
#endif
#ifdef KEY_SR
#ifdef KEY_SUP
	/* ncurses and Slang don't support KEY_SUP. */
	case KEY_SUP:
#endif
	case KEY_SR:	/* Scroll backward, on Xfce4-terminal. */
	    shift_held = TRUE;
	    return KEY_UP;
#endif
#ifdef KEY_SF
#ifdef KEY_SDOWN
	/* ncurses and Slang don't support KEY_SDOWN. */
	case KEY_SDOWN:
#endif
	case KEY_SF:	/* Scroll forward, on Xfce4-terminal. */
	    shift_held = TRUE;
	    return KEY_DOWN;
#endif
<<<<<<< HEAD
		retval = sc_seq_or(DO_RIGHT, *kbinput);
		break;
=======
>>>>>>> origin/tomato-shibby-RT-AC
#ifdef KEY_SHOME
	/* HP-UX 10-11 and Slang don't support KEY_SHOME. */
	case KEY_SHOME:
#endif
<<<<<<< HEAD
	    case KEY_A1:	/* Home (7) on numeric keypad with
				 * NumLock off. */
		retval = sc_seq_or(DO_HOME, *kbinput);
		break;
	    case KEY_BACKSPACE:
		retval = sc_seq_or(DO_BACKSPACE, *kbinput);
		break;
#ifdef KEY_SDC
	    /* Slang doesn't support KEY_SDC. */
	    case KEY_SDC:
		if (ISSET(REBIND_DELETE))
		   retval = sc_seq_or(DO_DELETE, *kbinput);
		else
		   retval = sc_seq_or(DO_BACKSPACE, *kbinput);
		break;
=======
	case SHIFT_HOME:
	    shift_held = TRUE;
	case KEY_A1:	/* Home (7) on keypad with NumLock off. */
	    return KEY_HOME;
#ifdef KEY_SEND
	/* HP-UX 10-11 and Slang don't support KEY_SEND. */
	case KEY_SEND:
#endif
	case SHIFT_END:
	    shift_held = TRUE;
	case KEY_C1:	/* End (1) on keypad with NumLock off. */
	    return KEY_END;
#ifndef NANO_TINY
#ifdef KEY_SPREVIOUS
	case KEY_SPREVIOUS:
#endif
	case SHIFT_PAGEUP:	/* Fake key, from Shift+Alt+Up. */
	    shift_held = TRUE;
#endif
	case KEY_A3:	/* PageUp (9) on keypad with NumLock off. */
	    return KEY_PPAGE;
#ifndef NANO_TINY
#ifdef KEY_SNEXT
	case KEY_SNEXT:
#endif
	case SHIFT_PAGEDOWN:	/* Fake key, from Shift+Alt+Down. */
	    shift_held = TRUE;
#endif
	case KEY_C3:	/* PageDown (3) on keypad with NumLock off. */
	    return KEY_NPAGE;
#ifdef KEY_SDC
	/* Slang doesn't support KEY_SDC. */
	case KEY_SDC:
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	case DEL_CODE:
	    if (ISSET(REBIND_DELETE))
		return the_code_for(do_delete, KEY_DC);
	    else
		return KEY_BACKSPACE;
#ifdef KEY_SIC
<<<<<<< HEAD
	    /* Slang doesn't support KEY_SIC. */
	    case KEY_SIC:
		retval = sc_seq_or(DO_INSERTFILE_VOID, *kbinput);
		break;
#endif
	    case KEY_C3:	/* PageDown (4) on numeric keypad with
				 * NumLock off. */
		retval = sc_seq_or(DO_PAGE_DOWN, *kbinput);
		break;
	    case KEY_A3:	/* PageUp (9) on numeric keypad with
				 * NumLock off. */
		retval = sc_seq_or(DO_PAGE_UP, *kbinput);
		break;
	    case KEY_ENTER:
		retval = sc_seq_or(DO_ENTER, *kbinput);
		break;
	    case KEY_B2:	/* Center (5) on numeric keypad with
				 * NumLock off. */
		retval = ERR;
		break;
	    case KEY_C1:	/* End (1) on numeric keypad with
				 * NumLock off. */
#ifdef KEY_SEND
	    /* HP-UX 10-11 and Slang don't support KEY_SEND. */
	    case KEY_SEND:
#endif
		retval = sc_seq_or(DO_END, *kbinput);
		break;
=======
	/* Slang doesn't support KEY_SIC. */
	case KEY_SIC:
	    return the_code_for(do_insertfile_void, KEY_IC);
#endif
#ifdef KEY_SBEG
	/* Slang doesn't support KEY_SBEG. */
	case KEY_SBEG:
#endif
>>>>>>> origin/tomato-shibby-RT-AC
#ifdef KEY_BEG
	/* Slang doesn't support KEY_BEG. */
	case KEY_BEG:
#endif
	case KEY_B2:	/* Center (5) on keypad with NumLock off. */
	    return ERR;
#ifdef KEY_CANCEL
#ifdef KEY_SCANCEL
<<<<<<< HEAD
	    /* Slang doesn't support KEY_SCANCEL. */
	    case KEY_SCANCEL:
#endif
		retval = first_sc_for(currmenu, CANCEL_MSG)->seq;
		break;
=======
	/* Slang doesn't support KEY_SCANCEL. */
	case KEY_SCANCEL:
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	/* Slang doesn't support KEY_CANCEL. */
	case KEY_CANCEL:
	    return the_code_for(do_cancel, 0x03);
#endif
#ifdef KEY_SUSPEND
#ifdef KEY_SSUSPEND
<<<<<<< HEAD
	    /* Slang doesn't support KEY_SSUSPEND. */
	    case KEY_SSUSPEND:
		retval = sc_seq_or(DO_SUSPEND_VOID, 0);
		break;
#endif
#ifdef KEY_SUSPEND
	    /* Slang doesn't support KEY_SUSPEND. */
	    case KEY_SUSPEND:
		retval =  sc_seq_or(DO_SUSPEND_VOID, 0);
		break;
=======
	/* Slang doesn't support KEY_SSUSPEND. */
	case KEY_SSUSPEND:
#endif
	/* Slang doesn't support KEY_SUSPEND. */
	case KEY_SUSPEND:
	    return the_code_for(do_suspend_void, KEY_SUSPEND);
>>>>>>> origin/tomato-shibby-RT-AC
#endif
#ifdef PDCURSES
	case KEY_SHIFT_L:
	case KEY_SHIFT_R:
	case KEY_CONTROL_L:
	case KEY_CONTROL_R:
	case KEY_ALT_L:
	case KEY_ALT_R:
	    return ERR;
#endif
<<<<<<< HEAD
	}

	/* If our result is an extended keypad value (i.e. a value
	 * outside of byte range), set func_key to TRUE. */
	if (retval != ERR)
	    *func_key = !is_byte(retval);
    }

#ifdef DEBUG
    fprintf(stderr, "parse_kbinput(): kbinput = %d, meta_key = %s, func_key = %s, escapes = %d, byte_digits = %d, retval = %d\n", *kbinput, *meta_key ? "TRUE" : "FALSE", *func_key ? "TRUE" : "FALSE", escapes, byte_digits, retval);
#endif

    free(kbinput);

    /* Return the result. */
=======
#ifdef KEY_RESIZE
	/* Slang and SunOS 5.7-5.9 don't support KEY_RESIZE. */
	case KEY_RESIZE:
	    return ERR;
#endif
    }

>>>>>>> origin/tomato-shibby-RT-AC
    return retval;
}

/* Translate escape sequences, most of which correspond to extended
 * keypad values, into their corresponding key values.  These sequences
 * are generated when the keypad doesn't support the needed keys.
 * Assume that Escape has already been read in. */
int get_escape_seq_kbinput(const int *seq, size_t seq_len)
{
    int retval = ERR;

    if (seq_len > 1) {
	switch (seq[0]) {
	    case 'O':
		switch (seq[1]) {
		    case '1':
			if (seq_len > 4  && seq[2] == ';') {

	switch (seq[3]) {
	    case '2':
<<<<<<< HEAD
		if (seq_len >= 5) {
		    switch (seq[4]) {
			case 'A': /* Esc O 1 ; 2 A == Shift-Up on
				   * Terminal. */
			case 'B': /* Esc O 1 ; 2 B == Shift-Down on
				   * Terminal. */
			case 'C': /* Esc O 1 ; 2 C == Shift-Right on
				   * Terminal. */
			case 'D': /* Esc O 1 ; 2 D == Shift-Left on
				   * Terminal. */
			    retval = get_escape_seq_abcd(seq[4]);
			    break;
			case 'P': /* Esc O 1 ; 2 P == F13 on
				   * Terminal. */
			    retval = KEY_F(13);
			    break;
			case 'Q': /* Esc O 1 ; 2 Q == F14 on
				   * Terminal. */
			    retval = KEY_F(14);
			    break;
			case 'R': /* Esc O 1 ; 2 R == F15 on
				   * Terminal. */
			    retval = KEY_F(15);
			    break;
			case 'S': /* Esc O 1 ; 2 S == F16 on
				   * Terminal. */
			    retval = KEY_F(16);
			    break;
		    }
		}
		break;
	    case '5':
		if (seq_len >= 5) {
		    switch (seq[4]) {
			case 'A': /* Esc O 1 ; 5 A == Ctrl-Up on
				   * Terminal. */
			case 'B': /* Esc O 1 ; 5 B == Ctrl-Down on
				   * Terminal. */
			case 'C': /* Esc O 1 ; 5 C == Ctrl-Right on
				   * Terminal. */
			case 'D': /* Esc O 1 ; 5 D == Ctrl-Left on
				   * Terminal. */
			    retval = get_escape_seq_abcd(seq[4]);
			    break;
		    }
=======
		switch (seq[4]) {
		    case 'A': /* Esc O 1 ; 2 A == Shift-Up on Terminal. */
		    case 'B': /* Esc O 1 ; 2 B == Shift-Down on Terminal. */
		    case 'C': /* Esc O 1 ; 2 C == Shift-Right on Terminal. */
		    case 'D': /* Esc O 1 ; 2 D == Shift-Left on Terminal. */
			shift_held = TRUE;
			return arrow_from_abcd(seq[4]);
		    case 'P': /* Esc O 1 ; 2 P == F13 on Terminal. */
			return KEY_F(13);
		    case 'Q': /* Esc O 1 ; 2 Q == F14 on Terminal. */
			return KEY_F(14);
		    case 'R': /* Esc O 1 ; 2 R == F15 on Terminal. */
			return KEY_F(15);
		    case 'S': /* Esc O 1 ; 2 S == F16 on Terminal. */
			return KEY_F(16);
		}
		break;
	    case '5':
		switch (seq[4]) {
		    case 'A': /* Esc O 1 ; 5 A == Ctrl-Up on Terminal. */
			return CONTROL_UP;
		    case 'B': /* Esc O 1 ; 5 B == Ctrl-Down on Terminal. */
			return CONTROL_DOWN;
		    case 'C': /* Esc O 1 ; 5 C == Ctrl-Right on Terminal. */
			return CONTROL_RIGHT;
		    case 'D': /* Esc O 1 ; 5 D == Ctrl-Left on Terminal. */
			return CONTROL_LEFT;
>>>>>>> origin/tomato-shibby-RT-AC
		}
		break;
	}

			}
			break;
		    case '2':
			if (seq_len >= 3) {
			    switch (seq[2]) {
				case 'P': /* Esc O 2 P == F13 on
					   * xterm. */
				    retval = KEY_F(13);
				    break;
				case 'Q': /* Esc O 2 Q == F14 on
					   * xterm. */
				    retval = KEY_F(14);
				    break;
				case 'R': /* Esc O 2 R == F15 on
					   * xterm. */
				    retval = KEY_F(15);
				    break;
				case 'S': /* Esc O 2 S == F16 on
					   * xterm. */
				    retval = KEY_F(16);
				    break;
			    }
			}
			break;
		    case 'A': /* Esc O A == Up on VT100/VT320/xterm. */
		    case 'B': /* Esc O B == Down on
			       * VT100/VT320/xterm. */
		    case 'C': /* Esc O C == Right on
			       * VT100/VT320/xterm. */
		    case 'D': /* Esc O D == Left on
			       * VT100/VT320/xterm. */
			retval = get_escape_seq_abcd(seq[1]);
			break;
		    case 'E': /* Esc O E == Center (5) on numeric keypad
			       * with NumLock off on xterm. */
			retval = KEY_B2;
			break;
		    case 'F': /* Esc O F == End on xterm/Terminal. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_END, 0);
			break;
		    case 'H': /* Esc O H == Home on xterm/Terminal. */
			retval = sc_seq_or(DO_HOME, 0);;
			break;
		    case 'M': /* Esc O M == Enter on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm. */
			retval = sc_seq_or(DO_HOME, 0);;
			break;
=======
			return KEY_END;
		    case 'H': /* Esc O H == Home on xterm/Terminal. */
			return KEY_HOME;
		    case 'M': /* Esc O M == Enter on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm. */
			return KEY_ENTER;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'P': /* Esc O P == F1 on VT100/VT220/VT320/Mach
			       * console. */
			retval = KEY_F(1);
			break;
		    case 'Q': /* Esc O Q == F2 on VT100/VT220/VT320/Mach
			       * console. */
			retval = KEY_F(2);
			break;
		    case 'R': /* Esc O R == F3 on VT100/VT220/VT320/Mach
			       * console. */
			retval = KEY_F(3);
			break;
		    case 'S': /* Esc O S == F4 on VT100/VT220/VT320/Mach
			       * console. */
			retval = KEY_F(4);
			break;
		    case 'T': /* Esc O T == F5 on Mach console. */
			retval = KEY_F(5);
			break;
		    case 'U': /* Esc O U == F6 on Mach console. */
			retval = KEY_F(6);
			break;
		    case 'V': /* Esc O V == F7 on Mach console. */
			retval = KEY_F(7);
			break;
		    case 'W': /* Esc O W == F8 on Mach console. */
			retval = KEY_F(8);
			break;
		    case 'X': /* Esc O X == F9 on Mach console. */
			retval = KEY_F(9);
			break;
		    case 'Y': /* Esc O Y == F10 on Mach console. */
			retval = KEY_F(10);
			break;
		    case 'a': /* Esc O a == Ctrl-Up on rxvt. */
			return CONTROL_UP;
		    case 'b': /* Esc O b == Ctrl-Down on rxvt. */
<<<<<<< HEAD
=======
			return CONTROL_DOWN;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'c': /* Esc O c == Ctrl-Right on rxvt. */
		    case 'd': /* Esc O d == Ctrl-Left on rxvt. */
			retval = get_escape_seq_abcd(seq[1]);
			break;
		    case 'j': /* Esc O j == '*' on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm/Terminal. */
			retval = '*';
			break;
		    case 'k': /* Esc O k == '+' on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm/Terminal. */
			retval = '+';
			break;
		    case 'l': /* Esc O l == ',' on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm/Terminal. */
			retval = ',';
			break;
		    case 'm': /* Esc O m == '-' on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm/Terminal. */
			retval = '-';
			break;
		    case 'n': /* Esc O n == Delete (.) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * xterm/rxvt/Eterm/Terminal. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_DELETE, 0);;
			break;
=======
			return KEY_DC;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'o': /* Esc O o == '/' on numeric keypad with
			       * NumLock off on VT100/VT220/VT320/xterm/
			       * rxvt/Eterm/Terminal. */
			retval = '/';
			break;
		    case 'p': /* Esc O p == Insert (0) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_INSERTFILE_VOID, 0);;
			break;
		    case 'q': /* Esc O q == End (1) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_END, 0);;
			break;
		    case 'r': /* Esc O r == Down (2) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_DOWN_VOID, 0);;
			break;
		    case 's': /* Esc O s == PageDown (3) on numeric
			       * keypad with NumLock off on VT100/VT220/
			       * VT320/rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_PAGE_DOWN, 0);;
			break;
		    case 't': /* Esc O t == Left (4) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_LEFT, 0);;
			break;
=======
			return KEY_IC;
		    case 'q': /* Esc O q == End (1) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			return KEY_END;
		    case 'r': /* Esc O r == Down (2) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			return KEY_DOWN;
		    case 's': /* Esc O s == PageDown (3) on numeric
			       * keypad with NumLock off on VT100/VT220/
			       * VT320/rxvt/Eterm/Terminal. */
			return KEY_NPAGE;
		    case 't': /* Esc O t == Left (4) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			return KEY_LEFT;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'u': /* Esc O u == Center (5) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm. */
			retval = KEY_B2;
			break;
		    case 'v': /* Esc O v == Right (6) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_RIGHT, 0);
			break;
		    case 'w': /* Esc O w == Home (7) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_HOME, 0);
			break;
		    case 'x': /* Esc O x == Up (8) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_UP_VOID, 0);
			break;
		    case 'y': /* Esc O y == PageUp (9) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			retval = sc_seq_or(DO_PAGE_UP, 0);
			break;
=======
			return KEY_RIGHT;
		    case 'w': /* Esc O w == Home (7) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			return KEY_HOME;
		    case 'x': /* Esc O x == Up (8) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			return KEY_UP;
		    case 'y': /* Esc O y == PageUp (9) on numeric keypad
			       * with NumLock off on VT100/VT220/VT320/
			       * rxvt/Eterm/Terminal. */
			return KEY_PPAGE;
>>>>>>> origin/tomato-shibby-RT-AC
		}
		break;
	    case 'o':
		switch (seq[1]) {
		    case 'a': /* Esc o a == Ctrl-Up on Eterm. */
			return CONTROL_UP;
		    case 'b': /* Esc o b == Ctrl-Down on Eterm. */
<<<<<<< HEAD
=======
			return CONTROL_DOWN;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'c': /* Esc o c == Ctrl-Right on Eterm. */
		    case 'd': /* Esc o d == Ctrl-Left on Eterm. */
			retval = get_escape_seq_abcd(seq[1]);
			break;
		}
		break;
	    case '[':
		switch (seq[1]) {
		    case '1':
			if (seq_len > 3 && seq[3] == '~') {
			    switch (seq[2]) {
				case '1': /* Esc [ 1 1 ~ == F1 on rxvt/
					   * Eterm. */
				    retval = KEY_F(1);
				    break;
				case '2': /* Esc [ 1 2 ~ == F2 on rxvt/
					   * Eterm. */
				    retval = KEY_F(2);
				    break;
				case '3': /* Esc [ 1 3 ~ == F3 on rxvt/
					   * Eterm. */
				    retval = KEY_F(3);
				    break;
				case '4': /* Esc [ 1 4 ~ == F4 on rxvt/
					   * Eterm. */
				    retval = KEY_F(4);
				    break;
				case '5': /* Esc [ 1 5 ~ == F5 on xterm/
					   * rxvt/Eterm. */
				    retval = KEY_F(5);
				    break;
				case '7': /* Esc [ 1 7 ~ == F6 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
				    retval = KEY_F(6);
				    break;
				case '8': /* Esc [ 1 8 ~ == F7 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
				    retval = KEY_F(7);
				    break;
				case '9': /* Esc [ 1 9 ~ == F8 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
<<<<<<< HEAD
				    retval = KEY_F(8);
				    break;
				case ';':
    if (seq_len >= 4) {
	switch (seq[3]) {
	    case '2':
		if (seq_len >= 5) {
		    switch (seq[4]) {
			case 'A': /* Esc [ 1 ; 2 A == Shift-Up on
				   * xterm. */
			case 'B': /* Esc [ 1 ; 2 B == Shift-Down on
				   * xterm. */
			case 'C': /* Esc [ 1 ; 2 C == Shift-Right on
				   * xterm. */
			case 'D': /* Esc [ 1 ; 2 D == Shift-Left on
				   * xterm. */
			    retval = get_escape_seq_abcd(seq[4]);
			    break;
		    }
=======
				    return KEY_F(8);
			    }
			} else if (seq_len > 4 && seq[2] == ';') {

	switch (seq[3]) {
	    case '2':
		switch (seq[4]) {
		    case 'A': /* Esc [ 1 ; 2 A == Shift-Up on xterm. */
		    case 'B': /* Esc [ 1 ; 2 B == Shift-Down on xterm. */
		    case 'C': /* Esc [ 1 ; 2 C == Shift-Right on xterm. */
		    case 'D': /* Esc [ 1 ; 2 D == Shift-Left on xterm. */
			shift_held = TRUE;
			return arrow_from_abcd(seq[4]);
		}
		break;
#ifndef NANO_TINY
	    case '4':
		/* When the arrow keys are held together with Shift+Meta,
		 * act as if they are Home/End/PgUp/PgDown with Shift. */
		switch (seq[4]) {
		    case 'A': /* Esc [ 1 ; 4 A == Shift-Alt-Up on xterm. */
			return SHIFT_PAGEUP;
		    case 'B': /* Esc [ 1 ; 4 B == Shift-Alt-Down on xterm. */
			return SHIFT_PAGEDOWN;
		    case 'C': /* Esc [ 1 ; 4 C == Shift-Alt-Right on xterm. */
			return SHIFT_END;
		    case 'D': /* Esc [ 1 ; 4 D == Shift-Alt-Left on xterm. */
			return SHIFT_HOME;
>>>>>>> origin/tomato-shibby-RT-AC
		}
		break;
#endif
	    case '5':
<<<<<<< HEAD
		if (seq_len >= 5) {
		    switch (seq[4]) {
			case 'A': /* Esc [ 1 ; 5 A == Ctrl-Up on
				   * xterm. */
			case 'B': /* Esc [ 1 ; 5 B == Ctrl-Down on
				   * xterm. */
			case 'C': /* Esc [ 1 ; 5 C == Ctrl-Right on
				   * xterm. */
			case 'D': /* Esc [ 1 ; 5 D == Ctrl-Left on
				   * xterm. */
			    retval = get_escape_seq_abcd(seq[4]);
			    break;
		    }
=======
		switch (seq[4]) {
		    case 'A': /* Esc [ 1 ; 5 A == Ctrl-Up on xterm. */
			return CONTROL_UP;
		    case 'B': /* Esc [ 1 ; 5 B == Ctrl-Down on xterm. */
			return CONTROL_DOWN;
		    case 'C': /* Esc [ 1 ; 5 C == Ctrl-Right on xterm. */
			return CONTROL_RIGHT;
		    case 'D': /* Esc [ 1 ; 5 D == Ctrl-Left on xterm. */
			return CONTROL_LEFT;
		}
		break;
#ifndef NANO_TINY
	    case '6':
		switch (seq[4]) {
		    case 'A': /* Esc [ 1 ; 6 A == Shift-Ctrl-Up on xterm. */
			return shiftcontrolup;
		    case 'B': /* Esc [ 1 ; 6 B == Shift-Ctrl-Down on xterm. */
			return shiftcontroldown;
		    case 'C': /* Esc [ 1 ; 6 C == Shift-Ctrl-Right on xterm. */
			return shiftcontrolright;
		    case 'D': /* Esc [ 1 ; 6 D == Shift-Ctrl-Left on xterm. */
			return shiftcontrolleft;
>>>>>>> origin/tomato-shibby-RT-AC
		}
		break;
#endif
	}
<<<<<<< HEAD
    }
				    break;
				default: /* Esc [ 1 ~ == Home on
					  * VT320/Linux console. */
				    retval = sc_seq_or(DO_HOME, 0);;
				    break;
			    }
			}
=======

			} else if (seq_len > 2 && seq[2] == '~')
			    /* Esc [ 1 ~ == Home on VT320/Linux console. */
			    return KEY_HOME;
>>>>>>> origin/tomato-shibby-RT-AC
			break;
		    case '2':
			if (seq_len > 3 && seq[3] == '~') {
			    switch (seq[2]) {
<<<<<<< HEAD
				case '0': /* Esc [ 2 0 ~ == F9 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
				    retval = KEY_F(9);
				    break;
				case '1': /* Esc [ 2 1 ~ == F10 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
				    retval = KEY_F(10);
				    break;
				case '3': /* Esc [ 2 3 ~ == F11 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
				    retval = KEY_F(11);
				    break;
				case '4': /* Esc [ 2 4 ~ == F12 on
					   * VT220/VT320/Linux console/
					   * xterm/rxvt/Eterm. */
				    retval = KEY_F(12);
				    break;
				case '5': /* Esc [ 2 5 ~ == F13 on
					   * VT220/VT320/Linux console/
					   * rxvt/Eterm. */
				    retval = KEY_F(13);
				    break;
				case '6': /* Esc [ 2 6 ~ == F14 on
					   * VT220/VT320/Linux console/
					   * rxvt/Eterm. */
				    retval = KEY_F(14);
				    break;
				case '8': /* Esc [ 2 8 ~ == F15 on
					   * VT220/VT320/Linux console/
					   * rxvt/Eterm. */
				    retval = KEY_F(15);
				    break;
				case '9': /* Esc [ 2 9 ~ == F16 on
					   * VT220/VT320/Linux console/
					   * rxvt/Eterm. */
				    retval = KEY_F(16);
				    break;
				default: /* Esc [ 2 ~ == Insert on
					  * VT220/VT320/Linux console/
					  * xterm/Terminal. */
				    retval = sc_seq_or(DO_INSERTFILE_VOID, 0);;
				    break;
=======
				case '0': /* Esc [ 2 0 ~ == F9 on VT220/VT320/
					   * Linux console/xterm/rxvt/Eterm. */
				    return KEY_F(9);
				case '1': /* Esc [ 2 1 ~ == F10 on VT220/VT320/
					   * Linux console/xterm/rxvt/Eterm. */
				    return KEY_F(10);
				case '3': /* Esc [ 2 3 ~ == F11 on VT220/VT320/
					   * Linux console/xterm/rxvt/Eterm. */
				    return KEY_F(11);
				case '4': /* Esc [ 2 4 ~ == F12 on VT220/VT320/
					   * Linux console/xterm/rxvt/Eterm. */
				    return KEY_F(12);
				case '5': /* Esc [ 2 5 ~ == F13 on VT220/VT320/
					   * Linux console/rxvt/Eterm. */
				    return KEY_F(13);
				case '6': /* Esc [ 2 6 ~ == F14 on VT220/VT320/
					   * Linux console/rxvt/Eterm. */
				    return KEY_F(14);
				case '8': /* Esc [ 2 8 ~ == F15 on VT220/VT320/
					   * Linux console/rxvt/Eterm. */
				    return KEY_F(15);
				case '9': /* Esc [ 2 9 ~ == F16 on VT220/VT320/
					   * Linux console/rxvt/Eterm. */
				    return KEY_F(16);
>>>>>>> origin/tomato-shibby-RT-AC
			    }
			} else if (seq_len > 2 && seq[2] == '~')
			    /* Esc [ 2 ~ == Insert on VT220/VT320/
			     * Linux console/xterm/Terminal. */
			    return KEY_IC;
			break;
		    case '3': /* Esc [ 3 ~ == Delete on VT220/VT320/
			       * Linux console/xterm/Terminal. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_DELETE, 0);;
			break;
		    case '4': /* Esc [ 4 ~ == End on VT220/VT320/Linux
			       * console/xterm. */
			retval = sc_seq_or(DO_END, 0);;
=======
			if (seq_len > 2 && seq[2] == '~')
			    return KEY_DC;
			break;
		    case '4': /* Esc [ 4 ~ == End on VT220/VT320/Linux
			       * console/xterm. */
			if (seq_len > 2 && seq[2] == '~')
			    return KEY_END;
>>>>>>> origin/tomato-shibby-RT-AC
			break;
		    case '5': /* Esc [ 5 ~ == PageUp on VT220/VT320/
			       * Linux console/xterm/Terminal;
			       * Esc [ 5 ^ == PageUp on Eterm. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_PAGE_UP, 0);;
			break;
		    case '6': /* Esc [ 6 ~ == PageDown on VT220/VT320/
			       * Linux console/xterm/Terminal;
			        * Esc [ 6 ^ == PageDown on Eterm. */
			retval = sc_seq_or(DO_PAGE_DOWN, 0);;
			break;
		    case '7': /* Esc [ 7 ~ == Home on rxvt. */
			retval = sc_seq_or(DO_HOME, 0);
			break;
		    case '8': /* Esc [ 8 ~ == End on rxvt. */
			retval = sc_seq_or(DO_END, 0);
			break;
		    case '9': /* Esc [ 9 == Delete on Mach console. */
			retval = sc_seq_or(DO_DELETE, 0);;
			break;
		    case '@': /* Esc [ @ == Insert on Mach console. */
			retval = sc_seq_or(DO_INSERTFILE_VOID, 0);;
			break;
=======
			if (seq_len > 2 && (seq[2] == '~' || seq[2] == '^'))
			    return KEY_PPAGE;
			break;
		    case '6': /* Esc [ 6 ~ == PageDown on VT220/VT320/
			       * Linux console/xterm/Terminal;
			       * Esc [ 6 ^ == PageDown on Eterm. */
			if (seq_len > 2 && (seq[2] == '~' || seq[2] == '^'))
			    return KEY_NPAGE;
			break;
		    case '7': /* Esc [ 7 ~ == Home on Eterm/rxvt,
			       * Esc [ 7 $ == Shift-Home on Eterm/rxvt. */
			if (seq_len > 2 && seq[2] == '~')
			    return KEY_HOME;
			else if (seq_len > 2 && seq[2] == '$')
			    return SHIFT_HOME;
			break;
		    case '8': /* Esc [ 8 ~ == End on Eterm/rxvt.
			       * Esc [ 8 $ == Shift-End on Eterm/rxvt. */
			if (seq_len > 2 && seq[2] == '~')
			    return KEY_END;
			else if (seq_len > 2 && seq[2] == '$')
			    return SHIFT_END;
			break;
		    case '9': /* Esc [ 9 == Delete on Mach console. */
			return KEY_DC;
		    case '@': /* Esc [ @ == Insert on Mach console. */
			return KEY_IC;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'A': /* Esc [ A == Up on ANSI/VT220/Linux
			       * console/FreeBSD console/Mach console/
			       * rxvt/Eterm/Terminal. */
		    case 'B': /* Esc [ B == Down on ANSI/VT220/Linux
			       * console/FreeBSD console/Mach console/
			       * rxvt/Eterm/Terminal. */
		    case 'C': /* Esc [ C == Right on ANSI/VT220/Linux
			       * console/FreeBSD console/Mach console/
			       * rxvt/Eterm/Terminal. */
		    case 'D': /* Esc [ D == Left on ANSI/VT220/Linux
			       * console/FreeBSD console/Mach console/
			       * rxvt/Eterm/Terminal. */
			retval = get_escape_seq_abcd(seq[1]);
			break;
		    case 'E': /* Esc [ E == Center (5) on numeric keypad
			       * with NumLock off on FreeBSD console/
			       * Terminal. */
<<<<<<< HEAD
			retval = KEY_B2;
			break;
		    case 'F': /* Esc [ F == End on FreeBSD
			       * console/Eterm. */
			retval = sc_seq_or(DO_END, 0);
			break;
		    case 'G': /* Esc [ G == PageDown on FreeBSD
			       * console. */
			retval = sc_seq_or(DO_PAGE_DOWN, 0);
			break;
		    case 'H': /* Esc [ H == Home on ANSI/VT220/FreeBSD
			       * console/Mach console/Eterm. */
			retval = sc_seq_or(DO_HOME, 0);
			break;
		    case 'I': /* Esc [ I == PageUp on FreeBSD
			       * console. */
			retval = sc_seq_or(DO_PAGE_UP, 0);
			break;
		    case 'L': /* Esc [ L == Insert on ANSI/FreeBSD
			       * console. */
			retval = sc_seq_or(DO_INSERTFILE_VOID, 0);
			break;
=======
			return KEY_B2;
		    case 'F': /* Esc [ F == End on FreeBSD console/Eterm. */
			return KEY_END;
		    case 'G': /* Esc [ G == PageDown on FreeBSD console. */
			return KEY_NPAGE;
		    case 'H': /* Esc [ H == Home on ANSI/VT220/FreeBSD
			       * console/Mach console/Eterm. */
			return KEY_HOME;
		    case 'I': /* Esc [ I == PageUp on FreeBSD console. */
			return KEY_PPAGE;
		    case 'L': /* Esc [ L == Insert on ANSI/FreeBSD console. */
			return KEY_IC;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'M': /* Esc [ M == F1 on FreeBSD console. */
			retval = KEY_F(1);
			break;
		    case 'N': /* Esc [ N == F2 on FreeBSD console. */
			retval = KEY_F(2);
			break;
		    case 'O':
			if (seq_len > 2) {
			    switch (seq[2]) {
				case 'P': /* Esc [ O P == F1 on
					   * xterm. */
				    retval = KEY_F(1);
				    break;
				case 'Q': /* Esc [ O Q == F2 on
					   * xterm. */
				    retval = KEY_F(2);
				    break;
				case 'R': /* Esc [ O R == F3 on
					   * xterm. */
				    retval = KEY_F(3);
				    break;
				case 'S': /* Esc [ O S == F4 on
					   * xterm. */
				    retval = KEY_F(4);
				    break;
			    }
			} else
			    /* Esc [ O == F3 on FreeBSD console. */
			    retval = KEY_F(3);
			break;
		    case 'P': /* Esc [ P == F4 on FreeBSD console. */
			retval = KEY_F(4);
			break;
		    case 'Q': /* Esc [ Q == F5 on FreeBSD console. */
			retval = KEY_F(5);
			break;
		    case 'R': /* Esc [ R == F6 on FreeBSD console. */
			retval = KEY_F(6);
			break;
		    case 'S': /* Esc [ S == F7 on FreeBSD console. */
			retval = KEY_F(7);
			break;
		    case 'T': /* Esc [ T == F8 on FreeBSD console. */
			retval = KEY_F(8);
			break;
		    case 'U': /* Esc [ U == PageDown on Mach console. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_PAGE_DOWN, 0);
			break;
		    case 'V': /* Esc [ V == PageUp on Mach console. */
			retval = sc_seq_or(DO_PAGE_UP, 0);
			break;
=======
			return KEY_NPAGE;
		    case 'V': /* Esc [ V == PageUp on Mach console. */
			return KEY_PPAGE;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'W': /* Esc [ W == F11 on FreeBSD console. */
			retval = KEY_F(11);
			break;
		    case 'X': /* Esc [ X == F12 on FreeBSD console. */
			retval = KEY_F(12);
			break;
		    case 'Y': /* Esc [ Y == End on Mach console. */
<<<<<<< HEAD
			retval = sc_seq_or(DO_END, 0);
			break;
=======
			return KEY_END;
>>>>>>> origin/tomato-shibby-RT-AC
		    case 'Z': /* Esc [ Z == F14 on FreeBSD console. */
			retval = KEY_F(14);
			break;
		    case 'a': /* Esc [ a == Shift-Up on rxvt/Eterm. */
		    case 'b': /* Esc [ b == Shift-Down on rxvt/Eterm. */
		    case 'c': /* Esc [ c == Shift-Right on rxvt/
			       * Eterm. */
		    case 'd': /* Esc [ d == Shift-Left on rxvt/Eterm. */
<<<<<<< HEAD
			retval = get_escape_seq_abcd(seq[1]);
			break;
=======
			shift_held = TRUE;
			return arrow_from_abcd(seq[1]);
>>>>>>> origin/tomato-shibby-RT-AC
		    case '[':
			if (seq_len > 2 ) {
			    switch (seq[2]) {
				case 'A': /* Esc [ [ A == F1 on Linux
					   * console. */
				    retval = KEY_F(1);
				    break;
				case 'B': /* Esc [ [ B == F2 on Linux
					   * console. */
				    retval = KEY_F(2);
				    break;
				case 'C': /* Esc [ [ C == F3 on Linux
					   * console. */
				    retval = KEY_F(3);
				    break;
				case 'D': /* Esc [ [ D == F4 on Linux
					   * console. */
				    retval = KEY_F(4);
				    break;
				case 'E': /* Esc [ [ E == F5 on Linux
					   * console. */
				    retval = KEY_F(5);
				    break;
			    }
			}
			break;
		}
		break;
	}
    }

#ifdef DEBUG
    fprintf(stderr, "get_escape_seq_kbinput(): retval = %d\n", retval);
#endif

    return retval;
}

<<<<<<< HEAD
/* Return the equivalent arrow key value for the case-insensitive
 * letters A (up), B (down), C (right), and D (left).  These are common
 * to many escape sequences. */
int get_escape_seq_abcd(int kbinput)
{
    switch (tolower(kbinput)) {
	case 'a':
	    return sc_seq_or(DO_UP_VOID, 0);;
	case 'b':
	    return sc_seq_or(DO_DOWN_VOID, 0);;
	case 'c':
	    return sc_seq_or(DO_RIGHT, 0);;
	case 'd':
	    return sc_seq_or(DO_LEFT, 0);;
=======
/* Return the equivalent arrow-key value for the first four letters
 * in the alphabet, common to many escape sequences. */
int arrow_from_abcd(int kbinput)
{
    switch (tolower(kbinput)) {
	case 'a':
	    return KEY_UP;
	case 'b':
	    return KEY_DOWN;
	case 'c':
	    return KEY_RIGHT;
	case 'd':
	    return KEY_LEFT;
>>>>>>> origin/tomato-shibby-RT-AC
	default:
	    return ERR;
    }
}

/* Interpret the escape sequence in the keystroke buffer, the first
 * character of which is kbinput.  Assume that the keystroke buffer
 * isn't empty, and that the initial escape has already been read in. */
int parse_escape_seq_kbinput(WINDOW *win, int kbinput)
{
    int retval, *seq;
    size_t seq_len;

    /* Put back the non-escape character, get the complete escape
     * sequence, translate the sequence into its corresponding key
     * value, and save that as the result. */
    unget_input(&kbinput, 1);
    seq_len = key_buffer_len;
    seq = get_input(NULL, seq_len);
    retval = get_escape_seq_kbinput(seq, seq_len);

    free(seq);

    /* If we got an unrecognized escape sequence, throw it out. */
    if (retval == ERR) {
	if (win == edit) {
	    statusbar(_("Unknown Command"));
	    beep();
	}
    }

#ifdef DEBUG
    fprintf(stderr, "parse_escape_seq_kbinput(): kbinput = %d, seq_len = %lu, retval = %d\n", kbinput, (unsigned long)seq_len, retval);
#endif

    return retval;
}

/* Translate a byte sequence: turn a three-digit decimal number (from
 * 000 to 255) into its corresponding byte value. */
int get_byte_kbinput(int kbinput)
{
    static int byte_digits = 0, byte = 0;
    int retval = ERR;

    /* Increment the byte digit counter. */
    byte_digits++;

    switch (byte_digits) {
	case 1:
	    /* First digit: This must be from zero to two.  Put it in
	     * the 100's position of the byte sequence holder. */
	    if ('0' <= kbinput && kbinput <= '2')
		byte = (kbinput - '0') * 100;
	    else
		/* This isn't the start of a byte sequence.  Return this
		 * character as the result. */
		retval = kbinput;
	    break;
	case 2:
	    /* Second digit: This must be from zero to five if the first
	     * was two, and may be any decimal value if the first was
	     * zero or one.  Put it in the 10's position of the byte
	     * sequence holder. */
	    if (('0' <= kbinput && kbinput <= '5') || (byte < 200 &&
		'6' <= kbinput && kbinput <= '9'))
		byte += (kbinput - '0') * 10;
	    else
		/* This isn't the second digit of a byte sequence.
		 * Return this character as the result. */
		retval = kbinput;
	    break;
	case 3:
	    /* Third digit: This must be from zero to five if the first
	     * was two and the second was between zero and five, and may
	     * be any decimal value if the first was zero or one and the
	     * second was between six and nine.  Put it in the 1's
	     * position of the byte sequence holder. */
	    if (('0' <= kbinput && kbinput <= '5') || (byte < 250 &&
		'6' <= kbinput && kbinput <= '9')) {
		byte += kbinput - '0';
		/* If this character is a valid decimal value, then the
		 * byte sequence is complete. */
		retval = byte;
	    } else
		/* This isn't the third digit of a byte sequence.
		 * Return this character as the result. */
		retval = kbinput;
	    break;
	default:
	    /* If there are more than three digits, return this
	     * character as the result.  (Maybe we should produce an
	     * error instead?) */
	    retval = kbinput;
	    break;
    }

    /* If we have a result, reset the byte digit counter and the byte
     * sequence holder. */
    if (retval != ERR) {
	byte_digits = 0;
	byte = 0;
    }

#ifdef DEBUG
    fprintf(stderr, "get_byte_kbinput(): kbinput = %d, byte_digits = %d, byte = %d, retval = %d\n", kbinput, byte_digits, byte, retval);
#endif

    return retval;
}

#ifdef ENABLE_UTF8
/* If the character in kbinput is a valid hexadecimal digit, multiply it
 * by factor and add the result to uni. */
long add_unicode_digit(int kbinput, long factor, long *uni)
{
    long retval = ERR;

    if ('0' <= kbinput && kbinput <= '9')
	*uni += (kbinput - '0') * factor;
    else if ('a' <= tolower(kbinput) && tolower(kbinput) <= 'f')
	*uni += (tolower(kbinput) - 'a' + 10) * factor;
    else
	/* If this character isn't a valid hexadecimal value, save it as
	 * the result. */
	retval = kbinput;

    return retval;
}

/* Translate a Unicode sequence: turn a six-digit hexadecimal number
 * (from 000000 to 10FFFF, case-insensitive) into its corresponding
 * multibyte value. */
long get_unicode_kbinput(WINDOW *win, int kbinput)
{
    static int uni_digits = 0;
    static long uni = 0;
    long retval = ERR;

    /* Increment the Unicode digit counter. */
    uni_digits++;

    switch (uni_digits) {
	case 1:
	    /* The first digit must be zero or one.  Put it in the
	     * 0x100000's position of the Unicode sequence holder.
	     * Otherwise, return the character itself as the result. */
	    if (kbinput == '0' || kbinput == '1')
		uni = (kbinput - '0') * 0x100000;
	    else
		retval = kbinput;
	    break;
	case 2:
	    /* The second digit must be zero if the first was one, but
	     * may be any hexadecimal value if the first was zero. */
	    if (kbinput == '0' || uni == 0)
		retval = add_unicode_digit(kbinput, 0x10000, &uni);
	    else
		retval = kbinput;
	    break;
	case 3:
	    /* Later digits may be any hexadecimal value. */
	    retval = add_unicode_digit(kbinput, 0x1000, &uni);
	    break;
	case 4:
	    retval = add_unicode_digit(kbinput, 0x100, &uni);
	    break;
	case 5:
	    retval = add_unicode_digit(kbinput, 0x10, &uni);
	    break;
	case 6:
	    retval = add_unicode_digit(kbinput, 0x1, &uni);
	    /* If also the sixth digit was a valid hexadecimal value, then
	     * the Unicode sequence is complete, so return it. */
	    if (retval == ERR)
		retval = uni;
	    break;
	default:
	    /* If there are more than six digits, return this character
	     * as the result.  (Maybe we should produce an error
	     * instead?) */
	    retval = kbinput;
	    break;
    }

    /* Show feedback only when editing, not when at a prompt. */
    if (retval == ERR && win == edit) {
	char partial[7] = "......";

	/* Construct the partial result, right-padding it with dots. */
	snprintf(partial, uni_digits + 1, "%06lX", uni);
	partial[uni_digits] = '.';

	/* TRANSLATORS: This is shown while a six-digit hexadecimal
	 * Unicode character code (%s) is being typed in. */
	statusline(HUSH, _("Unicode Input: %s"), partial);
    }

#ifdef DEBUG
    fprintf(stderr, "get_unicode_kbinput(): kbinput = %d, uni_digits = %d, uni = %ld, retval = %ld\n",
						kbinput, uni_digits, uni, retval);
#endif

    /* If we have an end result, reset the Unicode digit counter. */
    if (retval != ERR)
	uni_digits = 0;

    return retval;
}
#endif /* ENABLE_UTF8 */

/* Translate a control character sequence: turn an ASCII non-control
 * character into its corresponding control character. */
int get_control_kbinput(int kbinput)
{
    int retval;

     /* Ctrl-Space (Ctrl-2, Ctrl-@, Ctrl-`) */
    if (kbinput == ' ' || kbinput == '2')
	retval = 0;
    /* Ctrl-/ (Ctrl-7, Ctrl-_) */
    else if (kbinput == '/')
	retval = 31;
    /* Ctrl-3 (Ctrl-[, Esc) to Ctrl-7 (Ctrl-/, Ctrl-_) */
    else if ('3' <= kbinput && kbinput <= '7')
	retval = kbinput - 24;
    /* Ctrl-8 (Ctrl-?) */
    else if (kbinput == '8' || kbinput == '?')
	retval = DEL_CODE;
    /* Ctrl-@ (Ctrl-Space, Ctrl-2, Ctrl-`) to Ctrl-_ (Ctrl-/, Ctrl-7) */
    else if ('@' <= kbinput && kbinput <= '_')
	retval = kbinput - '@';
    /* Ctrl-` (Ctrl-2, Ctrl-Space, Ctrl-@) to Ctrl-~ (Ctrl-6, Ctrl-^) */
    else if ('`' <= kbinput && kbinput <= '~')
	retval = kbinput - '`';
    else
	retval = kbinput;

#ifdef DEBUG
    fprintf(stderr, "get_control_kbinput(): kbinput = %d, retval = %d\n", kbinput, retval);
#endif

    return retval;
}

/* Put the output-formatted characters in output back into the keystroke
 * buffer, so that they can be parsed and displayed as output again. */
void unparse_kbinput(char *output, size_t output_len)
{
    int *input;
    size_t i;

    if (output_len == 0)
	return;

    input = (int *)nmalloc(output_len * sizeof(int));

    for (i = 0; i < output_len; i++)
	input[i] = (int)output[i];

    unget_input(input, output_len);

    free(input);
}

/* Read in a stream of characters verbatim, and return the length of the
 * string in kbinput_len.  Assume nodelay(win) is FALSE. */
int *get_verbatim_kbinput(WINDOW *win, size_t *kbinput_len)
{
    int *retval;

    /* Turn off flow control characters if necessary so that we can type
     * them in verbatim, and turn the keypad off if necessary so that we
     * don't get extended keypad values. */
    if (ISSET(PRESERVE))
	disable_flow_control();
    if (!ISSET(REBIND_KEYPAD))
	keypad(win, FALSE);

    /* Read in one keycode, or one or two escapes. */
    retval = parse_verbatim_kbinput(win, kbinput_len);

    /* If the code is invalid in the current mode, discard it. */
    if ((*retval == '\n' && as_an_at) || (*retval == '\0' && !as_an_at)) {
	*kbinput_len = 0;
	beep();
    }

    /* Turn flow control characters back on if necessary and turn the
     * keypad back on if necessary now that we're done. */
    if (ISSET(PRESERVE))
	enable_flow_control();
    /* Use the global window pointers, because a resize may have freed
     * the data that the win parameter points to. */
    if (!ISSET(REBIND_KEYPAD)) {
	keypad(edit, TRUE);
	keypad(bottomwin, TRUE);
    }

    return retval;
}

/* Read in one control character (or an iTerm/Eterm/rxvt double Escape),
 * or convert a series of six digits into a Unicode codepoint.  Return
 * in count either 1 (for a control character or the first byte of a
 * multibyte sequence), or 2 (for an iTerm/Eterm/rxvt double Escape). */
int *parse_verbatim_kbinput(WINDOW *win, size_t *count)
{
    int *kbinput;

<<<<<<< HEAD
    /* Read in the first keystroke. */
    while ((kbinput = get_input(win, 1)) == NULL);
=======
    /* Read in the first code. */
    while ((kbinput = get_input(win, 1)) == NULL)
	;
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef NANO_TINY
    /* When the window was resized, abort and return nothing. */
    if (*kbinput == KEY_WINCH) {
	free(kbinput);
	*count = 0;
	return NULL;
    }
#endif

    *count = 1;

#ifdef ENABLE_UTF8
    if (using_utf8()) {
	/* Check whether the first code is a valid starter digit: 0 or 1. */
	long unicode = get_unicode_kbinput(win, *kbinput);

	/* If the first code isn't the digit 0 nor 1, put it back. */
	if (unicode != ERR)
	    unget_input(kbinput, 1);
	/* Otherwise, continue reading in digits until we have a complete
	 * Unicode value, and put back the corresponding byte(s). */
	else {
	    char *multibyte;
	    int onebyte, i;

<<<<<<< HEAD
	    while (uni == ERR) {
		while ((kbinput = get_input(win, 1)) == NULL);

		uni = get_unicode_kbinput(*kbinput);
=======
	    while (unicode == ERR) {
		free(kbinput);
		while ((kbinput = get_input(win, 1)) == NULL)
		    ;
		unicode = get_unicode_kbinput(win, *kbinput);
>>>>>>> origin/tomato-shibby-RT-AC
	    }

	    /* Convert the Unicode value to a multibyte sequence. */
	    multibyte = make_mbchar(unicode, (int *)count);

	    /* Insert the multibyte sequence into the input buffer. */
	    for (i = *count; i > 0 ; i--) {
		onebyte = (unsigned char)multibyte[i - 1];
		unget_input(&onebyte, 1);
	    }

	    free(multibyte);
	}
    } else
#endif /* ENABLE_UTF8 */
	/* Put back the first code. */
	unget_input(kbinput, 1);

    free(kbinput);

    /* If this is an iTerm/Eterm/rxvt double escape, take both Escapes. */
    if (key_buffer_len > 3 && *key_buffer == ESC_CODE &&
		key_buffer[1] == ESC_CODE && key_buffer[2] == '[')
	*count = 2;

    return get_input(NULL, *count);
}

#ifndef DISABLE_MOUSE
/* Handle any mouse event that may have occurred.  We currently handle
 * releases/clicks of the first mouse button.  If allow_shortcuts is
 * TRUE, releasing/clicking on a visible shortcut will put back the
 * keystroke associated with that shortcut.  If NCURSES_MOUSE_VERSION is
 * at least 2, we also currently handle presses of the fourth mouse
 * button (upward rolls of the mouse wheel) by putting back the
 * keystrokes to move up, and presses of the fifth mouse button
 * (downward rolls of the mouse wheel) by putting back the keystrokes to
 * move down.  We also store the coordinates of a mouse event that needs
 * to be handled in mouse_x and mouse_y, relative to the entire screen.
 * Return -1 on error, 0 if the mouse event needs to be handled, 1 if
 * it's been handled by putting back keystrokes that need to be handled.
 * or 2 if it's been ignored.  Assume that KEY_MOUSE has already been
 * read in. */
int get_mouseinput(int *mouse_x, int *mouse_y, bool allow_shortcuts)
{
    MEVENT mevent;
    bool in_bottomwin;
    subnfunc *f;

    *mouse_x = -1;
    *mouse_y = -1;

    /* First, get the actual mouse event. */
    if (getmouse(&mevent) == ERR)
	return -1;

    /* Save the screen coordinates where the mouse event took place. */
    *mouse_x = mevent.x - margin;
    *mouse_y = mevent.y;

    in_bottomwin = wenclose(bottomwin, *mouse_y, *mouse_x);

    /* Handle releases/clicks of the first mouse button. */
    if (mevent.bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED)) {
	/* If we're allowing shortcuts, the current shortcut list is
	 * being displayed on the last two lines of the screen, and the
	 * first mouse button was released on/clicked inside it, we need
	 * to figure out which shortcut was released on/clicked and put
	 * back the equivalent keystroke(s) for it. */
	if (allow_shortcuts && !ISSET(NO_HELP) && in_bottomwin) {
	    int i;
		/* The width of all the shortcuts, except for the last
		 * two, in the shortcut list in bottomwin. */
	    int j;
<<<<<<< HEAD
		/* The y-coordinate relative to the beginning of the
		 * shortcut list in bottomwin. */
	    size_t currslen;
		/* The number of shortcuts in the current shortcut
		 * list. */
=======
		/* The calculated index number of the clicked item. */
	    size_t number;
		/* The number of available shortcuts in the current menu. */
>>>>>>> origin/tomato-shibby-RT-AC

	    /* Translate the mouse event coordinates so that they're
	     * relative to bottomwin. */
	    wmouse_trafo(bottomwin, mouse_y, mouse_x, FALSE);

	    /* Handle releases/clicks of the first mouse button on the
	     * statusbar elsewhere. */
	    if (*mouse_y == 0) {
		/* Restore the untranslated mouse event coordinates, so
		 * that they're relative to the entire screen again. */
		*mouse_x = mevent.x - margin;
		*mouse_y = mevent.y;

		return 0;
	    }

<<<<<<< HEAD
	    /* Calculate the y-coordinate relative to the beginning of
	     * the shortcut list in bottomwin. */
	    j = *mouse_y - 1;

	    /* Get the shortcut lists' length. */
	    if (currmenu == MMAIN)
		currslen = MAIN_VISIBLE;
	    else {
		currslen = length_of_list(currmenu);
=======
	    /* Determine how many shortcuts are being shown. */
	    number = length_of_list(currmenu);
>>>>>>> origin/tomato-shibby-RT-AC

	    if (number > MAIN_VISIBLE)
		number = MAIN_VISIBLE;

	    /* Calculate the width of all of the shortcuts in the list
	     * except for the last two, which are longer by (COLS % i)
	     * columns so as to not waste space. */
	    if (number < 2)
		i = COLS / (MAIN_VISIBLE / 2);
	    else
		i = COLS / ((number / 2) + (number % 2));

	    /* Calculate the x-coordinate relative to the beginning of
	     * the shortcut list in bottomwin, and add it to j.  j
	     * should now be the index in the shortcut list of the
	     * shortcut we released/clicked on. */
	    j = (*mouse_x / i) * 2 + j;

<<<<<<< HEAD
	    /* Adjust j if we released on the last two shortcuts. */
	    if ((j >= currslen) && (*mouse_x % i < COLS % i))
=======
	    /* Adjust the index if we hit the last two wider ones. */
	    if ((j > number) && (*mouse_x % i < COLS % i))
>>>>>>> origin/tomato-shibby-RT-AC
		j -= 2;

	    /* Ignore releases/clicks of the first mouse button beyond
	     * the last shortcut. */
<<<<<<< HEAD
	    if (j >= currslen)
=======
	    if (j > number)
>>>>>>> origin/tomato-shibby-RT-AC
		return 2;

	    /* Go through the shortcut list to determine which shortcut
	     * we released/clicked on. */
	    f = allfuncs;

	    for (; j > 0; j--) {
		if (f->next != NULL)
		    f = f->next;

                while (f->next != NULL && ((f->menus & currmenu) == 0
#ifndef DISABLE_HELP
                       || strlen(f->help) == 0
#endif
		))
		     f = f->next;
	    }


	    /* And put back the equivalent key. */
	    if (f != NULL) {
<<<<<<< HEAD
                const sc *s = first_sc_for(currmenu, f->scfunc);
		if (s != NULL)
		    unget_kbinput(s->seq, s->type == META, FALSE);
=======
		const sc *s = first_sc_for(currmenu, f->scfunc);
		unget_kbinput(s->keycode, s->meta);
>>>>>>> origin/tomato-shibby-RT-AC
	    }
	} else
	    /* Handle releases/clicks of the first mouse button that
	     * aren't on the current shortcut list elsewhere. */
	    return 0;
    }
#if NCURSES_MOUSE_VERSION >= 2
    /* Handle presses of the fourth mouse button (upward rolls of the
     * mouse wheel) and presses of the fifth mouse button (downward
     * rolls of the mouse wheel) . */
    else if (mevent.bstate & (BUTTON4_PRESSED | BUTTON5_PRESSED)) {
	bool in_edit = wenclose(edit, *mouse_y, *mouse_x);

	if (in_bottomwin)
	    /* Translate the mouse event coordinates so that they're
	     * relative to bottomwin. */
	    wmouse_trafo(bottomwin, mouse_y, mouse_x, FALSE);

	if (in_edit || (in_bottomwin && *mouse_y == 0)) {
	    int i;

	    /* One upward roll of the mouse wheel is equivalent to
	     * moving up three lines, and one downward roll of the mouse
	     * wheel is equivalent to moving down three lines. */
	    for (i = 0; i < 3; i++)
		unget_kbinput((mevent.bstate & BUTTON4_PRESSED) ?
<<<<<<< HEAD
			 sc_seq_or(DO_UP_VOID, 0) : sc_seq_or(DO_DOWN_VOID, 0), FALSE,
			FALSE);
=======
				KEY_PPAGE : KEY_NPAGE, FALSE);
>>>>>>> origin/tomato-shibby-RT-AC

	    return 1;
	} else
	    /* Ignore presses of the fourth mouse button and presses of
	     * the fifth mouse buttons that aren't on the edit window or
	     * the statusbar. */
	    return 2;
    }
#endif

    /* Ignore all other mouse events. */
    return 2;
}
#endif /* !DISABLE_MOUSE */

/* Return the shortcut corresponding to the values of kbinput (the key
 * itself), meta_key (whether the key is a meta sequence), and func_key
 * (whether the key is a function key), if any.  The shortcut will be
 * the first one in the list (control key, meta key sequence, function
 * key, other meta key sequence) for the corresponding function.  For
 * example, passing in a meta key sequence that corresponds to a
 * function with a control key, a function key, and a meta key sequence
 * will return the control key corresponding to that function. */
const sc *get_shortcut(int menu, int *kbinput, bool
	*meta_key, bool *func_key)
{
    sc *s;

#ifdef DEBUG
<<<<<<< HEAD
    fprintf(stderr, "get_shortcut(): kbinput = %d, meta_key = %s, func_key = %s\n", *kbinput, *meta_key ? "TRUE" : "FALSE", *func_key ? "TRUE" : "FALSE");
=======
    fprintf(stderr, "get_shortcut(): kbinput = %d, meta_key = %s -- ",
				*kbinput, meta_key ? "TRUE" : "FALSE");
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    /* Check for shortcuts. */
    for (s = sclist; s != NULL; s = s->next) {
<<<<<<< HEAD
        if ((menu & s->menu)
		&& ((s->type == META && *meta_key == TRUE && *kbinput == s->seq)
		|| (s->type != META && *kbinput == s->seq))) {
#ifdef DEBUG
	    fprintf (stderr, "matched seq \"%s\" and btw meta was %d (menus %d = %d)\n", s->keystr, *meta_key, menu, s->menu);
=======
	if ((s->menus & currmenu) && *kbinput == s->keycode &&
					meta_key == s->meta) {
#ifdef DEBUG
	    fprintf (stderr, "matched seq '%s'  (menu is %x from %x)\n",
				s->keystr, currmenu, s->menus);
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	    return s;
	}
    }
#ifdef DEBUG
<<<<<<< HEAD
    fprintf (stderr, "matched nothing btw meta was %d\n", *meta_key);
=======
    fprintf (stderr, "matched nothing\n");
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    return NULL;
}


/* Try to get a function back from a window.  Just a wrapper so
   functions to need to create function_key meta_key blah blah 
    mmenu - what menu name to look through for valid funcs */
const subnfunc *getfuncfromkey(WINDOW *win)
{
    int kbinput;
    bool func_key = FALSE, meta_key = FALSE;
    const sc *s;
    const subnfunc *f;

    kbinput = parse_kbinput(win, &meta_key, &func_key);
    if (kbinput == 0)
	return NULL;

    s = get_shortcut(currmenu, &kbinput, &meta_key, &func_key);
    if (!s)
	return NULL;

    f = sctofunc((sc *) s);
    return f;

}



/* Move to (x, y) in win, and display a line of n spaces with the
 * current attributes. */
void blank_row(WINDOW *win, int y, int x, int n)
{
    wmove(win, y, x);

    for (; n > 0; n--)
	waddch(win, ' ');
}

/* Blank the first line of the top portion of the window. */
void blank_titlebar(void)
{
    blank_row(topwin, 0, 0, COLS);
}

/* Blank all the lines of the middle portion of the window, i.e. the
 * edit window. */
void blank_edit(void)
{
    int row;

    for (row = 0; row < editwinrows; row++)
	blank_row(edit, row, 0, COLS);
}

/* Blank the first line of the bottom portion of the window. */
void blank_statusbar(void)
{
    blank_row(bottomwin, 0, 0, COLS);
}

/* If the NO_HELP flag isn't set, blank the last two lines of the bottom
 * portion of the window. */
void blank_bottombars(void)
{
    if (!ISSET(NO_HELP) && LINES > 4) {
	blank_row(bottomwin, 1, 0, COLS);
	blank_row(bottomwin, 2, 0, COLS);
    }
}

/* Check if the number of keystrokes needed to blank the statusbar has
 * been pressed.  If so, blank the statusbar, unless constant cursor
 * position display is on. */
void check_statusblank(void)
{
    if (statusblank > 0) {
	statusblank--;

	if (statusblank == 0 && !ISSET(CONST_UPDATE)) {
	    blank_statusbar();
	    wnoutrefresh(bottomwin);
	    reset_cursor();
	    wnoutrefresh(edit);
	}
    }

    /* If the subwindows overlap, make sure to show the edit window now. */
    if (LINES == 1)
	edit_refresh();
}

/* Convert buf into a string that can be displayed on screen.  The
 * caller wants to display buf starting with column start_col, and
 * extending for at most span columns.  start_col is zero-based.  span
 * is one-based, so span == 0 means you get "" returned.  The returned
 * string is dynamically allocated, and should be freed.  If isdata is
 * TRUE, the caller might put "$" at the beginning or end of the line if
 * it's too long. */
char *display_string(const char *buf, size_t start_col, size_t span,
	bool isdata)
{
    size_t start_index;
	/* Index in buf of the first character shown. */
    size_t column;
	/* Screen column that start_index corresponds to. */
    char *converted;
	/* The string we return. */
    size_t index;
	/* Current position in converted. */
    size_t beyond = start_col + span;
	/* The column number just beyond the last shown character. */

    start_index = actual_x(buf, start_col);
    column = strnlenpt(buf, start_index);

    assert(column <= start_col);

    index = 0;
<<<<<<< HEAD
=======
#ifdef USING_OLD_NCURSES
    seen_wide = FALSE;
#endif
    buf += start_index;
>>>>>>> origin/tomato-shibby-RT-AC

    /* Allocate enough space for converting the relevant part of the line. */
    converted = charalloc(strlen(buf) * (mb_cur_max() + tabsize) + 1);

    /* If the first character starts before the left edge, or would be
     * overwritten by a "$" token, then show spaces instead. */
    if (*buf != '\0' && *buf != '\t' && (column < start_col ||
				(column > 0 && isdata && !ISSET(SOFTWRAP)))) {
	if (is_cntrl_mbchar(buf)) {
	    if (column < start_col) {
<<<<<<< HEAD
		char *ctrl_buf_mb = charalloc(mb_cur_max());
		int ctrl_buf_mb_len, i;

		ctrl_buf_mb = control_mbrep(buf_mb, ctrl_buf_mb,
			&ctrl_buf_mb_len);

		for (i = 0; i < ctrl_buf_mb_len; i++)
		    converted[index++] = ctrl_buf_mb[i];

		start_col += mbwidth(ctrl_buf_mb);

		free(ctrl_buf_mb);

		start_index += buf_mb_len;
=======
		converted[index++] = control_mbrep(buf, isdata);
		start_col++;
		buf += parse_mbchar(buf, NULL, NULL);
>>>>>>> origin/tomato-shibby-RT-AC
	    }
	}
#ifdef ENABLE_UTF8
	else if (using_utf8() && mbwidth(buf) == 2) {
	    if (column >= start_col) {
		converted[index++] = ' ';
		start_col++;
	    }

	    converted[index++] = ' ';
	    start_col++;

	    buf += parse_mbchar(buf, NULL, NULL);
	}
#endif
    }

<<<<<<< HEAD
    while (buf[start_index] != '\0') {
	buf_mb_len = parse_mbchar(buf + start_index, buf_mb, NULL);

	/* Make sure there's enough room for the next character, whether
	 * it's a multibyte control character, a non-control multibyte
	 * character, a tab character, or a null terminator. */
	if (index + mb_cur_max() + tabsize + 1 >= alloc_len - 1) {
	    alloc_len += (mb_cur_max() + tabsize + 1) * MAX_BUF_SIZE;
	    converted = charealloc(converted, alloc_len);
	}

	/* If buf contains a tab character, interpret it. */
	if (*buf_mb == '\t') {
#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
=======
    while (*buf != '\0' && start_col < beyond) {
	int charlength, charwidth = 1;

	if (*buf == ' ') {
	    /* Show a space as a visible character, or as a space. */
#ifndef NANO_TINY
	    if (ISSET(WHITESPACE_DISPLAY)) {
		int i = whitespace_len[0];

		while (i < whitespace_len[0] + whitespace_len[1])
		    converted[index++] = whitespace[i++];
	    } else
#endif
		converted[index++] = ' ';
	    start_col++;
	    buf++;
	    continue;
	} else if (*buf == '\t') {
	    /* Show a tab as a visible character, or as as a space. */
#ifndef NANO_TINY
>>>>>>> origin/tomato-shibby-RT-AC
	    if (ISSET(WHITESPACE_DISPLAY)) {
		int i;

		for (i = 0; i < whitespace_len[0]; i++)
		    converted[index++] = whitespace[i];
	    } else
#endif
		converted[index++] = ' ';
	    start_col++;
	    while (start_col % tabsize != 0) {
		converted[index++] = ' ';
		start_col++;
	    }
<<<<<<< HEAD
	/* If buf contains a control character, interpret it.  If buf
	 * contains an invalid multibyte control character, display it
	 * as such.*/
	} else if (is_cntrl_mbchar(buf_mb)) {
	    char *ctrl_buf_mb = charalloc(mb_cur_max());
	    int ctrl_buf_mb_len, i;

	    converted[index++] = '^';
	    start_col++;

	    ctrl_buf_mb = control_mbrep(buf_mb, ctrl_buf_mb,
		&ctrl_buf_mb_len);

	    for (i = 0; i < ctrl_buf_mb_len; i++)
		converted[index++] = ctrl_buf_mb[i];

	    start_col += mbwidth(ctrl_buf_mb);

	    free(ctrl_buf_mb);
	/* If buf contains a space character, interpret it. */
	} else if (*buf_mb == ' ') {
#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
	    if (ISSET(WHITESPACE_DISPLAY)) {
		int i;

		for (i = whitespace_len[0]; i < whitespace_len[0] +
			whitespace_len[1]; i++)
		    converted[index++] = whitespace[i];
	    } else
#endif
		converted[index++] = ' ';
	    start_col++;
	/* If buf contains a non-control character, interpret it.  If
	 * buf contains an invalid multibyte non-control character,
	 * display it as such. */
	} else {
	    char *nctrl_buf_mb = charalloc(mb_cur_max());
	    int nctrl_buf_mb_len, i;

	    nctrl_buf_mb = mbrep(buf_mb, nctrl_buf_mb,
		&nctrl_buf_mb_len);

	    for (i = 0; i < nctrl_buf_mb_len; i++)
		converted[index++] = nctrl_buf_mb[i];

	    start_col += mbwidth(nctrl_buf_mb);

	    free(nctrl_buf_mb);
	}
=======
	    buf++;
	    continue;
	}

	charlength = length_of_char(buf, &charwidth);

	/* If buf contains a control character, represent it. */
	if (is_cntrl_mbchar(buf)) {
	    converted[index++] = '^';
	    converted[index++] = control_mbrep(buf, isdata);
	    start_col += 2;
	    buf += charlength;
	    continue;
	}

	/* If buf contains a valid non-control character, simply copy it. */
	if (charlength > 0) {
	    for (; charlength > 0; charlength--)
		converted[index++] = *(buf++);

	    start_col += charwidth;
#ifdef USING_OLD_NCURSES
	    if (charwidth > 1)
		seen_wide = TRUE;
#endif
	    continue;
	}

	/* Represent an invalid sequence with the Replacement Character. */
	converted[index++] = '\xEF';
	converted[index++] = '\xBF';
	converted[index++] = '\xBD';

	start_col += 1;
	buf++;
>>>>>>> origin/tomato-shibby-RT-AC

	/* For invalid codepoints, skip extra bytes. */
	if (charlength < -1)
	   buf += charlength + 7;
    }

    /* If there is more text than can be shown, make room for the $. */
    if (*buf != '\0' && isdata && !ISSET(SOFTWRAP))
	index = move_mbleft(converted, index);

    /* Null-terminate the converted string. */
    converted[index] = '\0';

    return converted;
}

/* If path is NULL, we're in normal editing mode, so display the current
 * version of nano, the current filename, and whether the current file
 * has been modified on the titlebar.  If path isn't NULL, we're in the
 * file browser, and path contains the directory to start the file
 * browser in, so display the current version of nano and the contents
 * of path on the titlebar. */
void titlebar(const char *path)
{
<<<<<<< HEAD
    int space = COLS;
	/* The space we have available for display. */
    size_t verlen = strlenpt(PACKAGE_STRING) + 1;
	/* The length of the version message in columns, plus one for
	 * padding. */
    const char *prefix;
	/* "DIR:", "File:", or "New Buffer".  Goes before filename. */
    size_t prefixlen;
	/* The length of the prefix in columns, plus one for padding. */
    const char *state;
	/* "Modified", "View", or "".  Shows the state of this
	 * buffer. */
    size_t statelen = 0;
	/* The length of the state in columns, or the length of
	 * "Modified" if the state is blank and we're not in the file
	 * browser. */
    char *exppath = NULL;
	/* The filename, expanded for display. */
    bool newfie = FALSE;
	/* Do we say "New Buffer"? */
    bool dots = FALSE;
	/* Do we put an ellipsis before the path? */

    assert(path != NULL || openfile->filename != NULL);

    wattron(topwin, reverse_attr);
=======
    size_t verlen, prefixlen, pathlen, statelen;
	/* The width of the different titlebar elements, in columns. */
    size_t pluglen = 0;
	/* The width that "Modified" would take up. */
    size_t offset = 0;
	/* The position at which the center part of the titlebar starts. */
    const char *prefix = "";
	/* What is shown before the path -- "File:", "DIR:", or "". */
    const char *state = "";
	/* The state of the current buffer -- "Modified", "View", or "". */
    char *caption;
	/* The presentable form of the pathname. */

    /* If the screen is too small, there is no titlebar. */
    if (topwin == NULL)
	return;

    assert(path != NULL || openfile->filename != NULL);

    wattron(topwin, interface_color_pair[TITLE_BAR]);
>>>>>>> origin/tomato-shibby-RT-AC

    blank_titlebar();
    as_an_at = FALSE;

    /* space has to be at least 4: two spaces before the version message,
     * at least one character of the version message, and one space
     * after the version message. */
    if (space < 4)
	space = 0;
    else {
	/* Limit verlen to 1/3 the length of the screen in columns,
	 * minus three columns for spaces. */
	if (verlen > (COLS / 3) - 3)
	    verlen = (COLS / 3) - 3;
    }

    if (space >= 4) {
	/* Add a space after the version message, and account for both
	 * it and the two spaces before it. */
	mvwaddnstr(topwin, 0, 2, PACKAGE_STRING,
		actual_x(PACKAGE_STRING, verlen));
	verlen += 3;

	/* Account for the full length of the version message. */
	space -= verlen;
    }

#ifndef DISABLE_BROWSER
    /* Don't display the state if we're in the file browser. */
    if (path != NULL)
	state = "";
    else
#endif
	state = openfile->modified ? _("Modified") : ISSET(VIEW_MODE) ?
		_("View") : "";

    statelen = strlenpt((*state == '\0' && path == NULL) ?
	_("Modified") : state);

    /* If possible, add a space before state. */
    if (space > 0 && statelen < space)
	statelen++;
    else
	goto the_end;

#ifndef DISABLE_BROWSER
    /* path should be a directory if we're in the file browser. */
    if (path != NULL)
	prefix = _("DIR:");
    else
#endif
    if (openfile->filename[0] == '\0') {
	prefix = _("New Buffer");
	newfie = TRUE;
    } else
	prefix = _("File:");

    prefixlen = strnlenpt(prefix, space - statelen) + 1;

    /* If newfie is FALSE, add a space after prefix. */
    if (!newfie && prefixlen + statelen < space)
	prefixlen++;

    /* If we're not in the file browser, set path to the current
     * filename. */
    if (path == NULL)
	path = openfile->filename;

    /* Account for the full lengths of the prefix and the state. */
    if (space >= prefixlen + statelen)
	space -= prefixlen + statelen;
    else
	space = 0;
	/* space is now the room we have for the filename. */

    if (!newfie) {
	size_t lenpt = strlenpt(path), start_col;

	/* Don't set dots to TRUE if we have fewer than eight columns
	 * (i.e. one column for padding, plus seven columns for a
	 * filename). */
	dots = (space >= 8 && lenpt >= space);

	if (dots) {
	    start_col = lenpt - space + 3;
	    space -= 3;
	} else
	    start_col = 0;

	exppath = display_string(path, start_col, space, FALSE);
    }

    /* If dots is TRUE, we will display something like "File:
     * ...ename". */
    if (dots) {
	mvwaddnstr(topwin, 0, verlen - 1, prefix, actual_x(prefix,
		prefixlen));
	if (space <= -3 || newfie)
	    goto the_end;
	waddch(topwin, ' ');
	waddnstr(topwin, "...", space + 3);
	if (space <= 0)
	    goto the_end;
	waddstr(topwin, exppath);
    } else {
	size_t exppathlen = newfie ? 0 : strlenpt(exppath);
	    /* The length of the expanded filename. */

	/* There is room for the whole filename, so we center it. */
	mvwaddnstr(topwin, 0, verlen + ((space - exppathlen) / 3),
		prefix, actual_x(prefix, prefixlen));
	if (!newfie) {
	    waddch(topwin, ' ');
	    waddstr(topwin, exppath);
	}
    }

  the_end:
    free(exppath);

<<<<<<< HEAD
    if (state[0] != '\0') {
	if (statelen >= COLS - 1)
	    mvwaddnstr(topwin, 0, 0, state, actual_x(state, COLS));
	else {
	    assert(COLS - statelen - 1 >= 0);
=======
    /* Only print the prefix when there is room for it. */
    if (verlen + prefixlen + pathlen + pluglen + statelen <= COLS) {
	mvwaddstr(topwin, 0, offset, prefix);
	if (prefixlen > 0)
	    waddstr(topwin, " ");
    } else
	wmove(topwin, 0, offset);

    /* Print the full path if there's room; otherwise, dottify it. */
    if (pathlen + pluglen + statelen <= COLS) {
	caption = display_string(path, 0, pathlen, FALSE);
	waddstr(topwin, caption);
	free(caption);
    } else if (5 + statelen <= COLS) {
	waddstr(topwin, "...");
	caption = display_string(path, 3 + pathlen - COLS + statelen,
					COLS - statelen, FALSE);
	waddstr(topwin, caption);
	free(caption);
    }
>>>>>>> origin/tomato-shibby-RT-AC

	    mvwaddnstr(topwin, 0, COLS - statelen - 1, state,
		actual_x(state, statelen));
	}
    }

<<<<<<< HEAD
    wattroff(topwin, reverse_attr);
=======
    wattroff(topwin, interface_color_pair[TITLE_BAR]);
>>>>>>> origin/tomato-shibby-RT-AC

    wnoutrefresh(topwin);
    reset_cursor();
    wnoutrefresh(edit);
}

/* Mark the current file as modified if it isn't already, and then
 * update the titlebar to display the file's new status. */
void set_modified(void)
{
    if (!openfile->modified) {
	openfile->modified = TRUE;
	titlebar(NULL);
    }
}

<<<<<<< HEAD
/* Display a message on the statusbar, and set disable_cursorpos to
=======
/* Warn the user on the statusbar and pause for a moment, so that the
 * message can be noticed and read. */
void warn_and_shortly_pause(const char *msg)
{
    statusbar(msg);
    beep();
    napms(1800);
}

/* Display a message on the statusbar, and set suppress_cursorpos to
>>>>>>> origin/tomato-shibby-RT-AC
 * TRUE, so that the message won't be immediately overwritten if
 * constant cursor position display is on. */
void statusbar(const char *msg, ...)
{
    va_list ap;
<<<<<<< HEAD
    char *bar, *foo;
    size_t start_x, foo_len;
#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
    bool old_whitespace;
=======
    static int alerts = 0;
    char *compound, *message;
    size_t start_col;
    bool bracketed;
#ifndef NANO_TINY
    bool old_whitespace = ISSET(WHITESPACE_DISPLAY);

    UNSET(WHITESPACE_DISPLAY);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    va_start(ap, msg);

    /* Curses mode is turned off.  If we use wmove() now, it will muck
     * up the terminal settings.  So we just use vfprintf(). */
    if (isendwin()) {
	vfprintf(stderr, msg, ap);
	va_end(ap);
	return;
    }

<<<<<<< HEAD
    blank_statusbar();

#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
    old_whitespace = ISSET(WHITESPACE_DISPLAY);
    UNSET(WHITESPACE_DISPLAY);
#endif
    bar = charalloc(mb_cur_max() * (COLS - 3));
    vsnprintf(bar, mb_cur_max() * (COLS - 3), msg, ap);
    va_end(ap);
    foo = display_string(bar, 0, COLS - 4, FALSE);
#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
    if (old_whitespace)
	SET(WHITESPACE_DISPLAY);
#endif
    free(bar);
    foo_len = strlenpt(foo);
    start_x = (COLS - foo_len - 4) / 2;

    wmove(bottomwin, 0, start_x);
    wattron(bottomwin, reverse_attr);
    waddstr(bottomwin, "[ ");
    waddstr(bottomwin, foo);
    free(foo);
    waddstr(bottomwin, " ]");
    wattroff(bottomwin, reverse_attr);
=======
    /* If there already was an alert message, ignore lesser ones. */
    if ((lastmessage == ALERT && importance != ALERT) ||
		(lastmessage == MILD && importance == HUSH))
	return;

    /* If the ALERT status has been reset, reset the counter. */
    if (lastmessage == HUSH)
	alerts = 0;

    /* Shortly pause after each of the first three alert messages,
     * to give the user time to read them. */
    if (lastmessage == ALERT && alerts < 4 && !ISSET(NO_PAUSES))
	napms(1200);

    if (importance == ALERT) {
	if (++alerts > 3 && !ISSET(NO_PAUSES))
	    msg = _("Further warnings were suppressed");
	beep();
    }

    lastmessage = importance;

    /* Turn the cursor off while fiddling in the statusbar. */
    curs_set(0);

    blank_statusbar();

    /* Construct the message out of all the arguments. */
    compound = charalloc(mb_cur_max() * (COLS + 1));
    vsnprintf(compound, mb_cur_max() * (COLS + 1), msg, ap);
    va_end(ap);
    message = display_string(compound, 0, COLS, FALSE);
    free(compound);

    start_col = (COLS - strlenpt(message)) / 2;
    bracketed = (start_col > 1);

    wmove(bottomwin, 0, (bracketed ? start_col - 2 : start_col));
    wattron(bottomwin, interface_color_pair[STATUS_BAR]);
    if (bracketed)
	waddstr(bottomwin, "[ ");
    waddstr(bottomwin, message);
    free(message);
    if (bracketed)
	waddstr(bottomwin, " ]");
    wattroff(bottomwin, interface_color_pair[STATUS_BAR]);

    /* Push the message to the screen straightaway. */
>>>>>>> origin/tomato-shibby-RT-AC
    wnoutrefresh(bottomwin);
    reset_cursor();
    wnoutrefresh(edit);
	/* Leave the cursor at its position in the edit window, not in
	 * the statusbar. */

    disable_cursorpos = TRUE;

<<<<<<< HEAD
    /* If we're doing quick statusbar blanking, and constant cursor
     * position display is off, blank the statusbar after only one
     * keystroke.  Otherwise, blank it after twenty-six keystrokes, as
     * Pico does. */
    statusblank =
#ifndef NANO_TINY
	ISSET(QUICK_BLANK) && !ISSET(CONST_UPDATE) ? 1 :
=======
#ifndef NANO_TINY
    if (old_whitespace)
	SET(WHITESPACE_DISPLAY);

    /* If doing quick blanking, blank the statusbar after just one keystroke.
     * Otherwise, blank it after twenty-six keystrokes, as Pico does. */
    if (ISSET(QUICK_BLANK))
	statusblank = 1;
    else
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	26;
}

/* Display the shortcut list in s on the last two rows of the bottom
 * portion of the window. */
void bottombars(int menu)
{
    size_t number, itemwidth, i;
    subnfunc *f;
    const sc *s;

<<<<<<< HEAD
    if (ISSET(NO_HELP))
=======
    /* Set the global variable to the given menu. */
    currmenu = menu;

    if (ISSET(NO_HELP) || LINES < 5)
>>>>>>> origin/tomato-shibby-RT-AC
	return;

    /* Determine how many shortcuts there are to show. */
    number = length_of_list(menu);

    if (number > MAIN_VISIBLE)
	number = MAIN_VISIBLE;

    /* Compute the width of each keyname-plus-explanation pair. */
    itemwidth = COLS / ((number / 2) + (number % 2));

    /* If there is no room, don't print anything. */
    if (itemwidth == 0)
	return;

    blank_bottombars();

#ifdef DEBUG
    fprintf(stderr, "In bottombars, number of items == \"%d\"\n", (int) number);
#endif

    for (f = allfuncs, i = 0; i < number && f != NULL; f = f->next) {
#ifdef DEBUG
        fprintf(stderr, "Checking menu items....");
#endif
        if ((f->menus & menu) == 0)
	    continue;

        if (!f->desc || strlen(f->desc) == 0)
	    continue;

#ifdef DEBUG
        fprintf(stderr, "found one! f->menus = %d, desc = \"%s\"\n", f->menus, f->desc);
#endif
        s = first_sc_for(menu, f->scfunc);
        if (s == NULL) {
#ifdef DEBUG
	    fprintf(stderr, "Whoops, guess not, no shortcut key found for func!\n");
#endif
<<<<<<< HEAD
            continue;
        }
	wmove(bottomwin, 1 + i % 2, (i / 2) * colwidth);
=======
	    continue;
	}

	wmove(bottomwin, 1 + i % 2, (i / 2) * itemwidth);
>>>>>>> origin/tomato-shibby-RT-AC
#ifdef DEBUG
        fprintf(stderr, "Calling onekey with keystr \"%s\" and desc \"%s\"\n", s->keystr, f->desc);
#endif
<<<<<<< HEAD
	onekey(s->keystr, _(f->desc), colwidth + (COLS % colwidth));
        i++;
=======
	onekey(s->keystr, _(f->desc), itemwidth + (COLS % itemwidth));
	i++;
>>>>>>> origin/tomato-shibby-RT-AC
    }

    /* Defeat a VTE bug by moving the cursor and forcing a screen update. */
    wmove(bottomwin, 0, 0);
    wnoutrefresh(bottomwin);
    doupdate();

    reset_cursor();
    wnoutrefresh(edit);
}

/* Write a shortcut key to the help area at the bottom of the window.
 * keystroke is e.g. "^G" and desc is e.g. "Get Help".  We are careful
 * to write at most len characters, even if len is very small and
 * keystroke and desc are long.  Note that waddnstr(,,(size_t)-1) adds
 * the whole string!  We do not bother padding the entry with blanks. */
void onekey(const char *keystroke, const char *desc, size_t len)
{
    size_t keystroke_len = strlenpt(keystroke) + 1;

    assert(keystroke != NULL && desc != NULL);

<<<<<<< HEAD
    wattron(bottomwin, reverse_attr);
    waddnstr(bottomwin, keystroke, actual_x(keystroke, len));
    wattroff(bottomwin, reverse_attr);
=======
    wattron(bottomwin, interface_color_pair[KEY_COMBO]);
    waddnstr(bottomwin, keystroke, actual_x(keystroke, length));
    wattroff(bottomwin, interface_color_pair[KEY_COMBO]);
>>>>>>> origin/tomato-shibby-RT-AC

    if (len > keystroke_len)
	len -= keystroke_len;
    else
	len = 0;

    if (len > 0) {
	waddch(bottomwin, ' ');
<<<<<<< HEAD
	waddnstr(bottomwin, desc, actual_x(desc, len));
    }
}

/* Reset current_y, based on the position of current, and put the cursor
 * in the edit window at (current_y, current_x). */
void reset_cursor(void)
{
    size_t xpt;
    /* If we haven't opened any files yet, put the cursor in the top
     * left corner of the edit window and get out. */
    if (openfile == NULL) {
	wmove(edit, 0, 0);
	return;
    }

    xpt = xplustabs();
=======
	wattron(bottomwin, interface_color_pair[FUNCTION_TAG]);
	waddnstr(bottomwin, desc, actual_x(desc, length));
	wattroff(bottomwin, interface_color_pair[FUNCTION_TAG]);
    }
}

/* Redetermine current_y from the position of current relative to edittop,
 * and put the cursor in the edit window at (current_y, "current_x"). */
void reset_cursor(void)
{
    ssize_t row = 0;
    size_t col, xpt = xplustabs();
>>>>>>> origin/tomato-shibby-RT-AC

    if (ISSET(SOFTWRAP)) {
<<<<<<< HEAD
	filestruct *tmp;
	openfile->current_y = 0;

	for (tmp = openfile->edittop; tmp && tmp != openfile->current; tmp = tmp->next)
	    openfile->current_y += 1 + strlenpt(tmp->data) / COLS;

	openfile->current_y += xplustabs() / COLS;
	if (openfile->current_y < editwinrows)
	    wmove(edit, openfile->current_y, xpt % COLS);
    } else {
	openfile->current_y = openfile->current->lineno -
	    openfile->edittop->lineno;

	if (openfile->current_y < editwinrows)
	    wmove(edit, openfile->current_y, xpt - get_page_start(xpt));
=======
	filestruct *line = openfile->edittop;

	row -= (openfile->firstcolumn / editwincols);

	/* Calculate how many rows the lines from edittop to current use. */
	while (line != NULL && line != openfile->current) {
	    row += strlenpt(line->data) / editwincols + 1;
	    line = line->next;
	}

	/* Add the number of wraps in the current line before the cursor. */
	row += xpt / editwincols;
	col = xpt % editwincols;
    } else
#endif
    {
	row = openfile->current->lineno - openfile->edittop->lineno;
	col = xpt - get_page_start(xpt);
>>>>>>> origin/tomato-shibby-RT-AC
    }

    if (row < editwinrows)
	wmove(edit, row, margin + col);

    openfile->current_y = row;
}

/* edit_draw() takes care of the job of actually painting a line into
 * the edit window.  fileptr is the line to be painted, at row row of
 * the window.  converted is the actual string to be written to the
 * window, with tabs and control characters replaced by strings of
 * regular characters.  from_col is the column number of the first
 * character of this page.  That is, the first character of converted
 * corresponds to character number actual_x(fileptr->data, from_col) of the
 * line. */
void edit_draw(filestruct *fileptr, const char *converted,
	int row, size_t from_col)
{
<<<<<<< HEAD
#if !defined(NANO_TINY) || defined(ENABLE_COLOR)
    size_t startpos = actual_x(fileptr->data, start);
=======
#if !defined(NANO_TINY) || !defined(DISABLE_COLOR)
    size_t from_x = actual_x(fileptr->data, from_col);
>>>>>>> origin/tomato-shibby-RT-AC
	/* The position in fileptr->data of the leftmost character
	 * that displays at least partially on the window. */
    size_t till_x = actual_x(fileptr->data, from_col + editwincols - 1) + 1;
	/* The position in fileptr->data of the first character that is
	 * completely off the window to the right.  Note that till_x
	 * might be beyond the null terminator of the string. */
#endif

    assert(openfile != NULL && fileptr != NULL && converted != NULL);
    assert(strlenpt(converted) <= editwincols);

#ifdef ENABLE_LINENUMBERS
    /* If line numbering is switched on, put a line number in front of
     * the text -- but only for the parts that are not softwrapped. */
    if (margin > 0) {
	wattron(edit, interface_color_pair[LINE_NUMBER]);
#ifndef NANO_TINY
	if (ISSET(SOFTWRAP) && from_x != 0)
	    mvwprintw(edit, row, 0, "%*s", margin - 1, " ");
	else
#endif
	    mvwprintw(edit, row, 0, "%*ld", margin - 1, (long)fileptr->lineno);
	wattroff(edit, interface_color_pair[LINE_NUMBER]);
    }
#endif

<<<<<<< HEAD
    /* Just paint the string in any case (we'll add color or reverse on
     * just the text that needs it). */
    mvwaddstr(edit, line, 0, converted);

#ifdef ENABLE_COLOR
    /* If color syntaxes are available and turned on, we need to display
     * them. */
    if (openfile->colorstrings != NULL && !ISSET(NO_COLOR_SYNTAX)) {
	const colortype *tmpcolor = openfile->colorstrings;

	/* Set up multi-line color data for this line if it's not yet calculated  */
        if (fileptr->multidata == NULL && openfile->syntax
		&& openfile->syntax->nmultis > 0) {
 	    int i;
	    fileptr->multidata = (short *) nmalloc(openfile->syntax->nmultis * sizeof(short));
            for (i = 0; i < openfile->syntax->nmultis; i++)
		fileptr->multidata[i] = -1;	/* Assue this applies until we know otherwise */
	}
	for (; tmpcolor != NULL; tmpcolor = tmpcolor->next) {
	    int x_start;
		/* Starting column for mvwaddnstr.  Zero-based. */
	    int paintlen;
		/* Number of chars to paint on this line.  There are
		 * COLS characters on a whole line. */
	    size_t index;
		/* Index in converted where we paint. */
	    regmatch_t startmatch;
		/* Match position for start_regex. */
	    regmatch_t endmatch;
		/* Match position for end_regex. */

	    if (tmpcolor->bright)
		wattron(edit, A_BOLD);
	    wattron(edit, COLOR_PAIR(tmpcolor->pairnum));
=======
    /* First simply write the converted line -- afterward we'll add colors
     * and the marking highlight on just the pieces that need it. */
    mvwaddstr(edit, row, margin, converted);

#ifdef USING_OLD_NCURSES
    /* Tell ncurses to really redraw the line without trying to optimize
     * for what it thinks is already there, because it gets it wrong in
     * the case of a wide character in column zero.  See bug #31743. */
    if (seen_wide)
	wredrawln(edit, row, 1);
#endif

#ifndef DISABLE_COLOR
    /* If color syntaxes are available and turned on, apply them. */
    if (openfile->colorstrings != NULL && !ISSET(NO_COLOR_SYNTAX)) {
	const colortype *varnish = openfile->colorstrings;

	/* If there are multiline regexes, make sure there is a cache. */
	if (openfile->syntax->nmultis > 0)
	    alloc_multidata_if_needed(fileptr);

	/* Iterate through all the coloring regexes. */
	for (; varnish != NULL; varnish = varnish->next) {
	    size_t index = 0;
		/* Where in the line we currently begin looking for a match. */
	    int start_col;
		/* The starting column of a piece to paint.  Zero-based. */
	    int paintlen = 0;
		/* The number of characters to paint. */
	    const char *thetext;
		/* The place in converted from where painting starts. */
	    regmatch_t match;
		/* The match positions of a single-line regex. */

>>>>>>> origin/tomato-shibby-RT-AC
	    /* Two notes about regexec().  A return value of zero means
	     * that there is a match.  Also, rm_eo is the first
	     * non-matching character after the match. */

<<<<<<< HEAD
	    /* First case, tmpcolor is a single-line expression. */
	    if (tmpcolor->end == NULL) {
		size_t k = 0;

		/* We increment k by rm_eo, to move past the end of the
=======
	    wattron(edit, varnish->attributes);

	    /* First case: varnish is a single-line expression. */
	    if (varnish->end == NULL) {
		/* We increment index by rm_eo, to move past the end of the
>>>>>>> origin/tomato-shibby-RT-AC
		 * last match.  Even though two matches may overlap, we
		 * want to ignore them, so that we can highlight e.g. C
		 * strings correctly. */
		while (index < till_x) {
		    /* Note the fifth parameter to regexec().  It says
		     * not to match the beginning-of-line character
		     * unless index is zero.  If regexec() returns
		     * REG_NOMATCH, there are no more matches in the
		     * line. */
<<<<<<< HEAD
		    if (regexec(tmpcolor->start, &fileptr->data[k], 1,
			&startmatch, (k == 0) ? 0 : REG_NOTBOL) ==
			REG_NOMATCH)
=======
		    if (regexec(varnish->start, &fileptr->data[index], 1,
				&match, (index == 0) ? 0 : REG_NOTBOL) != 0)
>>>>>>> origin/tomato-shibby-RT-AC
			break;

		    /* If the match is of length zero, skip it. */
		    if (match.rm_so == match.rm_eo) {
			index = move_mbright(fileptr->data,
						index + match.rm_eo);
			continue;
		    }

		    /* Translate the match to the beginning of the line. */
		    match.rm_so += index;
		    match.rm_eo += index;
		    index = match.rm_eo;

		    /* If the matching part is not visible, skip it. */
		    if (match.rm_eo <= from_x || match.rm_so >= till_x)
			continue;

		    start_col = (match.rm_so <= from_x) ?
					0 : strnlenpt(fileptr->data,
					match.rm_so) - from_col;

<<<<<<< HEAD
			mvwaddnstr(edit, line, x_start, converted +
				index, paintlen);
		    }
		    k = startmatch.rm_eo;
		}
	    } else if (fileptr->multidata != NULL && fileptr->multidata[tmpcolor->id] != CNONE) {
		/* This is a multi-line regex.  There are two steps.
		 * First, we have to see if the beginning of the line is
		 * colored by a start on an earlier line, and an end on
		 * this line or later.
		 *
		 * We find the first line before fileptr matching the
		 * start.  If every match on that line is followed by an
		 * end, then go to step two.  Otherwise, find the next
		 * line after start_line matching the end.  If that line
		 * is not before fileptr, then paint the beginning of
		 * this line. */
		const filestruct *start_line = fileptr->prev;
		    /* The first line before fileptr matching start. */
		regoff_t start_col;
		    /* Where it starts in that line. */
		const filestruct *end_line;
		short md = fileptr->multidata[tmpcolor->id];

		if (md == -1)
		    fileptr->multidata[tmpcolor->id] = CNONE; /* until we find out otherwise */
		else if (md == CNONE)
		    continue;
		else if (md == CWHOLELINE) {
		    mvwaddnstr(edit, line, 0, converted, -1);
		    continue;
		} else if (md == CBEGINBEFORE) {
		    regexec(tmpcolor->end, fileptr->data, 1, &endmatch, 0);
		    paintlen = actual_x(converted, strnlenpt(fileptr->data,
			endmatch.rm_eo) - start);
		    mvwaddnstr(edit, line, 0, converted, paintlen);
		    continue;
		}

		while (start_line != NULL && regexec(tmpcolor->start,
			start_line->data, 1, &startmatch, 0) ==
			REG_NOMATCH) {
		    /* If there is an end on this line, there is no need
		     * to look for starts on earlier lines. */
		    if (regexec(tmpcolor->end, start_line->data, 0,
			NULL, 0) == 0)
			goto step_two;
		    start_line = start_line->prev;
=======
		    thetext = converted + actual_x(converted, start_col);

		    paintlen = actual_x(thetext, strnlenpt(fileptr->data,
					match.rm_eo) - from_col - start_col);

		    mvwaddnstr(edit, row, margin + start_col,
						thetext, paintlen);
>>>>>>> origin/tomato-shibby-RT-AC
		}
		goto tail_of_loop;
	    }

<<<<<<< HEAD
		/* Skip over a zero-length regex match. */
		if (startmatch.rm_so == startmatch.rm_eo)
		    startmatch.rm_eo++;
		else {
		    /* No start found, so skip to the next step. */
		    if (start_line == NULL)
			goto step_two;
		    /* Now start_line is the first line before fileptr
		     * containing a start match.  Is there a start on
		     * this line not followed by an end on this line? */
		    start_col = 0;
		    while (TRUE) {
			start_col += startmatch.rm_so;
			startmatch.rm_eo -= startmatch.rm_so;
			if (regexec(tmpcolor->end, start_line->data +
				start_col + startmatch.rm_eo, 0, NULL,
				(start_col + startmatch.rm_eo == 0) ?
				0 : REG_NOTBOL) == REG_NOMATCH)
			    /* No end found after this start. */
			    break;
			start_col++;
			if (regexec(tmpcolor->start, start_line->data +
				start_col, 1, &startmatch,
				REG_NOTBOL) == REG_NOMATCH)
			    /* No later start on this line. */
			    goto step_two;
		    }
		    /* Indeed, there is a start not followed on this
		     * line by an end. */

		    /* We have already checked that there is no end
		     * before fileptr and after the start.  Is there an
		     * end after the start at all?  We don't paint
		     * unterminated starts. */
		    end_line = fileptr;
		    while (end_line != NULL && regexec(tmpcolor->end,
			end_line->data, 1, &endmatch, 0) == REG_NOMATCH)
			end_line = end_line->next;

		    /* No end found, or it is too early. */
		    if (end_line == NULL || (end_line == fileptr &&
			endmatch.rm_eo <= startpos))
			goto step_two;

		    /* Now paint the start of fileptr.  If the start of
		     * fileptr is on a different line from the end,
		     * paintlen is -1, meaning that everything on the
		     * line gets painted.  Otherwise, paintlen is the
		     * expanded location of the end of the match minus
		     * the expanded location of the beginning of the
		     * page. */
		    if (end_line != fileptr) {
			paintlen = -1;
			fileptr->multidata[tmpcolor->id] = CWHOLELINE;
		    } else {
			paintlen = actual_x(converted,
				strnlenpt(fileptr->data,
				endmatch.rm_eo) - start);
			fileptr->multidata[tmpcolor->id] = CBEGINBEFORE;
		    }
		    mvwaddnstr(edit, line, 0, converted, paintlen);
  step_two:
		    /* Second step, we look for starts on this line. */
		    start_col = 0;

		    while (start_col < endpos) {
			if (regexec(tmpcolor->start, fileptr->data +
				start_col, 1, &startmatch, (start_col ==
				0) ? 0 : REG_NOTBOL) == REG_NOMATCH ||
				start_col + startmatch.rm_so >= endpos)
			    /* No more starts on this line. */
			    break;
			/* Translate the match to be relative to the
			 * beginning of the line. */
			startmatch.rm_so += start_col;
			startmatch.rm_eo += start_col;

			x_start = (startmatch.rm_so <= startpos) ? 0 :
				strnlenpt(fileptr->data,
				startmatch.rm_so) - start;

			index = actual_x(converted, x_start);

			if (regexec(tmpcolor->end, fileptr->data +
				startmatch.rm_eo, 1, &endmatch,
				(startmatch.rm_eo == 0) ? 0 :
				REG_NOTBOL) == 0) {
			    /* Translate the end match to be relative to
			     * the beginning of the line. */
			    endmatch.rm_so += startmatch.rm_eo;
			    endmatch.rm_eo += startmatch.rm_eo;
			    /* There is an end on this line.  But does
			     * it appear on this page, and is the match
			     * more than zero characters long? */
			    if (endmatch.rm_eo > startpos &&
				endmatch.rm_eo > startmatch.rm_so) {
				paintlen = actual_x(converted + index,
					strnlenpt(fileptr->data,
					endmatch.rm_eo) - start -
					x_start);

				assert(0 <= x_start && x_start < COLS);

				mvwaddnstr(edit, line, x_start,
					converted + index, paintlen);
				if (paintlen > 0)
				    fileptr->multidata[tmpcolor->id] = CSTARTENDHERE;

			    }
			} else {
			    /* There is no end on this line.  But we
			     * haven't yet looked for one on later
			     * lines. */
			    end_line = fileptr->next;

			    while (end_line != NULL &&
				regexec(tmpcolor->end, end_line->data,
				0, NULL, 0) == REG_NOMATCH)
				end_line = end_line->next;

			    if (end_line != NULL) {
				assert(0 <= x_start && x_start < COLS);

				mvwaddnstr(edit, line, x_start,
					converted + index, -1);
				/* We painted to the end of the line, so
				 * don't bother checking any more
				 * starts. */
				fileptr->multidata[tmpcolor->id] = CENDAFTER;
				break;
			    }
			}
			start_col = startmatch.rm_so + 1;
		    }
=======
	    /* Second case: varnish is a multiline expression. */
	    const filestruct *start_line = fileptr->prev;
		/* The first line before fileptr that matches 'start'. */
	    const filestruct *end_line = fileptr;
		/* The line that matches 'end'. */
	    regmatch_t startmatch, endmatch;
		/* The match positions of the start and end regexes. */

	    /* Assume nothing gets painted until proven otherwise below. */
	    fileptr->multidata[varnish->id] = CNONE;

	    /* First check the multidata of the preceding line -- it tells
	     * us about the situation so far, and thus what to do here. */
	    if (start_line != NULL && start_line->multidata != NULL) {
		if (start_line->multidata[varnish->id] == CWHOLELINE ||
			start_line->multidata[varnish->id] == CENDAFTER ||
			start_line->multidata[varnish->id] == CWOULDBE)
		    goto seek_an_end;
		if (start_line->multidata[varnish->id] == CNONE ||
			start_line->multidata[varnish->id] == CBEGINBEFORE ||
			start_line->multidata[varnish->id] == CSTARTENDHERE)
		    goto step_two;
	    }

	    /* The preceding line has no precalculated multidata.  So, do
	     * some backtracking to find out what to paint. */

	    /* First step: see if there is a line before current that
	     * matches 'start' and is not complemented by an 'end'. */
	    while (start_line != NULL && regexec(varnish->start,
		    start_line->data, 1, &startmatch, 0) == REG_NOMATCH) {
		/* There is no start on this line; but if there is an end,
		 * there is no need to look for starts on earlier lines. */
		if (regexec(varnish->end, start_line->data, 0, NULL, 0) == 0)
		    goto step_two;
		start_line = start_line->prev;
	    }

	    /* If no start was found, skip to the next step. */
	    if (start_line == NULL)
		goto step_two;

	    /* If a found start has been qualified as an end earlier,
	     * believe it and skip to the next step. */
	    if (start_line->multidata != NULL &&
			(start_line->multidata[varnish->id] == CBEGINBEFORE ||
			start_line->multidata[varnish->id] == CSTARTENDHERE))
		goto step_two;

	    /* Is there an uncomplemented start on the found line? */
	    while (TRUE) {
		/* Begin searching for an end after the start match. */
		index += startmatch.rm_eo;
		/* If there is no end after this last start, good. */
		if (regexec(varnish->end, start_line->data + index, 1, &endmatch,
				(index == 0) ? 0 : REG_NOTBOL) == REG_NOMATCH)
		    break;
		/* Begin searching for a new start after the end match. */
		index += endmatch.rm_eo;
		/* If both start and end match are mere anchors, advance. */
		if (startmatch.rm_so == startmatch.rm_eo &&
				endmatch.rm_so == endmatch.rm_eo) {
		    if (start_line->data[index] == '\0')
			break;
		    index = move_mbright(start_line->data, index);
		}
		/* If there is no later start on this line, next step. */
		if (regexec(varnish->start, start_line->data + index,
				1, &startmatch, REG_NOTBOL) == REG_NOMATCH)
		    goto step_two;
	    }
	    /* Indeed, there is a start without an end on that line. */

  seek_an_end:
	    /* We've already checked that there is no end between the start
	     * and the current line.  But is there an end after the start
	     * at all?  We don't paint unterminated starts. */
	    while (end_line != NULL && regexec(varnish->end, end_line->data,
				 1, &endmatch, 0) == REG_NOMATCH)
		end_line = end_line->next;

	    /* If there is no end, there is nothing to paint. */
	    if (end_line == NULL) {
		fileptr->multidata[varnish->id] = CWOULDBE;
		goto tail_of_loop;
	    }

	    /* If the end is on a later line, paint whole line, and be done. */
	    if (end_line != fileptr) {
		mvwaddnstr(edit, row, margin, converted, -1);
		fileptr->multidata[varnish->id] = CWHOLELINE;
		goto tail_of_loop;
	    }

	    /* Only if it is visible, paint the part to be coloured. */
	    if (endmatch.rm_eo > from_x) {
		paintlen = actual_x(converted, strnlenpt(fileptr->data,
						endmatch.rm_eo) - from_col);
		mvwaddnstr(edit, row, margin, converted, paintlen);
	    }
	    fileptr->multidata[varnish->id] = CBEGINBEFORE;

  step_two:
	    /* Second step: look for starts on this line, but begin
	     * looking only after an end match, if there is one. */
	    index = (paintlen == 0) ? 0 : endmatch.rm_eo;

	    while (regexec(varnish->start, fileptr->data + index,
				1, &startmatch, (index == 0) ?
				0 : REG_NOTBOL) == 0) {
		/* Translate the match to be relative to the
		 * beginning of the line. */
		startmatch.rm_so += index;
		startmatch.rm_eo += index;

		start_col = (startmatch.rm_so <= from_x) ?
				0 : strnlenpt(fileptr->data,
				startmatch.rm_so) - from_col;

		thetext = converted + actual_x(converted, start_col);

		if (regexec(varnish->end, fileptr->data + startmatch.rm_eo,
				1, &endmatch, (startmatch.rm_eo == 0) ?
				0 : REG_NOTBOL) == 0) {
		    /* Translate the end match to be relative to
		     * the beginning of the line. */
		    endmatch.rm_so += startmatch.rm_eo;
		    endmatch.rm_eo += startmatch.rm_eo;
		    /* Only paint the match if it is visible on screen and
		     * it is more than zero characters long. */
		    if (endmatch.rm_eo > from_x &&
					endmatch.rm_eo > startmatch.rm_so) {
			paintlen = actual_x(thetext, strnlenpt(fileptr->data,
					endmatch.rm_eo) - from_col - start_col);

			mvwaddnstr(edit, row, margin + start_col,
						thetext, paintlen);

			fileptr->multidata[varnish->id] = CSTARTENDHERE;
		    }
		    index = endmatch.rm_eo;
		    /* If both start and end match are anchors, advance. */
		    if (startmatch.rm_so == startmatch.rm_eo &&
				endmatch.rm_so == endmatch.rm_eo) {
			if (fileptr->data[index] == '\0')
			    break;
			index = move_mbright(fileptr->data, index);
		    }
		    continue;
		}

		/* There is no end on this line.  But maybe on later lines? */
		end_line = fileptr->next;

		while (end_line != NULL && regexec(varnish->end, end_line->data,
					0, NULL, 0) == REG_NOMATCH)
		    end_line = end_line->next;

		/* If there is no end, we're done with this regex. */
		if (end_line == NULL) {
		    fileptr->multidata[varnish->id] = CWOULDBE;
		    break;
>>>>>>> origin/tomato-shibby-RT-AC
		}

		/* Paint the rest of the line, and we're done. */
		mvwaddnstr(edit, row, margin + start_col, thetext, -1);
		fileptr->multidata[varnish->id] = CENDAFTER;
		break;
	    }
<<<<<<< HEAD

	    wattroff(edit, A_BOLD);
	    wattroff(edit, COLOR_PAIR(tmpcolor->pairnum));
=======
  tail_of_loop:
	    wattroff(edit, varnish->attributes);
>>>>>>> origin/tomato-shibby-RT-AC
	}
    }
#endif /* ENABLE_COLOR */

#ifndef NANO_TINY
<<<<<<< HEAD
    /* If the mark is on, we need to display it. */
    if (openfile->mark_set && (fileptr->lineno <=
	openfile->mark_begin->lineno || fileptr->lineno <=
	openfile->current->lineno) && (fileptr->lineno >=
	openfile->mark_begin->lineno || fileptr->lineno >=
	openfile->current->lineno)) {
	/* fileptr is at least partially selected. */
	const filestruct *top;
	    /* Either current or mark_begin, whichever is first. */
	size_t top_x;
	    /* current_x or mark_begin_x, corresponding to top. */
	const filestruct *bot;
	size_t bot_x;
	int x_start;
	    /* Starting column for mvwaddnstr().  Zero-based. */
	int paintlen;
	    /* Number of characters to paint on this line.  There are
	     * COLS characters on a whole line. */
	size_t index;
	    /* Index in converted where we paint. */

	mark_order(&top, &top_x, &bot, &bot_x, NULL);

	if (top->lineno < fileptr->lineno || top_x < startpos)
	    top_x = startpos;
	if (bot->lineno > fileptr->lineno || bot_x > endpos)
	    bot_x = endpos;

	/* The selected bit of fileptr is on this page. */
	if (top_x < endpos && bot_x > startpos) {
	    assert(startpos <= top_x);

	    /* x_start is the expanded location of the beginning of the
	     * mark minus the beginning of the page. */
	    x_start = strnlenpt(fileptr->data, top_x) - start;

	    /* If the end of the mark is off the page, paintlen is -1,
	     * meaning that everything on the line gets painted.
	     * Otherwise, paintlen is the expanded location of the end
	     * of the mark minus the expanded location of the beginning
	     * of the mark. */
	    if (bot_x >= endpos)
		paintlen = -1;
	    else
		paintlen = strnlenpt(fileptr->data, bot_x) - (x_start +
			start);

	    /* If x_start is before the beginning of the page, shift
	     * paintlen x_start characters to compensate, and put
	     * x_start at the beginning of the page. */
	    if (x_start < 0) {
		paintlen += x_start;
		x_start = 0;
	    }
=======
    /* If the mark is on, and fileptr is at least partially selected, we
     * need to paint it. */
    if (openfile->mark_set &&
		(fileptr->lineno <= openfile->mark_begin->lineno ||
		fileptr->lineno <= openfile->current->lineno) &&
		(fileptr->lineno >= openfile->mark_begin->lineno ||
		fileptr->lineno >= openfile->current->lineno)) {
	const filestruct *top, *bot;
	    /* The lines where the marked region begins and ends. */
	size_t top_x, bot_x;
	    /* The x positions where the marked region begins and ends. */
	int start_col;
	    /* The column where painting starts.  Zero-based. */
	const char *thetext;
	    /* The place in converted from where painting starts. */
	int paintlen = -1;
	    /* The number of characters to paint.  Negative means "all". */

	mark_order(&top, &top_x, &bot, &bot_x, NULL);

	if (top->lineno < fileptr->lineno || top_x < from_x)
	    top_x = from_x;
	if (bot->lineno > fileptr->lineno || bot_x > till_x)
	    bot_x = till_x;
>>>>>>> origin/tomato-shibby-RT-AC

	/* Only paint if the marked part of the line is on this page. */
	if (top_x < till_x && bot_x > from_x) {
	    /* Compute on which screen column to start painting. */
	    start_col = strnlenpt(fileptr->data, top_x) - from_col;

	    if (start_col < 0)
		start_col = 0;

	    thetext = converted + actual_x(converted, start_col);

	    /* If the end of the mark is onscreen, compute how many
	     * characters to paint.  Otherwise, just paint all. */
	    if (bot_x < till_x) {
		size_t end_col = strnlenpt(fileptr->data, bot_x) - from_col;
		paintlen = actual_x(thetext, end_col - start_col);
	    }

<<<<<<< HEAD
	    wattron(edit, reverse_attr);
	    mvwaddnstr(edit, line, x_start, converted + index,
		paintlen);
	    wattroff(edit, reverse_attr);
=======
	    wattron(edit, hilite_attribute);
	    mvwaddnstr(edit, row, margin + start_col, thetext, paintlen);
	    wattroff(edit, hilite_attribute);
>>>>>>> origin/tomato-shibby-RT-AC
	}
    }
#endif /* !NANO_TINY */
}

<<<<<<< HEAD
/* Just update one line in the edit buffer.  This is basically a wrapper
 * for edit_draw().  The line will be displayed starting with
 * fileptr->data[index].  Likely arguments are current_x or zero.
 * Returns: Number of additiona lines consumed (needed for SOFTWRAP)
 */
int update_line(filestruct *fileptr, size_t index)
{
    int line = 0;
    int extralinesused = 0;
	/* The line in the edit window that we want to update. */
    char *converted;
	/* fileptr->data converted to have tabs and control characters
	 * expanded. */
    size_t page_start;
    filestruct *tmp;

    assert(fileptr != NULL);

    if (ISSET(SOFTWRAP)) {
	for (tmp = openfile->edittop; tmp && tmp != fileptr; tmp = tmp->next) {
	    line += 1 + (strlenpt(tmp->data) / COLS);
	}
    } else
	line = fileptr->lineno - openfile->edittop->lineno;
=======
/* Redraw the line at fileptr.  The line will be displayed so that the
 * character with the given index is visible -- if necessary, the line
 * will be horizontally scrolled.  In softwrap mode, however, the entire
 * line will be passed to update_softwrapped_line().  Likely values of
 * index are current_x or zero.  Return the number of additional rows
 * consumed (when softwrapping). */
int update_line(filestruct *fileptr, size_t index)
{
    int row = 0;
	/* The row in the edit window we will be updating. */
    char *converted;
	/* The data of the line with tabs and control characters expanded. */
    size_t from_col = 0;
	/* From which column a horizontally scrolled line is displayed. */

#ifndef NANO_TINY
    if (ISSET(SOFTWRAP))
	return update_softwrapped_line(fileptr);
#endif
>>>>>>> origin/tomato-shibby-RT-AC

    row = fileptr->lineno - openfile->edittop->lineno;

    /* If the line is offscreen, don't even try to display it. */
    if (row < 0 || row >= editwinrows) {
	statusline(ALERT, "Badness: tried to display a line on row %i"
				" -- please report a bug", row);
	return 0;
    }

<<<<<<< HEAD
    /* Next, convert variables that index the line to their equivalent
     * positions in the expanded line. */
    if (ISSET(SOFTWRAP))
	index = 0;
    else
	index = strnlenpt(fileptr->data, index);
    page_start = get_page_start(index);

    /* Expand the line, replacing tabs with spaces, and control
     * characters with their displayed forms. */
    converted = display_string(fileptr->data, page_start, COLS, !ISSET(SOFTWRAP));

#ifdef DEBUG
    if (ISSET(SOFTWRAP) && strlen(converted) >= COLS - 2)
	    fprintf(stderr, "update_line(): converted(1) line = %s\n", converted);
#endif

=======
    /* First, blank out the row. */
    blank_row(edit, row, 0, COLS);

    /* Next, find out from which column to start displaying the line. */
    from_col = get_page_start(strnlenpt(fileptr->data, index));

    /* Expand the line, replacing tabs with spaces, and control
     * characters with their displayed forms. */
    converted = display_string(fileptr->data, from_col, editwincols, TRUE);
>>>>>>> origin/tomato-shibby-RT-AC

    /* Draw the line. */
    edit_draw(fileptr, converted, row, from_col);
    free(converted);

<<<<<<< HEAD
    if (!ISSET(SOFTWRAP)) {
	if (page_start > 0)
	    mvwaddch(edit, line, 0, '$');
	if (strlenpt(fileptr->data) > page_start + COLS)
	    mvwaddch(edit, line, COLS - 1, '$');
    } else {
        int full_length = strlenpt(fileptr->data);
	for (index += COLS; index <= full_length && line < editwinrows; index += COLS) {
	    line++;
#ifdef DEBUG
	    fprintf(stderr, "update_line(): Softwrap code, moving to %d index %lu\n", line, (unsigned long) index);
#endif
 	    blank_line(edit, line, 0, COLS);

	    /* Expand the line, replacing tabs with spaces, and control
 	     * characters with their displayed forms. */
	    converted = display_string(fileptr->data, index, COLS, !ISSET(SOFTWRAP));
#ifdef DEBUG
	    if (ISSET(SOFTWRAP) && strlen(converted) >= COLS - 2)
		fprintf(stderr, "update_line(): converted(2) line = %s\n", converted);
#endif

	    /* Paint the line. */
	    edit_draw(fileptr, converted, line, index);
 	    free(converted);
	    extralinesused++;
	}
    }
    return extralinesused;
=======
    if (from_col > 0)
	mvwaddch(edit, row, margin, '$');
    if (strlenpt(fileptr->data) > from_col + editwincols)
	mvwaddch(edit, row, COLS - 1, '$');

    return 1;
}

#ifndef NANO_TINY
/* Redraw all the chunks of the given line (as far as they fit onscreen),
 * unless it's edittop, which will be displayed from column firstcolumn.
 * Return the number of additional rows consumed. */
int update_softwrapped_line(filestruct *fileptr)
{
    int row = 0;
	/* The row in the edit window we will write to. */
    filestruct *line = openfile->edittop;
	/* An iterator needed to find the relevant row. */
    int starting_row;
	/* The first row in the edit window that gets updated. */
    size_t from_col = 0;
	/* The starting column of the current chunk. */
    char *converted;
	/* The data of the chunk with tabs and control characters expanded. */
    size_t full_length;
	/* The length of the expanded line. */

    if (fileptr == openfile->edittop)
	from_col = openfile->firstcolumn;
    else
	row -= (openfile->firstcolumn / editwincols);

    /* Find out on which screen row the target line should be shown. */
    while (line != fileptr && line != NULL) {
	row += (strlenpt(line->data) / editwincols) + 1;
	line = line->next;
    }

    /* If the first chunk is offscreen, don't even try to display it. */
    if (row < 0 || row >= editwinrows) {
	statusline(ALERT, "Badness: tried to display a chunk on row %i"
				" -- please report a bug", row);
	return 0;
    }

    full_length = strlenpt(fileptr->data);
    starting_row = row;

    while (from_col <= full_length && row < editwinrows) {
	blank_row(edit, row, 0, COLS);

	/* Convert the chunk to its displayable form and draw it. */
	converted = display_string(fileptr->data, from_col, editwincols, TRUE);
	edit_draw(fileptr, converted, row++, from_col);
	free(converted);

	from_col += editwincols;
    }

    return (row - starting_row);
>>>>>>> origin/tomato-shibby-RT-AC
}
#endif

<<<<<<< HEAD
/* Return TRUE if we need an update after moving horizontally, and FALSE
 * otherwise.  We need one if the mark is on or if pww_save and
 * placewewant are on different pages. */
bool need_horizontal_update(size_t pww_save)
{
    return
#ifndef NANO_TINY
	openfile->mark_set ||
#endif
	get_page_start(pww_save) !=
	get_page_start(openfile->placewewant);
}

/* Return TRUE if we need an update after moving vertically, and FALSE
 * otherwise.  We need one if the mark is on or if pww_save and
 * placewewant are on different pages. */
bool need_vertical_update(size_t pww_save)
=======
/* Check whether the mark is on, or whether old_column and new_column are on
 * different "pages" (in softwrap mode, only the former applies), which means
 * that the relevant line needs to be redrawn. */
bool line_needs_update(const size_t old_column, const size_t new_column)
>>>>>>> origin/tomato-shibby-RT-AC
{
#ifndef NANO_TINY
    if (openfile->mark_set)
	return TRUE;
    else
#endif
<<<<<<< HEAD
	get_page_start(pww_save) !=
	get_page_start(openfile->placewewant);
}

/* When edittop changes, try and figure out how many lines
 * we really have to work with (i.e. set maxrows)
 */
void compute_maxrows(void)
=======
	return (get_page_start(old_column) != get_page_start(new_column));
}

/* Try to move up nrows softwrapped chunks from the given line and the
 * given column (leftedge).  After moving, leftedge will be set to the
 * starting column of the current chunk.  Return the number of chunks we
 * couldn't move up, which will be zero if we completely succeeded. */
int go_back_chunks(int nrows, filestruct **line, size_t *leftedge)
>>>>>>> origin/tomato-shibby-RT-AC
{
    int i;

    /* Don't move more chunks than the window can hold. */
    if (nrows > editwinrows - 1)
	nrows = (editwinrows < 2) ? 1 : editwinrows - 1;

<<<<<<< HEAD
    maxrows = 0;
    for (n = 0; n < editwinrows && foo; n++) {
	maxrows ++;
	n += strlenpt(foo->data) / COLS;
	foo = foo->next;
    }
=======
#ifndef NANO_TINY
    if (ISSET(SOFTWRAP)) {
	size_t current_chunk = (*leftedge) / editwincols;
>>>>>>> origin/tomato-shibby-RT-AC

	/* Recede through the requested number of chunks. */
	for (i = nrows; i > 0; i--) {
	    if (current_chunk > 0) {
		current_chunk--;
		continue;
	    }

<<<<<<< HEAD
#ifdef DEBUG
    fprintf(stderr, "compute_maxrows(): maxrows = %ld\n", maxrows);
=======
	    if (*line == openfile->fileage)
		break;

	    *line = (*line)->prev;
	    current_chunk = strlenpt((*line)->data) / editwincols;
	}

	/* Only change leftedge when we actually could move. */
	if (i < nrows)
	    *leftedge = current_chunk * editwincols;
    } else
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	for (i = nrows; i > 0 && (*line)->prev != NULL; i--)
	    *line = (*line)->prev;

    return i;
}

<<<<<<< HEAD
/* Scroll the edit window in the given direction and the given number
 * of lines, and draw new lines on the blank lines left after the
 * scrolling.  direction is the direction to scroll, either UP_DIR or
 * DOWN_DIR, and nlines is the number of lines to scroll.  We change
 * edittop, and assume that current and current_x are up to date.  We
 * also assume that scrollok(edit) is FALSE. */
void edit_scroll(scroll_dir direction, ssize_t nlines)
{
    filestruct *foo;
    ssize_t i, extracuzsoft = 0;
    bool do_redraw = FALSE;

    /* Don't bother scrolling less than one line. */
    if (nlines < 1)
	return;

    if (need_vertical_update(0))
	do_redraw = TRUE;


    /* If using soft wrapping, we want to scroll down enough to display the entire next
        line, if possible... */
    if (ISSET(SOFTWRAP) && direction == DOWN_DIR) {
#ifdef DEBUG
	   fprintf(stderr, "Softwrap: Entering check for extracuzsoft\n");
#endif
	for (i = maxrows, foo = openfile->edittop; foo && i > 0; i--, foo = foo->next)
	    ;

	if (foo) {
	   extracuzsoft += strlenpt(foo->data) / COLS;
#ifdef DEBUG
	   fprintf(stderr, "Setting extracuzsoft to %lu due to strlen %lu of line %lu\n", (unsigned long) extracuzsoft,
		(unsigned long) strlenpt(foo->data), (unsigned long) foo->lineno);
#endif

	    /* Now account for whether the edittop line itself is >COLS, if scrolling down */
	   for (foo = openfile->edittop; foo && extracuzsoft > 0; nlines++) {
		extracuzsoft -= 1 + strlenpt(foo->data) / COLS;
#ifdef DEBUG
 		fprintf(stderr, "Edittop adjustment, setting nlines to %lu\n", (unsigned long) nlines);
#endif
		if (foo == openfile->filebot)
		    break;
		foo = foo->next;
	    }
	}
    }
=======
/* Try to move down nrows softwrapped chunks from the given line and the
 * given column (leftedge).  After moving, leftedge will be set to the
 * starting column of the current chunk.  Return the number of chunks we
 * couldn't move down, which will be zero if we completely succeeded. */
int go_forward_chunks(int nrows, filestruct **line, size_t *leftedge)
{
    int i;

    /* Don't move more chunks than the window can hold. */
    if (nrows > editwinrows - 1)
	nrows = (editwinrows < 2) ? 1 : editwinrows - 1;
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef NANO_TINY
    if (ISSET(SOFTWRAP)) {
	size_t current_chunk = (*leftedge) / editwincols;
	size_t last_chunk = strlenpt((*line)->data) / editwincols;

	/* Advance through the requested number of chunks. */
	for (i = nrows; i > 0; i--) {
	    if (current_chunk < last_chunk) {
		current_chunk++;
		continue;
	    }

<<<<<<< HEAD
    /* Move the top line of the edit window up or down (depending on the
     * value of direction) nlines lines, or as many lines as we can if
     * there are fewer than nlines lines available. */
    for (i = nlines; i > 0; i--) {
	if (direction == UP_DIR) {
	    if (openfile->edittop == openfile->fileage)
		break;
	    openfile->edittop = openfile->edittop->prev;
	} else {
	    if (openfile->edittop == openfile->filebot)
=======
	    if (*line == openfile->filebot)
>>>>>>> origin/tomato-shibby-RT-AC
		break;

	    *line = (*line)->next;
	    current_chunk = 0;
	    last_chunk = strlenpt((*line)->data) / editwincols;
	}
<<<<<<< HEAD
	/* Don't over-scroll on long lines */
	if (ISSET(SOFTWRAP)) {
	    ssize_t len = strlenpt(openfile->edittop->data) / COLS;
	    i -=  len;
	    if (len > 0)
		do_redraw = TRUE;
	}
    }
=======

	/* Only change leftedge when we actually could move. */
	if (i < nrows)
	    *leftedge = current_chunk * editwincols;
    } else
#endif
	for (i = nrows; i > 0 && (*line)->next != NULL; i--)
	    *line = (*line)->next;

    return i;
}

/* Return TRUE if there are fewer than a screen's worth of lines between
 * the line at line number was_lineno (and column was_leftedge, if we're
 * in softwrap mode) and the line at current[current_x]. */
bool less_than_a_screenful(size_t was_lineno, size_t was_leftedge)
{
#ifndef NANO_TINY
    if (ISSET(SOFTWRAP)) {
	filestruct *line = openfile->current;
	size_t leftedge = (xplustabs() / editwincols) * editwincols;
	int rows_left = go_back_chunks(editwinrows - 1, &line, &leftedge);

	return (rows_left > 0 || line->lineno < was_lineno ||
		(line->lineno == was_lineno && leftedge <= was_leftedge));
    } else
#endif
	return (openfile->current->lineno - was_lineno < editwinrows);
}

/* Scroll the edit window in the given direction and the given number of rows,
 * and draw new lines on the blank lines left after the scrolling. */
void edit_scroll(scroll_dir direction, int nrows)
{
    filestruct *line;
    size_t leftedge;

    /* Part 1: nrows is the number of rows we're going to scroll the text of
     * the edit window. */
>>>>>>> origin/tomato-shibby-RT-AC

    /* Move the top line of the edit window the requested number of rows up or
     * down, and reduce the number of rows with the amount we couldn't move. */
    if (direction == UPWARD)
	nrows -= go_back_chunks(nrows, &openfile->edittop, &openfile->firstcolumn);
    else
	nrows -= go_forward_chunks(nrows, &openfile->edittop, &openfile->firstcolumn);

<<<<<<< HEAD
    /* Don't bother scrolling zero lines or more than the number of
     * lines in the edit window minus one; in both cases, get out, and
     * call edit_refresh() beforehand if we need to. */
    if (nlines == 0 || do_redraw || nlines >= editwinrows) {
	if (do_redraw || nlines >= editwinrows)
	    edit_refresh_needed = TRUE;
=======
    /* Don't bother scrolling zero rows, nor more than the window can hold. */
    if (nrows == 0)
	return;
    if (nrows >= editwinrows) {
	refresh_needed = TRUE;
>>>>>>> origin/tomato-shibby-RT-AC
	return;
    }

    /* Scroll the text of the edit window a number of rows up or down. */
    scrollok(edit, TRUE);
<<<<<<< HEAD
    wscrl(edit, (direction == UP_DIR) ? -nlines : nlines);
    scrollok(edit, FALSE);

    /* Part 2: nlines is the number of lines in the scrolled region of
     * the edit window that we need to draw. */

    /* If the top or bottom line of the file is now visible in the edit
     * window, we need to draw the entire edit window. */
    if ((direction == UP_DIR && openfile->edittop ==
	openfile->fileage) || (direction == DOWN_DIR &&
	openfile->edittop->lineno + editwinrows - 1 >=
	openfile->filebot->lineno))
	nlines = editwinrows;

    /* If the scrolled region contains only one line, and the line
     * before it is visible in the edit window, we need to draw it too.
     * If the scrolled region contains more than one line, and the lines
     * before and after the scrolled region are visible in the edit
     * window, we need to draw them too. */
    nlines += (nlines == 1) ? 1 : 2;

    if (nlines > editwinrows)
	nlines = editwinrows;

    /* If we scrolled up, we're on the line before the scrolled
     * region. */
    foo = openfile->edittop;

    /* If we scrolled down, move down to the line before the scrolled
     * region. */
    if (direction == DOWN_DIR) {
	for (i = editwinrows - nlines; i > 0 && foo != NULL; i--)
	    foo = foo->next;
    }

    /* Draw new lines on any blank lines before or inside the scrolled
     * region.  If we scrolled down and we're on the top line, or if we
     * scrolled up and we're on the bottom line, the line won't be
     * blank, so we don't need to draw it unless the mark is on or we're
     * not on the first page. */
    for (i = nlines; i > 0 && foo != NULL; i--) {
	if ((i == nlines && direction == DOWN_DIR) || (i == 1 &&
		direction == UP_DIR)) {
	    if (do_redraw)
		update_line(foo, (foo == openfile->current) ?
			openfile->current_x : 0);
	} else
	    update_line(foo, (foo == openfile->current) ?
		openfile->current_x : 0);
	foo = foo->next;
    }
=======
    wscrl(edit, (direction == UPWARD) ? -nrows : nrows);
    scrollok(edit, FALSE);

    /* Part 2: nrows is now the number of rows in the scrolled region of the
     * edit window that we need to draw. */

    /* If we're not on the first "page" (when not softwrapping), or the mark
     * is on, the row next to the scrolled region needs to be redrawn too. */
    if (line_needs_update(openfile->placewewant, 0) && nrows < editwinrows)
	nrows++;

    /* If we scrolled backward, start on the first line of the blank region. */
    line = openfile->edittop;
    leftedge = openfile->firstcolumn;

    /* If we scrolled forward, move down to the start of the blank region. */
    if (direction == DOWNWARD)
	go_forward_chunks(editwinrows - nrows, &line, &leftedge);

#ifndef NANO_TINY
    /* Compensate for the earlier onscreen chunks of a softwrapped line
     * when the first blank row happens to be in the middle of that line. */
    if (ISSET(SOFTWRAP) && line != openfile->edittop)
	nrows += leftedge / editwincols;
#endif

    /* Draw new content on the blank rows inside the scrolled region
     * (and on the bordering row too when it was deemed necessary). */
    while (nrows > 0 && line != NULL) {
	nrows -= update_line(line, (line == openfile->current) ?
					openfile->current_x : 0);
	line = line->next;
    }
}

/* Ensure that firstcolumn is at the starting column of the softwrapped chunk
 * it's on.  We need to do this when the number of columns of the edit window
 * has changed, because then the width of softwrapped chunks has changed. */
void ensure_firstcolumn_is_aligned(void)
{
#ifndef NANO_TINY
    if (openfile->firstcolumn % editwincols != 0)
	openfile->firstcolumn -= (openfile->firstcolumn % editwincols);
#endif
}

/* Return TRUE if current[current_x] is above the top of the screen, and FALSE
 * otherwise. */
bool current_is_above_screen(void)
{
#ifndef NANO_TINY
    if (ISSET(SOFTWRAP))
	/* The cursor is above screen when current[current_x] is before edittop
	 * at column firstcolumn. */
	return (openfile->current->lineno < openfile->edittop->lineno ||
		(openfile->current->lineno == openfile->edittop->lineno &&
		xplustabs() < openfile->firstcolumn));
    else
#endif
	return (openfile->current->lineno < openfile->edittop->lineno);
}

/* Return TRUE if current[current_x] is below the bottom of the screen, and
 * FALSE otherwise. */
bool current_is_below_screen(void)
{
#ifndef NANO_TINY
    if (ISSET(SOFTWRAP)) {
	filestruct *line = openfile->edittop;
	size_t leftedge = openfile->firstcolumn;

	/* If current[current_x] is more than a screen's worth of lines after
	 * edittop at column firstcolumn, it's below the screen. */
	return (go_forward_chunks(editwinrows - 1, &line, &leftedge) == 0 &&
			(line->lineno < openfile->current->lineno ||
			(line->lineno == openfile->current->lineno &&
			leftedge < (xplustabs() / editwincols) * editwincols)));
    } else
#endif
	return (openfile->current->lineno >=
			openfile->edittop->lineno + editwinrows);
}

/* Return TRUE if current[current_x] is offscreen relative to edittop, and
 * FALSE otherwise. */
bool current_is_offscreen(void)
{
    return (current_is_above_screen() || current_is_below_screen());
>>>>>>> origin/tomato-shibby-RT-AC
}

/* Update any lines between old_current and current that need to be
 * updated.  Use this if we've moved without changing any text. */
void edit_redraw(filestruct *old_current, size_t pww_save)
{
    bool do_redraw = need_vertical_update(0) ||
	need_vertical_update(pww_save);
    filestruct *foo = NULL;

    /* If either old_current or current is offscreen, scroll the edit
     * window until it's onscreen and get out. */
    if (old_current->lineno < openfile->edittop->lineno ||
	old_current->lineno >= openfile->edittop->lineno +
	maxrows || openfile->current->lineno <
	openfile->edittop->lineno || openfile->current->lineno >=
	openfile->edittop->lineno + maxrows) {

<<<<<<< HEAD
#ifdef DEBUG
    fprintf(stderr, "edit_redraw(): line %lu was offscreen, oldcurrent = %lu edittop = %lu", openfile->current->lineno,
                    old_current->lineno, openfile->edittop->lineno);
#endif
	filestruct *old_edittop = openfile->edittop;
	ssize_t nlines;

#ifndef NANO_TINY
	/* If the mark is on, update all the lines between old_current
	 * and either the old first line or old last line (depending on
	 * whether we've scrolled up or down) of the edit window. */
	if (openfile->mark_set) {
	    ssize_t old_lineno;

	    if (old_edittop->lineno < openfile->edittop->lineno)
		old_lineno = old_edittop->lineno;
	    else
		old_lineno = (old_edittop->lineno + maxrows <=
			openfile->filebot->lineno) ?
			old_edittop->lineno + editwinrows :
			openfile->filebot->lineno;

	    foo = old_current;

	    while (foo->lineno != old_lineno) {
		update_line(foo, 0);

		foo = (foo->lineno > old_lineno) ? foo->prev :
			foo->next;
	    }
	}
#endif /* !NANO_TINY */

	/* Put edittop in range of current, get the difference in lines
	 * between the original edittop and the current edittop, and
	 * then restore the original edittop. */
	edit_update(CENTER);

	/* Update old_current if we're not on the same page as
	 * before. */
	if (do_redraw)
	    update_line(old_current, 0);

#ifndef NANO_TINY
	/* If the mark is on, update all the lines between the old first
	 * line or old last line of the edit window (depending on
	 * whether we've scrolled up or down) and current. */
	if (openfile->mark_set) {
	    while (foo->lineno != openfile->current->lineno) {
		update_line(foo, 0);

		foo = (foo->lineno > openfile->current->lineno) ?
			foo->prev : foo->next;
	    }
=======
    openfile->placewewant = xplustabs();

    /* If the current line is offscreen, scroll until it's onscreen. */
    if (current_is_offscreen()) {
	adjust_viewport((focusing || !ISSET(SMOOTH_SCROLL)) ? CENTERING : FLOWING);
	refresh_needed = TRUE;
	return;
    }

#ifndef NANO_TINY
    /* If the mark is on, update all lines between old_current and current. */
    if (openfile->mark_set) {
	filestruct *line = old_current;

	while (line != openfile->current) {
	    update_line(line, 0);

	    line = (line->lineno > openfile->current->lineno) ?
			line->prev : line->next;
>>>>>>> origin/tomato-shibby-RT-AC
	}
#endif /* !NANO_TINY */

	return;
    }

    /* Update old_current and current if we're not on the same page as
     * before.  If the mark is on, update all the lines between
     * old_current and current too. */
    foo = old_current;

    while (foo != openfile->current) {
	if (do_redraw)
	    update_line(foo, 0);

#ifndef NANO_TINY
	if (!openfile->mark_set)
#endif
	    break;

<<<<<<< HEAD
#ifndef NANO_TINY
	foo = (foo->lineno > openfile->current->lineno) ? foo->prev :
		foo->next;
#endif
    }

    if (do_redraw)
=======
    /* Update current if the mark is on or it has changed "page", or if it
     * differs from old_current and needs to be horizontally scrolled. */
    if (line_needs_update(was_pww, openfile->placewewant) ||
			(old_current != openfile->current &&
			get_page_start(openfile->placewewant) > 0))
>>>>>>> origin/tomato-shibby-RT-AC
	update_line(openfile->current, openfile->current_x);
}

/* Refresh the screen without changing the position of lines.  Use this
 * if we've moved and changed text. */
void edit_refresh(void)
{
<<<<<<< HEAD
    filestruct *foo;
    int nlines;

    /* Figure out what maxrows should really be */
    compute_maxrows();

    if (openfile->current->lineno < openfile->edittop->lineno ||
	openfile->current->lineno >= openfile->edittop->lineno +
	maxrows) {

#ifdef DEBUG
    fprintf(stderr, "edit_refresh(): line = %d, edittop %d + maxrows %d\n", openfile->current->lineno, openfile->edittop->lineno, maxrows);
#endif

	/* Put the top line of the edit window in range of the current
	 * line. */
	edit_update(CENTER);
=======
    filestruct *line;
    int row = 0;

    /* If the current line is out of view, get it back on screen. */
    if (current_is_offscreen()) {
#ifdef DEBUG
	fprintf(stderr, "edit-refresh: line = %ld, edittop = %ld and editwinrows = %d\n",
		(long)openfile->current->lineno, (long)openfile->edittop->lineno, editwinrows);
#endif
	adjust_viewport((focusing || !ISSET(SMOOTH_SCROLL)) ? CENTERING : STATIONARY);
>>>>>>> origin/tomato-shibby-RT-AC
    }

#ifdef DEBUG
    fprintf(stderr, "edit-refresh: now edittop = %ld\n", (long)openfile->edittop->lineno);
#endif

    line = openfile->edittop;

    while (row < editwinrows && line != NULL) {
	if (line == openfile->current)
	    row += update_line(line, openfile->current_x);
	else
	    row += update_line(line, 0);
	line = line->next;
    }

    while (row < editwinrows)
	blank_row(edit, row++, 0, COLS);

    reset_cursor();
    wnoutrefresh(edit);

    refresh_needed = FALSE;
}

<<<<<<< HEAD
/* Move edittop to put it in range of current, keeping current in the
 * same place.  location determines how we move it: if it's CENTER, we
 * center current, and if it's NONE, we put current current_y lines
 * below edittop. */
void edit_update(update_type location)
{
    filestruct *foo = openfile->current;
    int goal;

    /* If location is CENTER, we move edittop up (editwinrows / 2)
     * lines.  This puts current at the center of the screen.  If
     * location is NONE, we move edittop up current_y lines if current_y
     * is in range of the screen, 0 lines if current_y is less than 0,
     * or (editwinrows - 1) lines if current_y is greater than
     * (editwinrows - 1).  This puts current at the same place on the
     * screen as before, or at the top or bottom of the screen if
     * edittop is beyond either. */
    if (location == CENTER)
	goal = editwinrows / 2;
    else {
=======
/* Move edittop so that current is on the screen.  manner says how it
 * should be moved: CENTERING means that current should end up in the
 * middle of the screen, STATIONARY means that it should stay at the
 * same vertical position, and FLOWING means that it should scroll no
 * more than needed to bring current into view. */
void adjust_viewport(update_type manner)
{
    int goal = 0;

    /* If manner is CENTERING, move edittop half the number of window rows
     * back from current.  If manner is FLOWING, move edittop back 0 rows
     * or (editwinrows - 1) rows, depending on where current has moved.
     * This puts the cursor on the first or the last row.  If manner is
     * STATIONARY, move edittop back current_y rows if current_y is in range
     * of the screen, 0 rows if current_y is below zero, or (editwinrows - 1)
     * rows if current_y is too big.  This puts current at the same place on
     * the screen as before, or... at some undefined place. */
    if (manner == CENTERING)
	goal = editwinrows / 2;
    else if (manner == FLOWING) {
	if (!current_is_above_screen())
	    goal = editwinrows - 1;
    } else {
>>>>>>> origin/tomato-shibby-RT-AC
	goal = openfile->current_y;

	/* Limit goal to (editwinrows - 1) rows maximum. */
	if (goal > editwinrows - 1)
	    goal = editwinrows - 1;
    }

<<<<<<< HEAD
    for (; goal > 0 && foo->prev != NULL; goal--) {
	foo = foo->prev;
	if (ISSET(SOFTWRAP) && foo)
	    goal -= strlenpt(foo->data) / COLS;
    }
    openfile->edittop = foo;
#ifdef DEBUG
    fprintf(stderr, "edit_udpate(), setting edittop to lineno %d\n", openfile->edittop->lineno);
#endif
    compute_maxrows();
    edit_refresh_needed = TRUE;
=======
    openfile->edittop = openfile->current;

#ifndef NANO_TINY
    if (ISSET(SOFTWRAP))
	openfile->firstcolumn = (xplustabs() / editwincols) * editwincols;
#endif

    /* Move edittop back goal rows, starting at current[current_x]. */
    go_back_chunks(goal, &openfile->edittop, &openfile->firstcolumn);

#ifdef DEBUG
    fprintf(stderr, "adjust_viewport(): setting edittop to lineno %ld\n", (long)openfile->edittop->lineno);
#endif
>>>>>>> origin/tomato-shibby-RT-AC
}

/* Unconditionally redraw the entire screen. */
void total_redraw(void)
{
#ifdef USE_SLANG
    /* Slang curses emulation brain damage, part 4: Slang doesn't define
     * curscr. */
    SLsmg_touch_screen();
    SLsmg_refresh();
#else
    wrefresh(curscr);
#endif
}

/* Unconditionally redraw the entire screen, and then refresh it using
 * the current file. */
void total_refresh(void)
{
    total_redraw();
    titlebar(NULL);
    edit_refresh();
    bottombars(currmenu);
}

/* Display the main shortcut list on the last two rows of the bottom
 * portion of the window. */
void display_main_list(void)
{
    bottombars(MMAIN);
}

<<<<<<< HEAD
/* If constant is TRUE, we display the current cursor position only if
 * disable_cursorpos is FALSE.  Otherwise, we display it
 * unconditionally and set disable_cursorpos to FALSE.  If constant is
 * TRUE and disable_cursorpos is TRUE, we also set disable_cursorpos to
 * FALSE, so that we leave the current statusbar alone this time, and
 * display the current cursor position next time. */
void do_cursorpos(bool constant)
=======
/* Show info about the current cursor position on the statusbar.
 * Do this unconditionally when force is TRUE; otherwise, only if
 * suppress_cursorpos is FALSE.  In any case, reset the latter. */
void do_cursorpos(bool force)
>>>>>>> origin/tomato-shibby-RT-AC
{
    char saved_byte;
    size_t sum, cur_xpt = xplustabs() + 1;
    size_t cur_lenpt = strlenpt(openfile->current->data) + 1;
    int linepct, colpct, charpct;

    assert(openfile->fileage != NULL && openfile->current != NULL);

<<<<<<< HEAD
    f = openfile->current->next;
    c = openfile->current->data[openfile->current_x];

    openfile->current->next = NULL;
=======
    /* Determine the size of the file up to the cursor. */
    saved_byte = openfile->current->data[openfile->current_x];
>>>>>>> origin/tomato-shibby-RT-AC
    openfile->current->data[openfile->current_x] = '\0';

    sum = get_totsize(openfile->fileage, openfile->current);

    openfile->current->data[openfile->current_x] = saved_byte;

    /* When not at EOF, subtract 1 for an overcounted newline. */
    if (openfile->current != openfile->filebot)
	sum--;

<<<<<<< HEAD
    if (constant && disable_cursorpos) {
	disable_cursorpos = FALSE;
=======
    /* If the showing needs to be suppressed, don't suppress it next time. */
    if (suppress_cursorpos && !force) {
	suppress_cursorpos = FALSE;
>>>>>>> origin/tomato-shibby-RT-AC
	return;
    }

    /* Display the current cursor position on the statusbar, and set
     * disable_cursorpos to FALSE. */
    linepct = 100 * openfile->current->lineno /
	openfile->filebot->lineno;
    colpct = 100 * cur_xpt / cur_lenpt;
<<<<<<< HEAD
    charpct = (openfile->totsize == 0) ? 0 : 100 * i /
	openfile->totsize;
=======
    charpct = (openfile->totsize == 0) ? 0 : 100 * sum / openfile->totsize;
>>>>>>> origin/tomato-shibby-RT-AC

    statusbar(
	_("line %ld/%ld (%d%%), col %lu/%lu (%d%%), char %lu/%lu (%d%%)"),
	(long)openfile->current->lineno,
	(long)openfile->filebot->lineno, linepct,
	(unsigned long)cur_xpt, (unsigned long)cur_lenpt, colpct,
	(unsigned long)sum, (unsigned long)openfile->totsize, charpct);

    disable_cursorpos = FALSE;
}

/* Unconditionally display the current cursor position. */
void do_cursorpos_void(void)
{
    do_cursorpos(TRUE);
}

void enable_nodelay(void)
{
   nodelay_mode = TRUE;
   nodelay(edit, TRUE);
}

void disable_nodelay(void)
{
   nodelay_mode = FALSE;
   nodelay(edit, FALSE);
}


/* Highlight the current word being replaced or spell checked.  We
 * expect word to have tabs and control characters expanded. */
void do_replace_highlight(bool highlight, const char *word)
{
<<<<<<< HEAD
    size_t y = xplustabs(), word_len = strlenpt(word);

    y = get_page_start(y) + COLS - y;
	/* Now y is the number of columns that we can display on this
	 * line. */

    assert(y > 0);

    if (word_len > y)
	y--;
=======
    size_t word_span = strlenpt(word);
    size_t room = word_span;

    /* Compute the number of columns that are available for the word. */
    if (!ISSET(SOFTWRAP)) {
	room = editwincols + get_page_start(xplustabs()) - xplustabs();

	/* If the word is partially offscreen, reserve space for the "$". */
	if (word_span > room)
	    room--;
    }
>>>>>>> origin/tomato-shibby-RT-AC

    reset_cursor();

    if (highlight)
	wattron(edit, reverse_attr);

    /* This is so we can show zero-length matches. */
    if (word_span == 0)
	waddch(edit, ' ');
    else
	waddnstr(edit, word, actual_x(word, y));

<<<<<<< HEAD
    if (word_len > y)
	waddch(edit, '$');

    if (highlight)
	wattroff(edit, reverse_attr);
=======
    if (word_span > room)
	waddch(edit, '$');

    if (active)
	wattroff(edit, hilite_attribute);

    wnoutrefresh(edit);
>>>>>>> origin/tomato-shibby-RT-AC
}

#ifdef NANO_EXTRA
#define CREDIT_LEN 55
#define XLCREDIT_LEN 8

/* Easter egg: Display credits.  Assume nodelay(edit) and scrollok(edit)
 * are FALSE. */
void do_credits(void)
{
    bool old_more_space = ISSET(MORE_SPACE);
    bool old_no_help = ISSET(NO_HELP);
    int kbinput = ERR, crpos = 0, xlpos = 0;
    const char *credits[CREDIT_LEN] = {
	NULL,				/* "The nano text editor" */
	NULL,				/* "version" */
	VERSION,
	"",
	NULL,				/* "Brought to you by:" */
	"Chris Allegretta",
	"Jordi Mallach",
	"Adam Rogoyski",
	"Rob Siemborski",
	"Rocco Corsi",
	"David Lawrence Ramsey",
	"David Benbennick",
	"Mike Frysinger",
	"Ken Tyler",
	"Sven Guckes",
	NULL,				/* credits[15], handled below. */
	"Pauli Virtanen",
	"Daniele Medri",
	"Clement Laforet",
	"Tedi Heriyanto",
	"Bill Soudan",
	"Christian Weisgerber",
	"Erik Andersen",
	"Big Gaute",
	"Joshua Jensen",
	"Ryan Krebs",
	"Albert Chin",
	"",
	NULL,				/* "Special thanks to:" */
	"Plattsburgh State University",
	"Benet Laboratories",
	"Amy Allegretta",
	"Linda Young",
	"Jeremy Robichaud",
	"Richard Kolb II",
	NULL,				/* "The Free Software Foundation" */
	"Linus Torvalds",
	NULL,				/* "For ncurses:" */
	"Thomas Dickey",
	"Pavel Curtis",
	"Zeyd Ben-Halim",
	"Eric S. Raymond",
	NULL,				/* "and anyone else we forgot..." */
	NULL,				/* "Thank you for using nano!" */
	"",
	"",
	"",
	"",
	"(C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007",
	"Free Software Foundation, Inc.",
	"",
	"",
	"",
	"",
	"http://www.nano-editor.org/"
    };

    const char *xlcredits[XLCREDIT_LEN] = {
	N_("The nano text editor"),
	N_("version"),
	N_("Brought to you by:"),
	N_("Special thanks to:"),
	N_("The Free Software Foundation"),
	N_("For ncurses:"),
	N_("and anyone else we forgot..."),
	N_("Thank you for using nano!")
    };

    /* credits[15]: Make sure this name is displayed properly, since we
     * can't dynamically assign it above, using Unicode 00F6 (Latin
     * Small Letter O with Diaresis) if applicable. */
    credits[15] =
#ifdef ENABLE_UTF8
	 using_utf8() ? "Florian K\xC3\xB6nig" :
#endif
	"Florian K\xF6nig";

    if (!old_more_space || !old_no_help) {
	SET(MORE_SPACE);
	SET(NO_HELP);
	window_init();
    }

    curs_set(0);
    nodelay(edit, TRUE);

    blank_titlebar();
    blank_edit();
    blank_statusbar();

    wrefresh(topwin);
    wrefresh(edit);
    wrefresh(bottomwin);
    napms(700);

    for (crpos = 0; crpos < CREDIT_LEN + editwinrows / 2; crpos++) {
	if ((kbinput = wgetch(edit)) != ERR)
	    break;

	if (crpos < CREDIT_LEN) {
	    const char *what;
	    size_t start_col;

	    if (credits[crpos] == NULL) {
		assert(0 <= xlpos && xlpos < XLCREDIT_LEN);

		what = _(xlcredits[xlpos]);
		xlpos++;
	    } else
		what = credits[crpos];

	    start_col = COLS / 2 - strlenpt(what) / 2 - 1;
	    mvwaddstr(edit, editwinrows - 1 - (editwinrows % 2),
						start_col, what);
	}

	wrefresh(edit);

	if ((kbinput = wgetch(edit)) != ERR)
	    break;
	napms(700);

	scrollok(edit, TRUE);
	wscrl(edit, 1);
	scrollok(edit, FALSE);
	wrefresh(edit);

	if ((kbinput = wgetch(edit)) != ERR)
	    break;
	napms(700);

	scrollok(edit, TRUE);
	wscrl(edit, 1);
	scrollok(edit, FALSE);
	wrefresh(edit);
    }

    if (kbinput != ERR)
	ungetch(kbinput);

    if (!old_more_space || !old_no_help) {
	UNSET(MORE_SPACE);
	UNSET(NO_HELP);
	window_init();
    }

    curs_set(1);
    nodelay(edit, FALSE);

    total_refresh();
}
#endif /* NANO_EXTRA */
