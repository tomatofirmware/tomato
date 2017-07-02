/* $Id: cut.c 4453 2009-12-02 03:36:22Z astyanax $ */
/**************************************************************************
 *   cut.c  --  This file is part of GNU nano.                            *
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
 *   Copyright (C) 2014 Mark Majeres                                      *
 *   Copyright (C) 2016 Benno Schulenberg                                 *
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

#include <string.h>
#include <stdio.h>

static bool keep_cutbuffer = FALSE;
	/* Should we keep the contents of the cutbuffer? */
	/* Pointer to the end of the cutbuffer. */

/* Indicate that we should no longer keep the contents of the
 * cutbuffer. */
void cutbuffer_reset(void)
{
    keep_cutbuffer = FALSE;
}

/* If we aren't on the last line of the file, move all the text of the
 * current line, plus the newline at the end, into the cutbuffer.  If we
 * are, move all of the text of the current line into the cutbuffer.  In
 * both cases, set the current place we want to the beginning of the
 * current line. */
void cut_line(void)
{
    if (openfile->current != openfile->filebot)
	extract_buffer(&cutbuffer, &cutbottom, openfile->current, 0,
		openfile->current->next, 0);
    else
	extract_buffer(&cutbuffer, &cutbottom, openfile->current, 0,
		openfile->current, strlen(openfile->current->data));
    openfile->placewewant = 0;
}

#ifndef NANO_TINY
/* Move all currently marked text into the cutbuffer, and set the
 * current place we want to where the text used to start. */
void cut_marked(bool *right_side_up)
{
    filestruct *top, *bot;
    size_t top_x, bot_x;

    mark_order((const filestruct **)&top, &top_x,
<<<<<<< HEAD
	(const filestruct **)&bot, &bot_x, NULL);
=======
		(const filestruct **)&bot, &bot_x, right_side_up);
>>>>>>> origin/tomato-shibby-RT-AC

    extract_buffer(&cutbuffer, &cutbottom, top, top_x, bot, bot_x);
    openfile->placewewant = xplustabs();
}

/* If we aren't at the end of the current line, move all the text from
 * the current cursor position to the end of the current line, not
 * counting the newline at the end, into the cutbuffer.  If we are, and
 * we're not on the last line of the file, move the newline at the end
 * into the cutbuffer, and set the current place we want to where the
 * newline used to be. */
void cut_to_eol(void)
{
    size_t data_len = strlen(openfile->current->data);

    if (openfile->current_x < data_len)
	/* If we're not at the end of the line, move all the text from
	 * the current position up to it, not counting the newline at
	 * the end, into the cutbuffer. */
	extract_buffer(&cutbuffer, &cutbottom, openfile->current,
		openfile->current_x, openfile->current, data_len);
    else if (openfile->current != openfile->filebot) {
	/* If we're at the end of the line, and it isn't the last line
	 * of the file, move all the text from the current position up
	 * to the beginning of the next line, i.e. the newline at the
	 * end, into the cutbuffer. */
	extract_buffer(&cutbuffer, &cutbottom, openfile->current,
		openfile->current_x, openfile->current->next, 0);
	openfile->placewewant = xplustabs();
    }
}

/* Move all the text from the current cursor position to the end of the
 * file into the cutbuffer. */
void cut_to_eof(void)
{
<<<<<<< HEAD
    move_to_filestruct(&cutbuffer, &cutbottom, openfile->current,
	openfile->current_x, openfile->filebot,
	strlen(openfile->filebot->data));
=======
    extract_buffer(&cutbuffer, &cutbottom,
		openfile->current, openfile->current_x,
		openfile->filebot, strlen(openfile->filebot->data));
>>>>>>> origin/tomato-shibby-RT-AC
}
#endif /* !NANO_TINY */

/* Move text from the current filestruct into the cutbuffer.  If
 * copy_text is TRUE, copy the text back into the filestruct afterward.
 * If cut_till_end is TRUE, move all text from the current cursor
 * position to the end of the file into the cutbuffer. */
<<<<<<< HEAD
void do_cut_text(
#ifndef NANO_TINY
	bool copy_text, bool cut_till_end, bool undoing
#else
	void
#endif
	)
