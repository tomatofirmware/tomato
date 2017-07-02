/* $Id: utils.c 4453 2009-12-02 03:36:22Z astyanax $ */
/**************************************************************************
 *   utils.c  --  This file is part of GNU nano.                          *
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
#include <unistd.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#include <ctype.h>
#include <errno.h>

/* Return the number of decimal digits in n. */
int digits(size_t n)
{
    int i;

    if (n == 0)
	i = 1;
    else {
	for (i = 0; n != 0; n /= 10, i++)
	    ;
    }

    return i;
}

/* Return the user's home directory.  We use $HOME, and if that fails,
 * we fall back on the home directory of the effective user ID. */
void get_homedir(void)
{
    if (homedir == NULL) {
	const char *homenv = getenv("HOME");

<<<<<<< HEAD
	if (homenv == NULL) {
=======
#ifdef HAVE_PWD_H
	/* When HOME isn't set, or when we're root, get the home directory
	 * from the password file instead. */
	if (homenv == NULL || geteuid() == 0) {
>>>>>>> origin/tomato-shibby-RT-AC
	    const struct passwd *userage = getpwuid(geteuid());

	    if (userage != NULL)
		homenv = userage->pw_dir;
	}
<<<<<<< HEAD
	homedir = mallocstrcpy(NULL, homenv);
=======
#endif

	/* Only set homedir if some home directory could be determined,
	 * otherwise keep homedir NULL. */
	if (homenv != NULL && strcmp(homenv, "") != 0)
	    homedir = mallocstrcpy(NULL, homenv);
>>>>>>> origin/tomato-shibby-RT-AC
    }
}

#ifdef ENABLE_LINENUMBERS
/* Return the number of digits that the given integer n takes up. */
int digits(ssize_t n)
{
    if (n < 100000) {
        if (n < 1000) {
            if (n < 100)
                return 2;
            else
                return 3;
        } else {
            if (n < 10000)
                return 4;
            else
                return 5;
        }
    } else {
        if (n < 10000000) {
            if (n < 1000000)
                return 6;
            else
                return 7;
        }
        else {
            if (n < 100000000)
                return 8;
            else
                return 9;
        }
    }
}
#endif

/* Read an integer from str.  If it parses okay, store it in *result
 * and return TRUE; otherwise, return FALSE. */
bool parse_num(const char *str, ssize_t *result)
{
    char *first_error;
    ssize_t value;

<<<<<<< HEAD
    j = (ssize_t)strtol(str, &first_error, 10);
=======
    /* The manual page for strtol() says this is required. */
    errno = 0;

    value = (ssize_t)strtol(str, &first_error, 10);
>>>>>>> origin/tomato-shibby-RT-AC

    if (errno == ERANGE || *str == '\0' || *first_error != '\0')
	return FALSE;

    *result = value;

    return TRUE;
}

/* Read two numbers, separated by a comma, from str, and store them in
 * *line and *column.  Return FALSE on error, and TRUE otherwise. */
bool parse_line_column(const char *str, ssize_t *line, ssize_t *column)
{
    bool retval;
    char *firstpart;
    const char *comma;

    while (*str == ' ')
       str++;

    comma = strpbrk(str, "m,. /;");

    if (comma == NULL)
	return parse_num(str, line);

    retval = parse_num(comma + 1, column);

    if (comma == str)
	return retval;

    firstpart = mallocstrcpy(NULL, str);
    firstpart[comma - str] = '\0';

    retval = parse_num(firstpart, line) && retval;

    free(firstpart);

    return retval;
}

/* Reduce the memory allocation of a string to what is needed. */
void snuggly_fit(char **str)
{
    if (*str != NULL)
	*str = charealloc(*str, strlen(*str) + 1);
}

/* Null a string at a certain index and align it. */
void null_at(char **data, size_t index)
{
    *data = charealloc(*data, index + 1);
    (*data)[index] = '\0';
}

/* For non-null-terminated lines.  A line, by definition, shouldn't
 * normally have newlines in it, so encode its nulls as newlines. */
void unsunder(char *str, size_t true_len)
{
    for (; true_len > 0; true_len--, str++) {
	if (*str == '\0')
	    *str = '\n';
    }
}

/* For non-null-terminated lines.  A line, by definition, shouldn't
 * normally have newlines in it, so decode its newlines as nulls. */
void sunder(char *str)
{
    for (; *str != '\0'; str++) {
	if (*str == '\n')
	    *str = '\0';
    }
}

<<<<<<< HEAD
/* These functions, ngetline() (originally getline()) and ngetdelim()
 * (originally getdelim()), were adapted from GNU mailutils 0.5
 * (mailbox/getline.c).  Here is the notice from that file, after
 * converting to the GPL via LGPL clause 3, and with the Free Software
 * Foundation's address and the copyright years updated:
 *
 * GNU Mailutils -- a suite of utilities for electronic mail
 * Copyright (C) 1999, 2000, 2001, 2002, 2004, 2005, 2006, 2007
 * Free Software Foundation, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA. */

#ifdef ENABLE_NANORC

#ifndef HAVE_GETDELIM
/* This function is equivalent to getdelim(). */
ssize_t ngetdelim(char **lineptr, size_t *n, int delim, FILE *stream)
{
    size_t indx = 0;
    int c;

    /* Sanity checks. */
    if (lineptr == NULL || n == NULL || stream == NULL ||
	fileno(stream) == -1) {
	errno = EINVAL;
	return -1;
    }

    /* Allocate the line the first time. */
    if (*lineptr == NULL) {
	*n = MAX_BUF_SIZE;
	*lineptr = charalloc(*n);
    }

    while ((c = getc(stream)) != EOF) {
	/* Check if more memory is needed. */
	if (indx >= *n) {
	    *n += MAX_BUF_SIZE;
	    *lineptr = charealloc(*lineptr, *n);
	}

	/* Put the result in the line. */
	(*lineptr)[indx++] = (char)c;

	/* Bail out. */
	if (c == delim)
	    break;
    }

    /* Make room for the null character. */
    if (indx >= *n) {
	*n += MAX_BUF_SIZE;
	*lineptr = charealloc(*lineptr, *n);
    }

    /* Null-terminate the buffer. */
    null_at(lineptr, indx++);
    *n = indx;

    /* The last line may not have the delimiter.  We have to return what
     * we got, and the error will be seen on the next iteration. */
    return (c == EOF && (indx - 1) == 0) ? -1 : indx - 1;
}
#endif

#ifndef HAVE_GETLINE
/* This function is equivalent to getline(). */
ssize_t ngetline(char **lineptr, size_t *n, FILE *stream)
{
    return getdelim(lineptr, n, '\n', stream);
}
#endif
#endif /* ENABLE_NANORC */

#ifdef HAVE_REGEX_H
/* Do the compiled regex in preg and the regex in string match the
 * beginning or end of a line? */
bool regexp_bol_or_eol(const regex_t *preg, const char *string)
=======
/* Fix the regex if we're on platforms which require an adjustment
 * from GNU-style to BSD-style word boundaries. */
const char *fixbounds(const char *r)
>>>>>>> origin/tomato-shibby-RT-AC
{
    return (regexec(preg, string, 0, NULL, 0) == 0 &&
	regexec(preg, string, 0, NULL, REG_NOTBOL | REG_NOTEOL) ==
	REG_NOMATCH);
}

/* Fix the regex if we're on platforms which requires an adjustment
 * from GNU-style to BSD-style word boundaries.  */
const char *fixbounds(const char *r) {
#ifndef GNU_WORDBOUNDS
    int i, j = 0;
    char *r2 = charalloc(strlen(r) * 5);
    char *r3;

#ifdef DEBUG
    fprintf(stderr, "fixbounds(): Start string = \"%s\"\n", r);
#endif

    for (i = 0; i < strlen(r); i++) {
        if (r[i] != '\0' && r[i] == '\\' && (r[i+1] == '>' || r[i+1] == '<')) {
            strcpy(&r2[j], "[[:");
            r2[j+3] = r[i+1];
            strcpy(&r2[j+4], ":]]");
            i++;
            j += 6;
        } else
            r2[j] = r[i];
        j++;
    }
    r2[j] = '\0';
    r3 = mallocstrcpy(NULL, r2);
    free(r2);
#ifdef DEBUG
    fprintf(stderr, "fixbounds(): Ending string = \"%s\"\n", r3);
#endif
    return (const char *) r3;
#endif

    return r;
}
<<<<<<< HEAD

#endif
=======
>>>>>>> origin/tomato-shibby-RT-AC

#ifndef DISABLE_SPELLER
/* Is the word starting at position pos in buf a whole word? */
bool is_whole_word(size_t pos, const char *buf, const char *word)
{
<<<<<<< HEAD
    char *p = charalloc(mb_cur_max()), *r = charalloc(mb_cur_max());
    size_t word_end = pos + strlen(word);
    bool retval;

    assert(buf != NULL && pos <= strlen(buf) && word != NULL);
=======
    char before[mb_cur_max()], after[mb_cur_max()];
    size_t word_end = position + length;
>>>>>>> origin/tomato-shibby-RT-AC

    parse_mbchar(buf + move_mbleft(buf, pos), p, NULL);
    parse_mbchar(buf + word_end, r, NULL);

<<<<<<< HEAD
    /* If we're at the beginning of the line or the character before the
     * word isn't a non-punctuation "word" character, and if we're at
     * the end of the line or the character after the word isn't a
     * non-punctuation "word" character, we have a whole word. */
    retval = (pos == 0 || !is_word_mbchar(p, FALSE)) &&
	(word_end == strlen(buf) || !is_word_mbchar(r, FALSE));

    free(p);
    free(r);

    return retval;
=======
    /* If the word starts at the beginning of the line OR the character before
     * the word isn't a letter, and if the word ends at the end of the line OR
     * the character after the word isn't a letter, we have a whole word. */
    return ((position == 0 || !is_alpha_mbchar(before)) &&
		(buf[word_end] == '\0' || !is_alpha_mbchar(after)));
>>>>>>> origin/tomato-shibby-RT-AC
}
#endif /* !DISABLE_SPELLER */

/* Return the position of the needle in the haystack, or NULL if not found.
 * When searching backwards, we will find the last match that starts no later
 * than the given start; otherwise, we find the first match starting no earlier
 * than start.  If we are doing a regexp search, and we find a match, we fill
 * in the global variable regmatches with at most 9 subexpression matches. */
const char *strstrwrapper(const char *haystack, const char *needle,
	const char *start)
{
    if (ISSET(USE_REGEXP)) {
	if (ISSET(BACKWARDS_SEARCH)) {
	    size_t last_find, ceiling, far_end;
	    size_t floor = 0, next_rung = 0;
		/* The start of the search range, and the next start. */

	    if (regexec(&search_regexp, haystack, 1, regmatches, 0) != 0)
		return NULL;

	    far_end = strlen(haystack);
	    ceiling = start - haystack;
	    last_find = regmatches[0].rm_so;

	    /* A result beyond the search range also means: no match. */
	    if (last_find > ceiling)
		return NULL;

	    /* Move the start-of-range forward until there is no more match;
	     * then the last match found is the first match backwards. */
	    while (regmatches[0].rm_so <= ceiling) {
		floor = next_rung;
		last_find = regmatches[0].rm_so;
		/* If this is the last possible match, don't try to advance. */
		if (last_find == ceiling)
		    break;
		next_rung = move_mbright(haystack, last_find);
		regmatches[0].rm_so = next_rung;
		regmatches[0].rm_eo = far_end;
		if (regexec(&search_regexp, haystack, 1, regmatches,
					REG_STARTEND) != 0)
		    break;
	    }

	    /* Find the last match again, to get possible submatches. */
	    regmatches[0].rm_so = floor;
	    regmatches[0].rm_eo = far_end;
	    if (regexec(&search_regexp, haystack, 10, regmatches,
					REG_STARTEND) != 0)
		statusline(ALERT, "BAD: failed to refind the match!");

	    return haystack + regmatches[0].rm_so;
	}

	/* Do a forward regex search from the starting point. */
	regmatches[0].rm_so = start - haystack;
	regmatches[0].rm_eo = strlen(haystack);
	if (regexec(&search_regexp, haystack, 10, regmatches,
					REG_STARTEND) != 0)
	    return NULL;
	else
	    return haystack + regmatches[0].rm_so;
    }
    if (ISSET(CASE_SENSITIVE)) {
	if (ISSET(BACKWARDS_SEARCH))
	    return revstrstr(haystack, needle, start);
	else
	    return strstr(start, needle);
    } else if (ISSET(BACKWARDS_SEARCH))
	return mbrevstrcasestr(haystack, needle, start);

    return mbstrcasestr(start, needle);
}

/* This is a wrapper for the perror() function.  The wrapper temporarily
 * leaves curses mode, calls perror() (which writes to stderr), and then
 * reenters curses mode, updating the screen in the process.  Note that
 * nperror() causes the window to flicker once. */
void nperror(const char *s)
{
    endwin();
    perror(s);
    doupdate();
}

/* This is a wrapper for the malloc() function that properly handles
 * things when we run out of memory.  Thanks, BG, many people have been
 * asking for this... */
void *nmalloc(size_t howmuch)
{
    void *r = malloc(howmuch);

    if (r == NULL && howmuch != 0)
	die(_("nano is out of memory!"));

    return r;
}

/* This is a wrapper for the realloc() function that properly handles
 * things when we run out of memory. */
void *nrealloc(void *ptr, size_t howmuch)
{
    void *r = realloc(ptr, howmuch);

    if (r == NULL && howmuch != 0)
	die(_("nano is out of memory!"));

    return r;
}

<<<<<<< HEAD
/* Copy the first n characters of one malloc()ed string to another
 * pointer.  Should be used as: "dest = mallocstrncpy(dest, src,
 * n);". */
=======
/* Allocate and copy the first n characters of the given src string, after
 * freeing the destination.  Usage: "dest = mallocstrncpy(dest, src, n);". */
>>>>>>> origin/tomato-shibby-RT-AC
char *mallocstrncpy(char *dest, const char *src, size_t n)
{
    if (src == NULL)
	src = "";

    if (src != dest)
	free(dest);

    dest = charalloc(n);
    strncpy(dest, src, n);

    return dest;
}

/* Free the dest string and return a malloc'ed copy of src.  Should be used as:
 * "dest = mallocstrcpy(dest, src);". */
char *mallocstrcpy(char *dest, const char *src)
{
    return mallocstrncpy(dest, src, (src == NULL) ? 1 :
	strlen(src) + 1);
}

/* Free the string at dest and return the string at src. */
char *free_and_assign(char *dest, char *src)
{
    free(dest);
    return src;
}

/* When not in softwrap mode, nano scrolls horizontally within a line in
 * chunks (a bit smaller than the chunks used in softwrapping).  Return the
 * column number of the first character displayed in the edit window when the
 * cursor is at the given column.  Note that (0 <= column -
 * get_page_start(column) < COLS). */
size_t get_page_start(size_t column)
{
    if (column < editwincols - 1 || ISSET(SOFTWRAP) || column == 0)
	return 0;
    else if (editwincols > 8)
	return column - 7 - (column - 7) % (editwincols - 8);
    else
	return column - (editwincols - 2);
}

/* Return the placewewant associated with current_x, i.e. the zero-based
 * column position of the cursor.  The value will be no smaller than
 * current_x. */
size_t xplustabs(void)
{
    return strnlenpt(openfile->current->data, openfile->current_x);
}

/* Return the index in s of the character displayed at the given column,
 * i.e. the largest value such that strnlenpt(s, actual_x(s, column)) <=
 * column. */
size_t actual_x(const char *s, size_t column)
{
    size_t i = 0;
	/* The position in s, returned. */
    size_t len = 0;
	/* The screen display width to s[i]. */

<<<<<<< HEAD
    assert(s != NULL);

    while (*s != '\0') {
	int s_len = parse_mbchar(s, NULL, &len);
=======
    while (*text != '\0') {
	int charlen = parse_mbchar(text, NULL, &width);
>>>>>>> origin/tomato-shibby-RT-AC

	if (len > column)
	    break;

	i += s_len;
	s += s_len;
    }

    return i;
}

/* A strnlen() with tabs and multicolumn characters factored in, similar
 * to xplustabs().  How many columns wide are the first maxlen characters
 * of s? */
size_t strnlenpt(const char *s, size_t maxlen)
{
    size_t len = 0;
	/* The screen display width to s[i]. */

    if (maxlen == 0)
	return 0;

<<<<<<< HEAD
    assert(s != NULL);

    while (*s != '\0') {
	int s_len = parse_mbchar(s, NULL, &len);

	s += s_len;
=======
    while (*text != '\0') {
	int charlen = parse_mbchar(text, NULL, &width);
>>>>>>> origin/tomato-shibby-RT-AC

	if (maxlen <= s_len)
	    break;

	maxlen -= s_len;
    }

    return len;
}

<<<<<<< HEAD
/* A strlen() with tabs and multicolumn characters factored in, similar
 * to xplustabs().  How many columns wide is s? */
size_t strlenpt(const char *s)
{
    return strnlenpt(s, (size_t)-1);
=======
/* Return the number of columns that the given text occupies. */
size_t strlenpt(const char *text)
{
    size_t span = 0;

    while (*text != '\0')
	text += parse_mbchar(text, NULL, &span);

    return span;
>>>>>>> origin/tomato-shibby-RT-AC
}

/* Append a new magicline to filebot. */
void new_magicline(void)
{
    openfile->filebot->next = make_new_node(openfile->filebot);
    openfile->filebot->next->data = mallocstrcpy(NULL, "");
<<<<<<< HEAD
    openfile->filebot->next->prev = openfile->filebot;
    openfile->filebot->next->next = NULL;
    openfile->filebot->next->lineno = openfile->filebot->lineno + 1;
#ifdef ENABLE_COLOR
    openfile->filebot->next->multidata = NULL;
#endif
=======
>>>>>>> origin/tomato-shibby-RT-AC
    openfile->filebot = openfile->filebot->next;
    openfile->totsize++;
}

#ifndef NANO_TINY
/* Remove the magicline from filebot, if there is one and it isn't the
 * only line in the file.  Assume that edittop and current are not at
 * filebot. */
void remove_magicline(void)
{
    if (openfile->filebot->data[0] == '\0' &&
<<<<<<< HEAD
	openfile->filebot != openfile->fileage) {
	assert(openfile->filebot != openfile->edittop && openfile->filebot != openfile->current);

=======
		openfile->filebot != openfile->fileage) {
>>>>>>> origin/tomato-shibby-RT-AC
	openfile->filebot = openfile->filebot->prev;
	free_filestruct(openfile->filebot->next);
	openfile->filebot->next = NULL;
	openfile->totsize--;
    }
}

/* Set top_x and bot_x to the top and bottom x-coordinates of the mark,
 * respectively, based on the locations of top and bot.  If
 * right_side_up isn't NULL, set it to TRUE if the mark begins with
 * (mark_begin, mark_begin_x) and ends with (current, current_x), or
 * FALSE otherwise. */
void mark_order(const filestruct **top, size_t *top_x, const filestruct
	**bot, size_t *bot_x, bool *right_side_up)
{
    if ((openfile->current->lineno == openfile->mark_begin->lineno &&
	openfile->current_x > openfile->mark_begin_x) ||
	openfile->current->lineno > openfile->mark_begin->lineno) {
	*top = openfile->mark_begin;
	*top_x = openfile->mark_begin_x;
	*bot = openfile->current;
	*bot_x = openfile->current_x;
	if (right_side_up != NULL)
	    *right_side_up = TRUE;
    } else {
	*bot = openfile->mark_begin;
	*bot_x = openfile->mark_begin_x;
	*top = openfile->current;
	*top_x = openfile->current_x;
	if (right_side_up != NULL)
	    *right_side_up = FALSE;
    }
}
<<<<<<< HEAD
#endif

/* Calculate the number of characters between begin and end, and return
 * it. */
size_t get_totsize(const filestruct *begin, const filestruct *end)
{
    size_t totsize = 0;
    const filestruct *f;

    /* Go through the lines from begin to end->prev, if we can. */
    for (f = begin; f != end && f != NULL; f = f->next) {
	/* Count the number of characters on this line. */
	totsize += mbstrlen(f->data);

	/* Count the newline if we have one. */
	if (f->next != NULL)
	    totsize++;
    }

    /* Go through the line at end, if we can. */
    if (f != NULL) {
	/* Count the number of characters on this line. */
	totsize += mbstrlen(f->data);

	/* Count the newline if we have one. */
	if (f->next != NULL)
	    totsize++;
    }

    return totsize;
}
=======
>>>>>>> origin/tomato-shibby-RT-AC

/* Get back a pointer given a line number in the current openfilestruct */
filestruct *fsfromline(ssize_t lineno)
{
    filestruct *f = openfile->current;

    if (lineno <= openfile->current->lineno)
	for (; f->lineno != lineno && f != openfile->fileage; f = f->prev)
	   ;
    else
<<<<<<< HEAD
        for (; f->lineno != lineno && f->next != NULL; f = f->next)
	    ;
=======
	while (f->lineno != lineno && f->next != NULL)
	    f = f->next;

    if (f->lineno != lineno) {
	statusline(ALERT, _("Internal error: can't match line %ld.  "
			"Please save your work."), (long)lineno);
	return NULL;
    }
>>>>>>> origin/tomato-shibby-RT-AC

    if (f->lineno != lineno)
	f = NULL;
    return f;
}
#endif /* !NANO_TINY */

/* Count the number of characters from begin to end, and return it. */
size_t get_totsize(const filestruct *begin, const filestruct *end)
{
    const filestruct *line;
    size_t totsize = 0;

    /* Sum the number of characters (plus a newline) in each line. */
    for (line = begin; line != end->next; line = line->next)
	totsize += mbstrlen(line->data) + 1;

    /* The last line of a file doesn't have a newline -- otherwise it
     * wouldn't be the last line -- so subtract 1 when at EOF. */
    if (line == NULL)
	totsize--;

    return totsize;
}

#ifdef DEBUG
/* Dump the filestruct inptr to stderr. */
void dump_filestruct(const filestruct *inptr)
{
    if (inptr == openfile->fileage)
	fprintf(stderr, "Dumping file buffer to stderr...\n");
    else if (inptr == cutbuffer)
	fprintf(stderr, "Dumping cutbuffer to stderr...\n");
    else
	fprintf(stderr, "Dumping a buffer to stderr...\n");

    while (inptr != NULL) {
	fprintf(stderr, "(%ld) %s\n", (long)inptr->lineno, inptr->data);
	inptr = inptr->next;
    }
}

/* Dump the current buffer's filestruct to stderr in reverse. */
void dump_filestruct_reverse(void)
{
    const filestruct *fileptr = openfile->filebot;

    while (fileptr != NULL) {
	fprintf(stderr, "(%ld) %s\n", (long)fileptr->lineno,
		fileptr->data);
	fileptr = fileptr->prev;
    }
}
#endif /* DEBUG */
