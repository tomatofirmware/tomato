/* $Id: color.c 4453 2009-12-02 03:36:22Z astyanax $ */
/**************************************************************************
 *   color.c  --  This file is part of GNU nano.                          *
 *                                                                        *
<<<<<<< HEAD
 *   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009   *
 *   Free Software Foundation, Inc.                                       *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 3, or (at your option)  *
 *   any later version.                                                   *
=======
 *   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,  *
 *   2010, 2011, 2013, 2014, 2015 Free Software Foundation, Inc.          *
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

#include <stdio.h>
#include <string.h>

#ifdef ENABLE_COLOR

/* For each syntax list entry, go through the list of colors and assign
 * the color pairs. */
void set_colorpairs(void)
{
<<<<<<< HEAD
    const syntaxtype *this_syntax = syntaxes;

    for (; this_syntax != NULL; this_syntax = this_syntax->next) {
	colortype *this_color = this_syntax->color;
	int color_pair = 1;
=======
    const syntaxtype *sint;
    bool using_defaults = FALSE;
    short foreground, background;
    size_t i;

    /* Tell ncurses to enable colors. */
    start_color();

#ifdef HAVE_USE_DEFAULT_COLORS
    /* Allow using the default colors, if available. */
    using_defaults = (use_default_colors() != ERR);
#endif

    /* Initialize the color pairs for nano's interface elements. */
    for (i = 0; i < NUMBER_OF_ELEMENTS; i++) {
	bool bright = FALSE;

	if (parse_color_names(specified_color_combo[i],
			&foreground, &background, &bright)) {
	    if (foreground == -1 && !using_defaults)
		foreground = COLOR_WHITE;
	    if (background == -1 && !using_defaults)
		background = COLOR_BLACK;
	    init_pair(i + 1, foreground, background);
	    interface_color_pair[i] =
			COLOR_PAIR(i + 1) | (bright ? A_BOLD : A_NORMAL);
	}
	else {
	    if (i != FUNCTION_TAG)
		interface_color_pair[i] = hilite_attribute;
	    else
		interface_color_pair[i] = A_NORMAL;
	}

	free(specified_color_combo[i]);
	specified_color_combo[i] = NULL;
    }

    /* For each syntax, go through its list of colors and assign each
     * its pair number, giving identical color pairs the same number. */
    for (sint = syntaxes; sint != NULL; sint = sint->next) {
	colortype *ink;
	int new_number = NUMBER_OF_ELEMENTS + 1;
>>>>>>> origin/tomato-shibby-RT-AC

	for (ink = sint->color; ink != NULL; ink = ink->next) {
	    const colortype *beforenow = sint->color;

<<<<<<< HEAD
	    for (; beforenow != this_color &&
		(beforenow->fg != this_color->fg ||
		beforenow->bg != this_color->bg ||
		beforenow->bright != this_color->bright);
		beforenow = beforenow->next)
		;

	    if (beforenow != this_color)
		this_color->pairnum = beforenow->pairnum;
	    else {
		this_color->pairnum = color_pair;
		color_pair++;
	    }
=======
	    while (beforenow != ink && (beforenow->fg != ink->fg ||
					beforenow->bg != ink->bg ||
					beforenow->bright != ink->bright))
		beforenow = beforenow->next;

	    if (beforenow != ink)
		ink->pairnum = beforenow->pairnum;
	    else
		ink->pairnum = new_number++;

	    ink->attributes = COLOR_PAIR(ink->pairnum) |
				(ink->bright ? A_BOLD : A_NORMAL);
>>>>>>> origin/tomato-shibby-RT-AC
	}
    }
}

/* Initialize the color information. */
void color_init(void)
{
<<<<<<< HEAD
    assert(openfile != NULL);

    if (has_colors()) {
	const colortype *tmpcolor;
=======
    const colortype *ink;
    bool using_defaults = FALSE;
    short foreground, background;

    /* If the terminal is not capable of colors, forget it. */
    if (!has_colors())
	return;

>>>>>>> origin/tomato-shibby-RT-AC
#ifdef HAVE_USE_DEFAULT_COLORS
	bool defok;
#endif

<<<<<<< HEAD
	start_color();
=======
    /* For each coloring expression, initialize the color pair. */
    for (ink = openfile->colorstrings; ink != NULL; ink = ink->next) {
	foreground = ink->fg;
	background = ink->bg;
>>>>>>> origin/tomato-shibby-RT-AC

#ifdef HAVE_USE_DEFAULT_COLORS
	/* Use the default colors, if available. */
	defok = (use_default_colors() != ERR);
#endif

<<<<<<< HEAD
	for (tmpcolor = openfile->colorstrings; tmpcolor != NULL;
		tmpcolor = tmpcolor->next) {
	    short foreground = tmpcolor->fg, background = tmpcolor->bg;
	    if (foreground == -1) {
#ifdef HAVE_USE_DEFAULT_COLORS
		if (!defok)
=======
	init_pair(ink->pairnum, foreground, background);
#ifdef DEBUG
	fprintf(stderr, "init_pair(): fg = %hd, bg = %hd\n", foreground, background);
>>>>>>> origin/tomato-shibby-RT-AC
#endif
		    foreground = COLOR_WHITE;
	    }

	    if (background == -1) {
#ifdef HAVE_USE_DEFAULT_COLORS
		if (!defok)
#endif
		    background = COLOR_BLACK;
	    }

	    init_pair(tmpcolor->pairnum, foreground, background);

#ifdef DEBUG
	    fprintf(stderr, "init_pair(): fg = %hd, bg = %hd\n", tmpcolor->fg, tmpcolor->bg);
#endif
	}
    }
}

/* Update the color information based on the current filename. */
void color_update(void)
{
    syntaxtype *tmpsyntax;
    syntaxtype *defsyntax = NULL;
    colortype *tmpcolor, *defcolor = NULL;

<<<<<<< HEAD
    assert(openfile != NULL);

    openfile->syntax = NULL;
    openfile->colorstrings = NULL;
=======
    /* If the rcfiles were not read, or contained no syntaxes, get out. */
    if (syntaxes == NULL)
	return;
>>>>>>> origin/tomato-shibby-RT-AC

    /* If we specified a syntax override string, use it. */
    if (syntaxstr != NULL) {
	/* If the syntax override is "none", it's the same as not having
	 * a syntax at all, so get out. */
	if (strcmp(syntaxstr, "none") == 0)
	    return;

	for (tmpsyntax = syntaxes; tmpsyntax != NULL;
		tmpsyntax = tmpsyntax->next) {
	    if (strcmp(tmpsyntax->desc, syntaxstr) == 0) {
		openfile->syntax = tmpsyntax;
		openfile->colorstrings = tmpsyntax->color;
	    }

	    if (openfile->colorstrings != NULL)
		break;
	}
    }

    /* If we didn't specify a syntax override string, or if we did and
     * there was no syntax by that name, get the syntax based on the
     * file extension, and then look in the header. */
    if (openfile->colorstrings == NULL) {
	for (tmpsyntax = syntaxes; tmpsyntax != NULL;
		tmpsyntax = tmpsyntax->next) {
	    exttype *e;

	    /* If this is the default syntax, it has no associated
	     * extensions, which we've checked for elsewhere.  Skip over
	     * it here, but keep track of its color regexes. */
	    if (strcmp(tmpsyntax->desc, "default") == 0) {
		defsyntax = tmpsyntax;
		defcolor = tmpsyntax->color;
		continue;
	    }

	    for (e = tmpsyntax->extensions; e != NULL; e = e->next) {
		bool not_compiled = (e->ext == NULL);

		/* e->ext_regex has already been checked for validity
		 * elsewhere.  Compile its specified regex if we haven't
		 * already. */
		if (not_compiled) {
		    e->ext = (regex_t *)nmalloc(sizeof(regex_t));
		    regcomp(e->ext, fixbounds(e->ext_regex), REG_EXTENDED);
		}

		/* Set colorstrings if we matched the extension
		 * regex. */
		if (regexec(e->ext, openfile->filename, 0, NULL,
			0) == 0) {
		    openfile->syntax = tmpsyntax;
		    openfile->colorstrings = tmpsyntax->color;
		}

		if (openfile->colorstrings != NULL)
		    break;

		/* Decompile e->ext_regex's specified regex if we aren't
		 * going to use it. */
		if (not_compiled) {
		    regfree(e->ext);
		    free(e->ext);
		    e->ext = NULL;
		}
	    }
	}

	/* If we haven't matched anything yet, try the headers */
	if (openfile->colorstrings == NULL) {
#ifdef DEBUG
	    fprintf(stderr, "No match for file extensions, looking at headers...\n");
#endif
	    for (tmpsyntax = syntaxes; tmpsyntax != NULL;
		tmpsyntax = tmpsyntax->next) {
		exttype *e;

		for (e = tmpsyntax->headers; e != NULL; e = e->next) {
		    bool not_compiled = (e->ext == NULL);

		    /* e->ext_regex has already been checked for validity
		     * elsewhere.  Compile its specified regex if we haven't
		     * already. */
		    if (not_compiled) {
			e->ext = (regex_t *)nmalloc(sizeof(regex_t));
			regcomp(e->ext, fixbounds(e->ext_regex), REG_EXTENDED);
		    }

		    /* Set colorstrings if we matched the extension
		     * regex. */
#ifdef DEBUG
		fprintf(stderr, "Comparing header regex \"%s\" to fileage \"%s\"...\n", e->ext_regex, openfile->fileage->data);
#endif
		    if (regexec(e->ext, openfile->fileage->data, 0, NULL, 0) == 0) {
			openfile->syntax = tmpsyntax;
			openfile->colorstrings = tmpsyntax->color;
		    }

		    if (openfile->colorstrings != NULL)
			break;

		    /* Decompile e->ext_regex's specified regex if we aren't
		     * going to use it. */
		    if (not_compiled) {
			regfree(e->ext);
			free(e->ext);
			e->ext = NULL;
		    }
		}
	    }
	}
    }


    /* If we didn't get a syntax based on the file extension, and we
     * have a default syntax, use it. */
    if (openfile->colorstrings == NULL && defcolor != NULL) {
	openfile->syntax = defsyntax;
	openfile->colorstrings = defcolor;
    }

    for (tmpcolor = openfile->colorstrings; tmpcolor != NULL;
	tmpcolor = tmpcolor->next) {
	/* tmpcolor->start_regex and tmpcolor->end_regex have already
	 * been checked for validity elsewhere.  Compile their specified
	 * regexes if we haven't already. */
	if (tmpcolor->start == NULL) {
	    tmpcolor->start = (regex_t *)nmalloc(sizeof(regex_t));
	    regcomp(tmpcolor->start, fixbounds(tmpcolor->start_regex),
		REG_EXTENDED | (tmpcolor->icase ? REG_ICASE : 0));
	}

	if (tmpcolor->end_regex != NULL && tmpcolor->end == NULL) {
	    tmpcolor->end = (regex_t *)nmalloc(sizeof(regex_t));
	    regcomp(tmpcolor->end, fixbounds(tmpcolor->end_regex),
		REG_EXTENDED | (tmpcolor->icase ? REG_ICASE : 0));
	}
    }
}

<<<<<<< HEAD
/* Reset the multicolor info cache for records for any lines which need
   to be recalculated */
void reset_multis_after(filestruct *fileptr, int mindex)
{
    filestruct *oof;
    for (oof = fileptr->next; oof != NULL; oof = oof->next) {
	alloc_multidata_if_needed(oof);
	if (oof->multidata == NULL)
	    continue;
	if (oof->multidata[mindex] != CNONE)
	    oof->multidata[mindex] = -1;
	else
	    break;
    }
    for (; oof != NULL; oof = oof->next) {
	alloc_multidata_if_needed(oof);
	if (oof->multidata == NULL)
	    continue;
	if (oof->multidata[mindex] == CNONE)
	    oof->multidata[mindex] = -1;
	else
	    break;
    }
    edit_refresh_needed = TRUE;
}

void reset_multis_before(filestruct *fileptr, int mindex)
{
    filestruct *oof;
    for (oof = fileptr->prev; oof != NULL; oof = oof->prev) {
	alloc_multidata_if_needed(oof);
	if (oof->multidata == NULL)
	    continue;
	if (oof->multidata[mindex] != CNONE)
	    oof->multidata[mindex] = -1;
	else
	    break;
    }
    for (; oof != NULL; oof = oof->prev) {
	alloc_multidata_if_needed(oof);
	if (oof->multidata == NULL)
	    continue;
	if (oof->multidata[mindex] == CNONE)
	    oof->multidata[mindex] = -1;
	else
	    break;
    }

    edit_refresh_needed = TRUE;
}

/* Reset one multiline regex info */
void reset_multis_for_id(filestruct *fileptr, int num)
{
    reset_multis_before(fileptr, num);
    reset_multis_after(fileptr, num);
    fileptr->multidata[num] = -1;
}

/* Reset multi line strings around a filestruct ptr, trying to be smart about stopping
   force = reset everything regardless, useful when we don't know how much screen state
           has changed  */
void reset_multis(filestruct *fileptr, bool force)
{
    int nobegin, noend;
=======
/* Determine whether the matches of multiline regexes are still the same,
 * and if not, schedule a screen refresh, so things will be repainted. */
void check_the_multis(filestruct *line)
{
    const colortype *ink;
    bool astart, anend;
>>>>>>> origin/tomato-shibby-RT-AC
    regmatch_t startmatch, endmatch;

    if (!openfile->syntax)
	return;

<<<<<<< HEAD
    for (; tmpcolor != NULL; tmpcolor = tmpcolor->next) {

	/* If it's not a multi-line regex, amscray */
	if (tmpcolor->end == NULL)
	    continue;

	alloc_multidata_if_needed(fileptr);
	if (force == TRUE) {
	    reset_multis_for_id(fileptr, tmpcolor->id);
	    continue;
	}

	/* Figure out where the first begin and end are to determine if
	   things changed drastically for the precalculated multi values */
        nobegin = regexec(tmpcolor->start, fileptr->data, 1, &startmatch, 0);
        noend = regexec(tmpcolor->end, fileptr->data, 1, &endmatch, 0);
	if (fileptr->multidata[tmpcolor->id] ==  CWHOLELINE) {
	    if (nobegin && noend)
		continue;
	} else if (fileptr->multidata[tmpcolor->id] == CNONE) {
	    if (nobegin && noend)
=======
    for (ink = openfile->colorstrings; ink != NULL; ink = ink->next) {
	/* If it's not a multiline regex, skip. */
	if (ink->end == NULL)
	    continue;

	alloc_multidata_if_needed(line);

	astart = (regexec(ink->start, line->data, 1, &startmatch, 0) == 0);
	anend = (regexec(ink->end, line->data, 1, &endmatch, 0) == 0);

	/* Check whether the multidata still matches the current situation. */
	if (line->multidata[ink->id] == CNONE ||
			line->multidata[ink->id] == CWHOLELINE) {
	    if (!astart && !anend)
		continue;
	} else if (line->multidata[ink->id] == CSTARTENDHERE) {
	    if (astart && anend && startmatch.rm_so < endmatch.rm_so)
		continue;
	} else if (line->multidata[ink->id] == CBEGINBEFORE) {
	    if (!astart && anend)
		continue;
	} else if (line->multidata[ink->id] == CENDAFTER) {
	    if (astart && !anend)
>>>>>>> origin/tomato-shibby-RT-AC
		continue;
	}  else if (fileptr->multidata[tmpcolor->id] & CBEGINBEFORE && !noend
	  && (nobegin || endmatch.rm_eo > startmatch.rm_eo)) {
	    reset_multis_after(fileptr, tmpcolor->id);
	    continue;
	}

<<<<<<< HEAD
	/* If we got here assume the worst */
	reset_multis_for_id(fileptr, tmpcolor->id);
    }
}
#endif /* ENABLE_COLOR */
=======
	/* There is a mismatch, so something changed: repaint. */
	refresh_needed = TRUE;
	return;
    }
}

/* Allocate (for one line) the cache space for multiline color regexes. */
void alloc_multidata_if_needed(filestruct *fileptr)
{
    int i;

    if (fileptr->multidata == NULL) {
	fileptr->multidata = (short *)nmalloc(openfile->syntax->nmultis * sizeof(short));

	for (i = 0; i < openfile->syntax->nmultis; i++)
	    fileptr->multidata[i] = -1;
    }
}

/* Precalculate the multi-line start and end regex info so we can
 * speed up rendering (with any hope at all...). */
void precalc_multicolorinfo(void)
{
    const colortype *ink;
    regmatch_t startmatch, endmatch;
    filestruct *line, *tailline;

    if (openfile->colorstrings == NULL || ISSET(NO_COLOR_SYNTAX))
	return;

#ifdef DEBUG
    fprintf(stderr, "Precalculating the multiline color info...\n");
#endif

    for (ink = openfile->colorstrings; ink != NULL; ink = ink->next) {
	/* If this is not a multi-line regex, skip it. */
	if (ink->end == NULL)
	    continue;

	for (line = openfile->fileage; line != NULL; line = line->next) {
	    int index = 0;

	    alloc_multidata_if_needed(line);
	    /* Assume nothing applies until proven otherwise below. */
	    line->multidata[ink->id] = CNONE;

	    /* For an unpaired start match, mark all remaining lines. */
	    if (line->prev && line->prev->multidata[ink->id] == CWOULDBE) {
		line->multidata[ink->id] = CWOULDBE;
		continue;
	    }

	    /* When the line contains a start match, look for an end, and if
	     * found, mark all the lines that are affected. */
	    while (regexec(ink->start, line->data + index, 1,
			&startmatch, (index == 0) ? 0 : REG_NOTBOL) == 0) {
		/* Begin looking for an end match after the start match. */
		index += startmatch.rm_eo;

		/* If there is an end match on this line, mark the line, but
		 * continue looking for other starts after it. */
		if (regexec(ink->end, line->data + index, 1,
			&endmatch, (index == 0) ? 0 : REG_NOTBOL) == 0) {
		    line->multidata[ink->id] = CSTARTENDHERE;
		    index += endmatch.rm_eo;
		    /* If both start and end are mere anchors, step ahead. */
		    if (startmatch.rm_so == startmatch.rm_eo &&
				endmatch.rm_so == endmatch.rm_eo) {
			/* When at end-of-line, we're done. */
			if (line->data[index] == '\0')
			    break;
			index = move_mbright(line->data, index);
		    }
		    continue;
		}

		/* Look for an end match on later lines. */
		tailline = line->next;

		while (tailline != NULL) {
		    if (regexec(ink->end, tailline->data, 1, &endmatch, 0) == 0)
			break;
		    tailline = tailline->next;
		}

		if (tailline == NULL) {
		    line->multidata[ink->id] = CWOULDBE;
		    break;
		}

		/* We found it, we found it, la la la la la.  Mark all
		 * the lines in between and the end properly. */
		line->multidata[ink->id] = CENDAFTER;

		for (line = line->next; line != tailline; line = line->next) {
		    alloc_multidata_if_needed(line);
		    line->multidata[ink->id] = CWHOLELINE;
		}

		alloc_multidata_if_needed(tailline);
		tailline->multidata[ink->id] = CBEGINBEFORE;

		/* Begin looking for a new start after the end match. */
		index = endmatch.rm_eo;
	    }
	}
    }
}

#endif /* !DISABLE_COLOR */
>>>>>>> origin/tomato-shibby-RT-AC