=======
void do_cut_text(bool copy_text, bool cut_till_eof)
>>>>>>> origin/tomato-shibby-RT-AC
{
#ifndef NANO_TINY
    filestruct *cb_save = NULL;
	/* The current end of the cutbuffer, before we add text to
	 * it. */
    size_t cb_save_len = 0;
	/* The length of the string at the current end of the cutbuffer,
	 * before we add text to it.  */
    bool old_no_newlines = ISSET(NO_NEWLINES);
    bool right_side_up = TRUE;
	/* There *is* no region, *or* it is marked forward. */
#endif
    size_t was_totsize = openfile->totsize;

<<<<<<< HEAD
    assert(openfile->current != NULL && openfile->current->data != NULL);

    /* If keep_cutbuffer is FALSE and the cutbuffer isn't empty, blow
     * away the text in the cutbuffer. */
    if (!keep_cutbuffer && cutbuffer != NULL) {
=======
    /* If a chain of cuts was broken, empty the cutbuffer. */
    if (!keep_cutbuffer) {
>>>>>>> origin/tomato-shibby-RT-AC
	free_filestruct(cutbuffer);
	cutbuffer = NULL;
#ifdef DEBUG
	fprintf(stderr, "Blew away cutbuffer =)\n");
#endif
	/* Indicate that future cuts should add to the cutbuffer. */
	keep_cutbuffer = TRUE;
    }

#ifndef NANO_TINY
    if (copy_text) {
	/* If the cutbuffer isn't empty, remember where it currently ends. */
	if (cutbuffer != NULL) {
	    cb_save = cutbottom;
	    cb_save_len = strlen(cutbottom->data);
	}
	/* Don't add a magicline when moving text to the cutbuffer. */
	SET(NO_NEWLINES);
    }
<<<<<<< HEAD
#endif

    /* Set keep_cutbuffer to TRUE, so that the text we're going to move
     * into the cutbuffer will be added to the text already in the
     * cutbuffer instead of replacing it. */
    keep_cutbuffer = TRUE;

#ifndef NANO_TINY

    if (cut_till_end) {
	/* If cut_till_end is TRUE, move all text up to the end of the
	 * file into the cutbuffer. */
	cut_to_eof();
    } else if (openfile->mark_set) {
	/* If the mark is on, move the marked text to the cutbuffer, and
	 * turn the mark off. */
	cut_marked();
=======

    if (cut_till_eof) {
	/* Move all text up to the end of the file into the cutbuffer. */
	cut_to_eof();
    } else if (openfile->mark_set) {
	/* Move the marked text to the cutbuffer, and turn the mark off. */
	cut_marked(&right_side_up);
>>>>>>> origin/tomato-shibby-RT-AC
	openfile->mark_set = FALSE;
    } else if (ISSET(CUT_TO_END))
	/* If the CUT_TO_END flag is set, move all text up to the end of
	 * the line into the cutbuffer. */
	cut_to_eol();
    else
#endif
	/* Move the entire line into the cutbuffer. */
	cut_line();

#ifndef NANO_TINY
    if (copy_text) {
	/* Copy the text that is in the cutbuffer (starting at its saved end,
	 * if there is one) back into the current buffer.  This effectively
	 * uncuts the text we just cut. */
	if (cutbuffer != NULL) {
	    if (cb_save != NULL) {
		cb_save->data += cb_save_len;
<<<<<<< HEAD
		copy_from_filestruct(cb_save, cutbottom);
		cb_save->data -= cb_save_len;
	    } else
		copy_from_filestruct(cutbuffer, cutbottom);
=======
		copy_from_buffer(cb_save);
		cb_save->data -= cb_save_len;
	    } else
		copy_from_buffer(cutbuffer);
>>>>>>> origin/tomato-shibby-RT-AC

	    /* If the copied region was marked forward, put the new desired
	     * x position at its end; otherwise, leave it at its beginning. */
	    if (right_side_up)
		openfile->placewewant = xplustabs();
	}
	/* Restore the magicline behavior now that we're done fiddling. */
	if (!old_no_newlines)
	    UNSET(NO_NEWLINES);
<<<<<<< HEAD
    } else if (!undoing)
	update_undo(CUT);
#endif
	/* Leave the text in the cutbuffer, and mark the file as
	 * modified. */
=======
    } else
#endif /* !NANO_TINY */
    /* Only set the modification flag if actually something was cut. */
    if (openfile->totsize != was_totsize)
>>>>>>> origin/tomato-shibby-RT-AC
	set_modified();

    /* Update the screen. */
    edit_refresh_needed = TRUE;

<<<<<<< HEAD
#ifdef ENABLE_COLOR
    reset_multis(openfile->current, FALSE);
=======
#ifndef DISABLE_COLOR
    check_the_multis(openfile->current);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

#ifdef DEBUG
    dump_filestruct(cutbuffer);
#endif
}

/* Move text from the current filestruct into the cutbuffer. */
void do_cut_text_void(void)
{
#ifndef NANO_TINY
    add_undo(CUT);
<<<<<<< HEAD
=======
#endif
    do_cut_text(FALSE, FALSE);
#ifndef NANO_TINY
    update_undo(CUT);
>>>>>>> origin/tomato-shibby-RT-AC
#endif
    do_cut_text(
#ifndef NANO_TINY
	FALSE, FALSE, FALSE
#endif
	);
}

#ifndef NANO_TINY
/* Move text from the current filestruct into the cutbuffer, and copy it
 * back into the filestruct afterward. */
void do_copy_text(void)
{
<<<<<<< HEAD
    do_cut_text(TRUE, FALSE, FALSE);
=======
    static struct filestruct *next_contiguous_line = NULL;
    bool mark_set = openfile->mark_set;

    /* Remember the current viewport and cursor position. */
    ssize_t is_edittop_lineno = openfile->edittop->lineno;
    size_t is_firstcolumn = openfile->firstcolumn;
    ssize_t is_current_lineno = openfile->current->lineno;
    size_t is_current_x = openfile->current_x;

    if (mark_set || openfile->current != next_contiguous_line)
	cutbuffer_reset();

    do_cut_text(TRUE, FALSE);

    /* If the mark was set, blow away the cutbuffer on the next copy. */
    next_contiguous_line = (mark_set ? NULL : openfile->current);

    if (mark_set) {
	/* Restore the viewport and cursor position. */
	openfile->edittop = fsfromline(is_edittop_lineno);
	openfile->firstcolumn = is_firstcolumn;
	openfile->current = fsfromline(is_current_lineno);
	openfile->current_x = is_current_x;
    }
>>>>>>> origin/tomato-shibby-RT-AC
}

/* Cut from the current cursor position to the end of the file. */
void do_cut_till_end(void)
{
#ifndef NANO_TINY
    add_undo(CUT);
#endif
    do_cut_text(FALSE, TRUE, FALSE);
}
#endif /* !NANO_TINY */

/* Copy text from the cutbuffer into the current filestruct. */
void do_uncut_text(void)
{
<<<<<<< HEAD
    assert(openfile->current != NULL && openfile->current->data != NULL);

    /* If the cutbuffer is empty, get out. */
=======
    ssize_t was_lineno = openfile->current->lineno;
	/* The line number where we started the paste. */
    size_t was_leftedge = 0;
	/* The leftedge where we started the paste. */

    /* If the cutbuffer is empty, there is nothing to do. */
>>>>>>> origin/tomato-shibby-RT-AC
    if (cutbuffer == NULL)
	return;

#ifndef NANO_TINY
<<<<<<< HEAD
     update_undo(UNCUT);
=======
    add_undo(PASTE);

    if (ISSET(SOFTWRAP))
	was_leftedge = (xplustabs() / editwincols) * editwincols;
>>>>>>> origin/tomato-shibby-RT-AC
#endif

    /* Add a copy of the text in the cutbuffer to the current filestruct
     * at the current cursor position. */
<<<<<<< HEAD
    copy_from_filestruct(cutbuffer, cutbottom);
=======
    copy_from_buffer(cutbuffer);

#ifndef NANO_TINY
    update_undo(PASTE);
#endif

    /* If we pasted less than a screenful, don't center the cursor. */
    if (less_than_a_screenful(was_lineno, was_leftedge))
	focusing = FALSE;
>>>>>>> origin/tomato-shibby-RT-AC

    /* Set the desired x position to where the pasted text ends. */
    openfile->placewewant = xplustabs();

    /* Mark the file as modified. */
    set_modified();

<<<<<<< HEAD
    /* Update the screen. */
    edit_refresh_needed = TRUE;

#ifdef ENABLE_COLOR
    reset_multis(openfile->current, FALSE);
=======
    /* Update the cursor position to account for the inserted lines. */
    reset_cursor();

    refresh_needed = TRUE;

#ifndef DISABLE_COLOR
    check_the_multis(openfile->current);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

#ifdef DEBUG
    dump_filestruct_reverse();
#endif
}
