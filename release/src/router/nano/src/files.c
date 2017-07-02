/* $Id: files.c 4520 2010-11-12 06:23:14Z astyanax $ */
/**************************************************************************
 *   files.c  --  This file is part of GNU nano.                          *
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
 *   Copyright (C) 2015, 2016 Benno Schulenberg                           *
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
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
<<<<<<< HEAD

/* Add an entry to the openfile openfilestruct.  This should only be
 * called from open_buffer(). */
=======
#endif
#include <libgen.h>

#define LOCKBUFSIZE 8192

/* Verify that the containing directory of the given filename exists. */
bool has_valid_path(const char *filename)
{
    char *namecopy = mallocstrcpy(NULL, filename);
    char *parentdir = dirname(namecopy);
    struct stat parentinfo;
    bool validity = FALSE;

    if (stat(parentdir, &parentinfo) == -1) {
	if (errno == ENOENT)
	    statusline(ALERT, _("Directory '%s' does not exist"), parentdir);
	else
	    statusline(ALERT, _("Path '%s': %s"), parentdir, strerror(errno));
    } else if (!S_ISDIR(parentinfo.st_mode))
	statusline(ALERT, _("Path '%s' is not a directory"), parentdir);
    else if (access(parentdir, X_OK) == -1)
	statusline(ALERT, _("Path '%s' is not accessible"), parentdir);
    else if (ISSET(LOCKING) && access(parentdir, W_OK) == -1)
	statusline(MILD, _("Directory '%s' is not writable"), parentdir);
    else
	validity = TRUE;

    free(namecopy);

    return validity;
}

/* Add an entry to the circular list of openfile structs. */
>>>>>>> origin/tomato-shibby-RT-AC
void make_new_buffer(void)
{
    /* If there are no entries in openfile, make the first one and
     * move to it. */
    if (openfile == NULL) {
	openfile = make_new_opennode();
	splice_opennode(openfile, openfile, openfile);
    /* Otherwise, make a new entry for openfile, splice it in after
     * the current entry, and move to it. */
    } else {
	splice_opennode(openfile, make_new_opennode(), openfile->next);
	openfile = openfile->next;
    }

    /* Initialize the new buffer. */
    initialize_buffer();
}

/* Initialize the current entry of the openfile openfilestruct. */
void initialize_buffer(void)
{
    assert(openfile != NULL);

    openfile->filename = mallocstrcpy(NULL, "");

    initialize_buffer_text();

    openfile->placewewant = 0;
    openfile->current_y = 0;

    openfile->modified = FALSE;
#ifndef NANO_TINY
    openfile->mark_set = FALSE;

    openfile->mark_begin = NULL;
    openfile->mark_begin_x = 0;
    openfile->kind_of_mark = SOFTMARK;

    openfile->fmt = NIX_FILE;

    openfile->current_stat = NULL;
    openfile->undotop = NULL;
    openfile->current_undo = NULL;
#endif
#ifdef ENABLE_COLOR
    openfile->colorstrings = NULL;
#endif
}

/* Initialize the text of the current entry of the openfile
 * openfilestruct. */
void initialize_buffer_text(void)
{
    assert(openfile != NULL);

    openfile->fileage = make_new_node(NULL);
    openfile->fileage->data = mallocstrcpy(NULL, "");

    openfile->filebot = openfile->fileage;
    openfile->edittop = openfile->fileage;
    openfile->current = openfile->fileage;

<<<<<<< HEAD
#ifdef ENABLE_COLOR
    openfile->fileage->multidata = NULL;
#endif

    openfile->totsize = 0;
=======
    openfile->firstcolumn = 0;
    openfile->current_x = 0;
    openfile->totsize = 0;
}

/* Mark the current file as modified if it isn't already, and then
 * update the titlebar to display the file's new status. */
void set_modified(void)
{
    if (openfile->modified)
	return;

    openfile->modified = TRUE;
    titlebar(NULL);

#ifndef NANO_TINY
    if (!ISSET(LOCKING) || openfile->filename[0] == '\0')
	return;

    if (openfile->lock_filename != NULL) {
	char *fullname = get_full_path(openfile->filename);

	write_lockfile(openfile->lock_filename, fullname, TRUE);
	free(fullname);
    }
#endif
}

#ifndef NANO_TINY
/* Actually write the lockfile.  This function will ALWAYS annihilate
 * any previous version of the file.  We'll borrow INSECURE_BACKUP here
 * to decide about lockfile paranoia here as well...
 *
 * Args:
 *     lockfilename: file name for lock
 *     origfilename: name of the file the lock is for
 *     modified: whether to set the modified bit in the file
 *
 * Returns 1 on success, and 0 on failure (but continue anyway). */
int write_lockfile(const char *lockfilename, const char *origfilename, bool modified)
{
#ifdef HAVE_PWD_H
    int cflags, fd;
    FILE *filestream;
    pid_t mypid;
    uid_t myuid;
    struct passwd *mypwuid;
    struct stat fileinfo;
    char *lockdata = charalloc(1024);
    char myhostname[32];
    size_t lockdatalen = 1024;
    size_t wroteamt;

    mypid = getpid();
    myuid = geteuid();

    /* First run things that might fail before blowing away the old state. */
    if ((mypwuid = getpwuid(myuid)) == NULL) {
	/* TRANSLATORS: Keep the next eight messages at most 76 characters. */
	statusline(MILD, _("Couldn't determine my identity for lock file "
				"(getpwuid() failed)"));
	goto free_the_data;
    }

    if (gethostname(myhostname, 31) < 0) {
	if (errno == ENAMETOOLONG)
	    myhostname[31] = '\0';
	else {
	    statusline(MILD, _("Couldn't determine hostname for lock file: %s"),
			strerror(errno));
	    goto free_the_data;
	}
    }

    /* If the lockfile exists, try to delete it. */
    if (stat(lockfilename, &fileinfo) != -1)
	if (delete_lockfile(lockfilename) < 0)
	    goto free_the_data;

    if (ISSET(INSECURE_BACKUP))
	cflags = O_WRONLY | O_CREAT | O_APPEND;
    else
	cflags = O_WRONLY | O_CREAT | O_EXCL | O_APPEND;

    /* Try to create the lockfile. */
    fd = open(lockfilename, cflags,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (fd < 0) {
	statusline(MILD, _("Error writing lock file %s: %s"),
			lockfilename, strerror(errno));
	goto free_the_data;
    }

    /* Try to associate a stream with the now open lockfile. */
    filestream = fdopen(fd, "wb");

    if (filestream == NULL) {
	statusline(MILD, _("Error writing lock file %s: %s"), lockfilename,
			strerror(errno));
	goto free_the_data;
    }

    /* This is the lock data we will store:
     *
     * byte 0        - 0x62
     * byte 1        - 0x30
     * bytes 2-12    - program name which created the lock
     * bytes 24-27   - PID (little endian) of creator process
     * bytes 28-44   - username of who created the lock
     * bytes 68-100  - hostname of where the lock was created
     * bytes 108-876 - filename the lock is for
     * byte 1007     - 0x55 if file is modified
     *
     * Looks like VIM also stores undo state in this file, so we're
     * gonna have to figure out how to slap a 'OMG don't use recover on
     * our lockfile' message in here...
     *
     * This is likely very wrong, so this is a WIP. */
    memset(lockdata, 0, lockdatalen);
    lockdata[0] = 0x62;
    lockdata[1] = 0x30;
    lockdata[24] = mypid % 256;
    lockdata[25] = (mypid / 256) % 256;
    lockdata[26] = (mypid / (256 * 256)) % 256;
    lockdata[27] = mypid / (256 * 256 * 256);
    snprintf(&lockdata[2], 11, "nano %s", VERSION);
    strncpy(&lockdata[28], mypwuid->pw_name, 16);
    strncpy(&lockdata[68], myhostname, 31);
    if (origfilename != NULL)
	strncpy(&lockdata[108], origfilename, 768);
    if (modified == TRUE)
	lockdata[1007] = 0x55;

    wroteamt = fwrite(lockdata, sizeof(char), lockdatalen, filestream);
    if (wroteamt < lockdatalen) {
	statusline(MILD, _("Error writing lock file %s: %s"),
			lockfilename, ferror(filestream));
	goto free_the_data;
    }

#ifdef DEBUG
    fprintf(stderr, "In write_lockfile(), write successful (wrote %lu bytes)\n", (unsigned long)wroteamt);
#endif

    if (fclose(filestream) == EOF) {
	statusline(MILD, _("Error writing lock file %s: %s"),
			lockfilename, strerror(errno));
	goto free_the_data;
    }

    openfile->lock_filename = (char *) lockfilename;

    free(lockdata);
    return 1;

  free_the_data:
    free(lockdata);
    return 0;
#else
    return 1;
#endif
}

/* Delete the lockfile.  Return -1 if unsuccessful, and 1 otherwise. */
int delete_lockfile(const char *lockfilename)
{
    if (unlink(lockfilename) < 0 && errno != ENOENT) {
	statusline(MILD, _("Error deleting lock file %s: %s"), lockfilename,
			strerror(errno));
	return -1;
    }
    return 1;
}

/* Deal with lockfiles.  Return -1 on refusing to override the lockfile,
 * and 1 on successfully creating it; 0 means we were not successful in
 * creating the lockfile but we should continue to load the file. */
int do_lockfile(const char *filename)
{
    char *namecopy = (char *) mallocstrcpy(NULL, filename);
    char *secondcopy = (char *) mallocstrcpy(NULL, filename);
    size_t locknamesize = strlen(filename) + strlen(locking_prefix)
		+ strlen(locking_suffix) + 3;
    char *lockfilename = charalloc(locknamesize);
    static char lockprog[11], lockuser[17];
    struct stat fileinfo;
    int lockfd, lockpid, retval = -1;

    snprintf(lockfilename, locknamesize, "%s/%s%s%s", dirname(namecopy),
		locking_prefix, basename(secondcopy), locking_suffix);
    free(secondcopy);
#ifdef DEBUG
    fprintf(stderr, "lock file name is %s\n", lockfilename);
#endif
    if (stat(lockfilename, &fileinfo) != -1) {
	size_t readtot = 0;
	size_t readamt = 0;
	char *lockbuf, *question, *pidstring, *postedname, *promptstr;
	int room, response;

	if ((lockfd = open(lockfilename, O_RDONLY)) < 0) {
	    statusline(MILD, _("Error opening lock file %s: %s"),
			lockfilename, strerror(errno));
	    goto free_the_name;
	}

	lockbuf = charalloc(LOCKBUFSIZE);
	do {
	    readamt = read(lockfd, &lockbuf[readtot], LOCKBUFSIZE - readtot);
	    readtot += readamt;
	} while (readamt > 0 && readtot < LOCKBUFSIZE);

	close(lockfd);

	if (readtot < 48) {
	    statusline(MILD, _("Error reading lock file %s: "
			"Not enough data read"), lockfilename);
	    free(lockbuf);
	    goto free_the_name;
	}

	strncpy(lockprog, &lockbuf[2], 10);
	lockpid = (((unsigned char)lockbuf[27] * 256 + (unsigned char)lockbuf[26]) * 256 +
			(unsigned char)lockbuf[25]) * 256 + (unsigned char)lockbuf[24];
	strncpy(lockuser, &lockbuf[28], 16);
	free(lockbuf);

	pidstring = charalloc(11);
	sprintf (pidstring, "%u", (unsigned int)lockpid);

#ifdef DEBUG
	fprintf(stderr, "lockpid = %d\n", lockpid);
	fprintf(stderr, "program name which created this lock file should be %s\n", lockprog);
	fprintf(stderr, "user which created this lock file should be %s\n", lockuser);
#endif
	/* TRANSLATORS: The second %s is the name of the user, the third that of the editor. */
	question = _("File %s is being edited (by %s with %s, PID %s); continue?");
	room = COLS - strlenpt(question) + 7 - strlenpt(lockuser) -
				strlenpt(lockprog) - strlenpt(pidstring);
	if (room < 4)
	    postedname = mallocstrcpy(NULL, "_");
	else if (room < strlenpt(filename)) {
	    char *fragment = display_string(filename,
				strlenpt(filename) - room + 3, room, FALSE);
	    postedname = charalloc(strlen(fragment) + 4);
	    strcpy(postedname, "...");
	    strcat(postedname, fragment);
	    free(fragment);
	} else
	    postedname = mallocstrcpy(NULL, filename);

	/* Allow extra space for username (14), program name (8), PID (8),
	 * and terminating \0 (1), minus the %s (2) for the file name. */
	promptstr = charalloc(strlen(question) + 29 + strlen(postedname));
	sprintf(promptstr, question, postedname, lockuser, lockprog, pidstring);
	free(postedname);
	free(pidstring);

	response = do_yesno_prompt(FALSE, promptstr);
	free(promptstr);

	if (response < 1) {
	    blank_statusbar();
	    goto free_the_name;
	}
    }

    retval = write_lockfile(lockfilename, filename, FALSE);

  free_the_name:
    free(namecopy);
    if (retval < 1)
	free(lockfilename);

    return retval;
>>>>>>> origin/tomato-shibby-RT-AC
}

/* If it's not "", filename is a file to open.  We make a new buffer, if
 * necessary, and then open and read the file, if applicable. */
void open_buffer(const char *filename, bool undoable)
{
<<<<<<< HEAD
    bool new_buffer = (openfile == NULL
#ifdef ENABLE_MULTIBUFFER
	 || ISSET(MULTIBUFFER)
#endif
	);
	/* Whether we load into this buffer or a new one. */
=======
    bool new_buffer = (openfile == NULL || ISSET(MULTIBUFFER));
	/* Whether we load into the current buffer or a new one. */
    char *realname;
	/* The filename after tilde expansion. */
>>>>>>> origin/tomato-shibby-RT-AC
    FILE *f;
    int rc;
	/* rc == -2 means that we have a new file.  -1 means that the
	 * open() failed.  0 means that the open() succeeded. */

    assert(filename != NULL);

    /* Display newlines in filenames as ^J. */
    as_an_at = FALSE;

#ifndef DISABLE_OPERATINGDIR
    if (check_operating_dir(filename, FALSE)) {
	statusbar(_("Can't insert file from outside of %s"),
		operating_dir);
	return;
    }
#endif

    /* If the filename isn't blank, open the file.  Otherwise, treat it
     * as a new file. */
    rc = (filename[0] != '\0') ? open_file(filename, new_buffer, &f) :
	-2;

    /* If we're loading into a new buffer, add a new entry to
     * openfile. */
    if (new_buffer)
	make_new_buffer();

    /* If we have a file, and we're loading into a new buffer, update
     * the filename. */
    if (rc != -1 && new_buffer)
	openfile->filename = mallocstrcpy(openfile->filename, filename);

    /* If we have a non-new file, read it in.  Then, if the buffer has
     * no stat, update the stat, if applicable. */
    if (rc > 0) {
	read_file(f, rc, filename, undoable, new_buffer);
#ifndef NANO_TINY
	if (openfile->current_stat == NULL) {
	    openfile->current_stat =
		(struct stat *)nmalloc(sizeof(struct stat));
	    stat(filename, openfile->current_stat);
	}
#endif
    }

    /* If we have a file, and we're loading into a new buffer, move back
     * to the beginning of the first line of the buffer. */
    if (rc != -1 && new_buffer) {
	openfile->current = openfile->fileage;
	openfile->current_x = 0;
	openfile->placewewant = 0;
    }

#ifdef ENABLE_COLOR
    /* If we're loading into a new buffer, update the colors to account
     * for it, if applicable. */
    if (new_buffer)
	color_update();
#endif
}

#ifndef DISABLE_SPELLER
/* If it's not "", filename is a file to open.  We blow away the text of
 * the current buffer, and then open and read the file, if
 * applicable.  Note that we skip the operating directory test when
 * doing this. */
void replace_buffer(const char *filename)
{
    FILE *f;
    int rc;
	/* rc == -2 means that we have a new file.  -1 means that the
	 * open() failed.  0 means that the open() succeeded. */

    assert(filename != NULL);

    /* If the filename isn't blank, open the file.  Otherwise, treat it
     * as a new file. */
    rc = (filename[0] != '\0') ? open_file(filename, TRUE, &f) : -2;

    /* Reinitialize the text of the current buffer. */
    free_filestruct(openfile->fileage);
    initialize_buffer_text();

    /* If we have a non-new file, read it in. */
    if (rc > 0)
	read_file(f, rc, filename, FALSE, TRUE);

    /* Move back to the beginning of the first line of the buffer. */
    openfile->current = openfile->fileage;
    openfile->current_x = 0;
    openfile->placewewant = 0;
}

#ifndef NANO_TINY
/* Open the specified file, and if that succeeds, blow away the text of
 * the current buffer at the given coordinates and read the file
 * contents into its place. */
void replace_marked_buffer(const char *filename, filestruct *top, size_t top_x,
	filestruct *bot, size_t bot_x)
{
    FILE *f;
    int descriptor;
    bool old_no_newlines = ISSET(NO_NEWLINES);
    filestruct *trash_top = NULL;
    filestruct *trash_bot = NULL;

    descriptor = open_file(filename, FALSE, TRUE, &f);

    if (descriptor < 0)
	return;

    /* Don't add a magicline when replacing text in the buffer. */
    SET(NO_NEWLINES);

    /* Throw away the text under the mark, and insert the processed file
     * where the marked text was. */
    extract_buffer(&trash_top, &trash_bot, top, top_x, bot, bot_x);
    free_filestruct(trash_top);
    read_file(f, descriptor, filename, FALSE, TRUE);

    /* Restore the magicline behavior now that we're done fiddling. */
    if (!old_no_newlines)
	UNSET(NO_NEWLINES);
}
#endif /* !ENABLE_TINY */
#endif /* !DISABLE_SPELLER */

/* Update the screen to account for the current buffer. */
void display_buffer(void)
{
    /* Update the titlebar, since the filename may have changed. */
    titlebar(NULL);

#ifdef ENABLE_COLOR
    /* Make sure we're using the buffer's associated colors, if
     * applicable. */
    color_init();
#endif

    /* Update the content of the edit window straightaway. */
    edit_refresh();
}

#ifdef ENABLE_MULTIBUFFER
/* Switch to the next file buffer if next_buf is TRUE.  Otherwise,
 * switch to the previous file buffer. */
void switch_to_prevnext_buffer(bool next_buf)
{
    assert(openfile != NULL);

    /* If only one file buffer is open, indicate it on the statusbar and
     * get out. */
    if (openfile == openfile->next) {
	statusbar(_("No more open file buffers"));
	return;
    }

    /* Switch to the next or previous file buffer, depending on the
     * value of next_buf. */
    openfile = next_buf ? openfile->next : openfile->prev;

#ifdef DEBUG
    fprintf(stderr, "filename is %s\n", openfile->filename);
#endif

#ifndef NANO_TINY
    /* When not in softwrap mode, make sure firstcolumn is zero.  It might
     * be nonzero if we had softwrap mode on while in this buffer, and then
     * turned softwrap mode off while in a different buffer. */
    if (!ISSET(SOFTWRAP))
	openfile->firstcolumn = 0;
#endif

    /* Update the screen to account for the current buffer. */
    display_buffer();

    /* Indicate the switch on the statusbar. */
    statusbar(_("Switched to %s"),
	((openfile->filename[0] == '\0') ? _("New Buffer") :
	openfile->filename));

#ifdef DEBUG
    dump_filestruct(openfile->current);
#endif
}

/* Switch to the previous entry in the openfile filebuffer. */
void switch_to_prev_buffer_void(void)
{
    switch_to_prevnext_buffer(FALSE);
}

/* Switch to the next entry in the openfile filebuffer. */
void switch_to_next_buffer_void(void)
{
    switch_to_prevnext_buffer(TRUE);
}

/* Delete an entry from the openfile filebuffer, and switch to the one
 * after it.  Return TRUE on success, or FALSE if there are no more open
 * file buffers. */
bool close_buffer(void)
{
    assert(openfile != NULL);

    /* If only one file buffer is open, get out. */
    if (openfile == openfile->next)
	return FALSE;

    /* Switch to the next file buffer. */
    switch_to_next_buffer_void();

    /* Close the file buffer we had open before. */
    unlink_opennode(openfile->prev);

    display_main_list();

    return TRUE;
}
#endif /* ENABLE_MULTIBUFFER */

/* A bit of a copy and paste from open_file(), is_file_writable()
 * just checks whether the file is appendable as a quick
 * permissions check, and we tend to err on the side of permissiveness
 * (reporting TRUE when it might be wrong) to not fluster users
 * editing on odd filesystems by printing incorrect warnings.
 */
int is_file_writable(const char *filename)
{
    struct stat fileinfo, fileinfo2;
    int fd;
    FILE *f;
    char *full_filename;
    bool ans = TRUE;


    if (ISSET(VIEW_MODE))
	return TRUE;

    assert(filename != NULL);

    /* Get the specified file's full path. */
    full_filename = get_full_path(filename);

    /* Okay, if we can't stat the path due to a component's
       permissions, just try the relative one */
    if (full_filename == NULL
        || (stat(full_filename, &fileinfo) == -1 && stat(filename, &fileinfo2) != -1))
        full_filename = mallocstrcpy(NULL, filename);

    if ((fd = open(full_filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR |
                S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1
	|| (f = fdopen(fd, "a")) == NULL)
	ans = FALSE;
    else
        fclose(f);
    close(fd);

    free(full_filename);
    return ans;
}

<<<<<<< HEAD
/* We make a new line of text from buf.  buf is length buf_len.  If
 * first_line_ins is TRUE, then we put the new line at the top of the
 * file.  Otherwise, we assume prevnode is the last line of the file,
 * and put our line after prevnode. */
filestruct *read_line(char *buf, filestruct *prevnode, bool
	*first_line_ins, size_t buf_len)
{
    filestruct *fileptr = (filestruct *)nmalloc(sizeof(filestruct));

    /* Convert nulls to newlines.  buf_len is the string's real
     * length. */
=======
/* Encode any NUL bytes in the given line of text, which is of length buf_len,
 * and return a dynamically allocated copy of the resultant string. */
char *encode_data(char *buf, size_t buf_len)
{
>>>>>>> origin/tomato-shibby-RT-AC
    unsunder(buf, buf_len);

<<<<<<< HEAD
    assert(openfile->fileage != NULL && strlen(buf) == buf_len);

    fileptr->data = mallocstrcpy(NULL, buf);

#ifndef NANO_TINY
    /* If it's a DOS file ("\r\n"), and file conversion isn't disabled,
     * strip the '\r' part from fileptr->data. */
    if (!ISSET(NO_CONVERT) && buf_len > 0 && buf[buf_len - 1] == '\r')
	fileptr->data[buf_len - 1] = '\0';
#endif

#ifdef ENABLE_COLOR
	fileptr->multidata = NULL;
#endif

    if (*first_line_ins) {
	/* Special case: We're inserting with the cursor on the first
	 * line. */
	fileptr->prev = NULL;
	fileptr->next = openfile->fileage;
	fileptr->lineno = 1;
	if (*first_line_ins) {
	    *first_line_ins = FALSE;
	    /* If we're inserting into the first line of the file, then
	     * we want to make sure that our edit buffer stays on the
	     * first line and that fileage stays up to date. */
	    openfile->edittop = fileptr;
	} else
	    openfile->filebot = fileptr;
	openfile->fileage = fileptr;
    } else {
	assert(prevnode != NULL);

	fileptr->prev = prevnode;
	fileptr->next = NULL;
	fileptr->lineno = prevnode->lineno + 1;
	prevnode->next = fileptr;
    }

    return fileptr;
=======
    return mallocstrcpy(NULL, buf);
>>>>>>> origin/tomato-shibby-RT-AC
}

/* Read an open file into the current buffer.  f should be set to the
 * open file, and filename should be set to the name of the file.
<<<<<<< HEAD
 * undoable  means do we want to create undo records to try and undo this.
 * Will also attempt to check file writability if fd > 0 and checkwritable == TRUE
 */
void read_file(FILE *f, int fd, const char *filename, bool undoable, bool checkwritable)
=======
 * undoable means do we want to create undo records to try and undo
 * this.  Will also attempt to check file writability if fd > 0 and
 * checkwritable == TRUE. */
void read_file(FILE *f, int fd, const char *filename, bool undoable,
		bool checkwritable)
>>>>>>> origin/tomato-shibby-RT-AC
{
    ssize_t was_lineno = openfile->current->lineno;
	/* The line number where we start the insertion. */
    size_t was_leftedge = 0;
	/* The leftedge where we start the insertion. */
    size_t num_lines = 0;
	/* The number of lines in the file. */
    size_t len = 0;
	/* The length of the current line of the file. */
<<<<<<< HEAD
    size_t i = 0;
	/* The position in the current line of the file. */
    size_t bufx = MAX_BUF_SIZE;
	/* The size of each chunk of the file that we read. */
    char input = '\0';
	/* The current input character. */
    char *buf;
	/* The buffer where we store chunks of the file. */
    filestruct *fileptr = openfile->current;
	/* The current line of the file. */
    bool first_line_ins = FALSE;
	/* Whether we're inserting with the cursor on the first line. */
=======
    char input = '\0';
	/* The current input character. */
    char *buf;
	/* The buffer in which we assemble each line of the file. */
    size_t bufx = MAX_BUF_SIZE;
	/* The allocated size of the line buffer; increased as needed. */
    filestruct *topline;
	/* The top of the new buffer where we store the read file. */
    filestruct *bottomline;
	/* The bottom of the new buffer. */
>>>>>>> origin/tomato-shibby-RT-AC
    int input_int;
	/* The current value we read from the file, whether an input
	 * character or EOF. */
    bool writable = TRUE;
	/* Whether the file is writable (in case we care). */
#ifndef NANO_TINY
    int format = 0;
	/* 0 = *nix, 1 = DOS, 2 = Mac, 3 = both DOS and Mac. */
#endif

    buf = charalloc(bufx);
    buf[0] = '\0';

#ifndef NANO_TINY
    if (undoable)
	add_undo(INSERT);

    if (ISSET(SOFTWRAP))
	was_leftedge = (xplustabs() / editwincols) * editwincols;
#endif

<<<<<<< HEAD
    if (openfile->current == openfile->fileage)
	first_line_ins = TRUE;
    else
	fileptr = openfile->current->prev;

    /* Read the entire file into the filestruct. */
=======
    /* Create an empty buffer. */
    topline = make_new_node(NULL);
    bottomline = topline;

    /* Read the entire file into the new buffer. */
>>>>>>> origin/tomato-shibby-RT-AC
    while ((input_int = getc(f)) != EOF) {
	input = (char)input_int;

	/* If it's a *nix file ("\n") or a DOS file ("\r\n"), and file
	 * conversion isn't disabled, handle it! */
	if (input == '\n') {
#ifndef NANO_TINY
	    /* If it's a DOS file or a DOS/Mac file ('\r' before '\n' on
	     * the first line if we think it's a *nix file, or on any
	     * line otherwise), and file conversion isn't disabled,
	     * handle it! */
	    if (!ISSET(NO_CONVERT) && (num_lines == 0 || format != 0) &&
		i > 0 && buf[i - 1] == '\r') {
		if (format == 0 || format == 2)
		    format++;
	    }
<<<<<<< HEAD
#endif

	    /* Read in the line properly. */
	    fileptr = read_line(buf, fileptr, &first_line_ins, len);

	    /* Reset the line length in preparation for the next
	     * line. */
	    len = 0;

	    num_lines++;
	    buf[0] = '\0';
	    i = 0;
#ifndef NANO_TINY
=======
>>>>>>> origin/tomato-shibby-RT-AC
	/* If it's a Mac file ('\r' without '\n' on the first line if we
	 * think it's a *nix file, or on any line otherwise), and file
	 * conversion isn't disabled, handle it! */
	} else if (!ISSET(NO_CONVERT) && (num_lines == 0 ||
		format != 0) && i > 0 && buf[i - 1] == '\r') {
	    /* If we currently think the file is a *nix file, set format
	     * to Mac.  If we currently think the file is a DOS file,
	     * set format to both DOS and Mac. */
	    if (format == 0 || format == 1)
		format += 2;
<<<<<<< HEAD

	    /* Read in the line properly. */
	    fileptr = read_line(buf, fileptr, &first_line_ins, len);

	    /* Reset the line length in preparation for the next line.
	     * Since we've already read in the next character, reset it
	     * to 1 instead of 0. */
	    len = 1;

	    num_lines++;
	    buf[0] = input;
	    buf[1] = '\0';
	    i = 1;
=======
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	} else {
	    /* Calculate the total length of the line.  It might have
	     * nulls in it, so we can't just use strlen() here. */
	    len++;

	    /* Now we allocate a bigger buffer MAX_BUF_SIZE characters
	     * at a time.  If we allocate a lot of space for one line,
	     * we may indeed have to use a buffer this big later on, so
	     * we don't decrease it at all.  We do free it at the end,
	     * though. */
	    if (i >= bufx - 1) {
		bufx += MAX_BUF_SIZE;
		buf = charealloc(buf, bufx);
	    }
<<<<<<< HEAD

	    buf[i] = input;
	    buf[i + 1] = '\0';
	    i++;
=======
	    continue;
>>>>>>> origin/tomato-shibby-RT-AC
	}

#ifndef NANO_TINY
	/* If it's a DOS or Mac line, strip the '\r' from it. */
	if (len > 0 && buf[len - 1] == '\r' && !ISSET(NO_CONVERT))
	    buf[--len] = '\0';
#endif

	/* Store the data and make a new line. */
	bottomline->data = encode_data(buf, len);
	bottomline->next = make_new_node(bottomline);
	bottomline = bottomline->next;
	num_lines++;

	/* Reset the length in preparation for the next line. */
	len = 0;

#ifndef NANO_TINY
	/* If it happens to be a Mac line, store the character after the \r
	 * as the first character of the next line. */
	if (input != '\n')
	    buf[len++] = input;
#endif
    }

    /* Perhaps this could use some better handling. */
    if (ferror(f))
	nperror(filename);
    fclose(f);
    if (fd > 0 && checkwritable) {
	close(fd);
	writable = is_file_writable(filename);
    }

<<<<<<< HEAD
#ifndef NANO_TINY
    /* If file conversion isn't disabled and the last character in this
     * file is '\r', read it in properly as a Mac format line. */
    if (len == 0 && !ISSET(NO_CONVERT) && input == '\r') {
	len = 1;

	buf[0] = input;
	buf[1] = '\0';
    }
#endif

    /* Did we not get a newline and still have stuff to do? */
    if (len > 0) {
#ifndef NANO_TINY
	/* If file conversion isn't disabled and the last character in
	 * this file is '\r', set format to Mac if we currently think
	 * the file is a *nix file, or to both DOS and Mac if we
	 * currently think the file is a DOS file. */
	if (!ISSET(NO_CONVERT) && buf[len - 1] == '\r' &&
		(format == 0 || format == 1))
	    format += 2;
#endif

	/* Read in the last line properly. */
	fileptr = read_line(buf, fileptr, &first_line_ins, len);
	num_lines++;
    }
=======
    /* If the file ended with newline, or it was entirely empty, make the
     * last line blank.  Otherwise, put the last read data in. */
    if (len == 0)
	bottomline->data = mallocstrcpy(NULL, "");
    else {
	bool mac_line_needs_newline = FALSE;

#ifndef NANO_TINY
	/* If the final character is '\r', and file conversion isn't disabled,
	 * set format to Mac if we currently think the file is a *nix file, or
	 * to DOS-and-Mac if we currently think it is a DOS file. */
	if (buf[len - 1] == '\r' && !ISSET(NO_CONVERT)) {
	    if (format < 2)
		format += 2;
>>>>>>> origin/tomato-shibby-RT-AC

	    /* Strip the carriage return. */
	    buf[--len] = '\0';

<<<<<<< HEAD
    /* If we didn't get a file and we don't already have one, open a
     * blank buffer. */
    if (fileptr == NULL)
	open_buffer("", FALSE);

    /* Attach the file we got to the filestruct.  If we got a file of
     * zero bytes, don't do anything. */
    if (num_lines > 0) {
	/* If the file we got doesn't end in a newline, tack its last
	 * line onto the beginning of the line at current. */
	if (len > 0) {
	    size_t current_len = strlen(openfile->current->data);

	    /* Adjust the current x-coordinate to compensate for the
	     * change in the current line. */
	    if (num_lines == 1)
		openfile->current_x += len;
	    else
		openfile->current_x = len;

	    /* Tack the text at fileptr onto the beginning of the text
	     * at current. */
	    openfile->current->data =
		charealloc(openfile->current->data, len +
		current_len + 1);
	    charmove(openfile->current->data + len,
		openfile->current->data, current_len + 1);
	    strncpy(openfile->current->data, fileptr->data, len);

	    /* Don't destroy fileage, edittop, or filebot! */
	    if (fileptr == openfile->fileage)
		openfile->fileage = openfile->current;
	    if (fileptr == openfile->edittop)
		openfile->edittop = openfile->current;
	    if (fileptr == openfile->filebot)
		openfile->filebot = openfile->current;

	    /* Move fileptr back one line and blow away the old fileptr,
	     * since its text has been saved. */
	    fileptr = fileptr->prev;
	    if (fileptr != NULL) {
		if (fileptr->next != NULL)
		    free(fileptr->next);
	    }
=======
	    /* Indicate we need to put a blank line in after this one. */
	    mac_line_needs_newline = TRUE;
>>>>>>> origin/tomato-shibby-RT-AC
	}
#endif
	/* Store the data of the final line. */
	bottomline->data = encode_data(buf, len);
	num_lines++;

<<<<<<< HEAD
	/* Attach the line at current after the line at fileptr. */
	if (fileptr != NULL) {
	    fileptr->next = openfile->current;
	    openfile->current->prev = fileptr;
	}

	/* Renumber starting with the last line of the file we
	 * inserted. */
	renumber(openfile->current);
    }

    openfile->totsize += get_totsize(openfile->fileage,
	openfile->filebot);
=======
	if (mac_line_needs_newline) {
	    bottomline->next = make_new_node(bottomline);
	    bottomline = bottomline->next;
	    bottomline->data = mallocstrcpy(NULL, "");
	}
    }

    free(buf);
>>>>>>> origin/tomato-shibby-RT-AC

    /* Insert the just read buffer into the current one. */
    ingraft_buffer(topline);

    /* Set the desired x position at the end of what was inserted. */
    openfile->placewewant = xplustabs();

<<<<<<< HEAD
#ifndef NANO_TINY
    if (undoable)
	update_undo(INSERT);

    if (format == 3) {
	if (writable)
	    statusbar(
		P_("Read %lu line (Converted from DOS and Mac format)",
		"Read %lu lines (Converted from DOS and Mac format)",
		(unsigned long)num_lines), (unsigned long)num_lines);
	else
	    statusbar(
		P_("Read %lu line (Converted from DOS and Mac format - Warning: No write permission)",
		"Read %lu lines (Converted from DOS and Mac format - Warning: No write permission)",
		(unsigned long)num_lines), (unsigned long)num_lines);
    } else if (format == 2) {
	openfile->fmt = MAC_FILE;
	if (writable)
	    statusbar(P_("Read %lu line (Converted from Mac format)",
		"Read %lu lines (Converted from Mac format)",
		(unsigned long)num_lines), (unsigned long)num_lines);
	else
	    statusbar(P_("Read %lu line (Converted from Mac format - Warning: No write permission)",
		"Read %lu lines (Converted from Mac format - Warning: No write permission)",
		(unsigned long)num_lines), (unsigned long)num_lines);
    } else if (format == 1) {
	openfile->fmt = DOS_FILE;
	if (writable)
	    statusbar(P_("Read %lu line (Converted from DOS format)",
		"Read %lu lines (Converted from DOS format)",
		(unsigned long)num_lines), (unsigned long)num_lines);
	else
	    statusbar(P_("Read %lu line (Converted from DOS format - Warning: No write permission)",
		"Read %lu lines (Converted from DOS format - Warning: No write permission)",
		(unsigned long)num_lines), (unsigned long)num_lines);
    } else
=======
    if (!writable)
	statusline(ALERT, _("File '%s' is unwritable"), filename);
#ifndef NANO_TINY
    else if (format == 3) {
	/* TRANSLATORS: Keep the next four messages at most 78 characters. */
	statusline(HUSH, P_("Read %lu line (Converted from DOS and Mac format)",
			"Read %lu lines (Converted from DOS and Mac format)",
			(unsigned long)num_lines), (unsigned long)num_lines);
    } else if (format == 2) {
	openfile->fmt = MAC_FILE;
	statusline(HUSH, P_("Read %lu line (Converted from Mac format)",
			"Read %lu lines (Converted from Mac format)",
			(unsigned long)num_lines), (unsigned long)num_lines);
    } else if (format == 1) {
	openfile->fmt = DOS_FILE;
	statusline(HUSH, P_("Read %lu line (Converted from DOS format)",
			"Read %lu lines (Converted from DOS format)",
			(unsigned long)num_lines), (unsigned long)num_lines);
    }
#endif
    else
	statusline(HUSH, P_("Read %lu line", "Read %lu lines",
			(unsigned long)num_lines), (unsigned long)num_lines);

    /* If we inserted less than a screenful, don't center the cursor. */
    if (less_than_a_screenful(was_lineno, was_leftedge))
	focusing = FALSE;

#ifndef NANO_TINY
    if (undoable)
	update_undo(INSERT);

    if (ISSET(MAKE_IT_UNIX))
	openfile->fmt = NIX_FILE;
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	if (writable)
	    statusbar(P_("Read %lu line", "Read %lu lines",
		(unsigned long)num_lines), (unsigned long)num_lines);
	else
	    statusbar(P_("Read %lu line ( Warning: No write permission)",
		"Read %lu lines (Warning: No write permission)",
		(unsigned long)num_lines), (unsigned long)num_lines);
}

/* Open the file with the given name.  If the file does not exist, display
 * "New File" if newfie is TRUE, and say "File not found" otherwise.
 * Return -2 if we say "New File", -1 if the file isn't opened, and the
<<<<<<< HEAD
 * fd opened otherwise.  The file might still have an error while reading 
 * with a 0 return value.  *f is set to the opened file. */
int open_file(const char *filename, bool newfie, FILE **f)
=======
 * obtained fd otherwise.  *f is set to the opened file. */
int open_file(const char *filename, bool newfie, bool quiet, FILE **f)
>>>>>>> origin/tomato-shibby-RT-AC
{
    struct stat fileinfo, fileinfo2;
    int fd;
    char *full_filename;

    assert(filename != NULL && f != NULL);

    /* Get the specified file's full path. */
    full_filename = get_full_path(filename);

    /* Okay, if we can't stat the path due to a component's
<<<<<<< HEAD
       permissions, just try the relative one */
    if (full_filename == NULL 
	|| (stat(full_filename, &fileinfo) == -1 && stat(filename, &fileinfo2) != -1))
	full_filename = mallocstrcpy(NULL, filename);

    if (stat(full_filename, &fileinfo) == -1) {
	/* Well, maybe we can open the file even if the OS
	   says its not there */
        if ((fd = open(filename, O_RDONLY)) != -1) {
	    statusbar(_("Reading File"));
	    free(full_filename);
	    return 0;
	}

	if (newfie) {
	    statusbar(_("New File"));
	    return -2;
	}
	statusbar(_("\"%s\" not found"), filename);
	beep();
	return -1;
    } else if (S_ISDIR(fileinfo.st_mode) || S_ISCHR(fileinfo.st_mode) ||
	S_ISBLK(fileinfo.st_mode)) {
	/* Don't open directories, character files, or block files.
	 * Sorry, /dev/sndstat! */
	statusbar(S_ISDIR(fileinfo.st_mode) ?
		_("\"%s\" is a directory") :
		_("\"%s\" is a device file"), filename);
	beep();
	return -1;
    } else if ((fd = open(full_filename, O_RDONLY)) == -1) {
	statusbar(_("Error reading %s: %s"), filename,
		strerror(errno));
	beep();
 	return -1;
     } else {
	/* The file is A-OK.  Open it. */
=======
     * permissions, just try the relative one. */
    if (full_filename == NULL || (stat(full_filename, &fileinfo) == -1 &&
					stat(filename, &fileinfo2) != -1))
	full_filename = mallocstrcpy(full_filename, filename);

    if (stat(full_filename, &fileinfo) == -1) {
	if (newfie) {
	    if (!quiet)
		statusbar(_("New File"));
	    free(full_filename);
	    return -2;
	}

	statusline(ALERT, _("File \"%s\" not found"), filename);
	free(full_filename);
	return -1;
    }

    /* Don't open directories, character files, or block files. */
    if (S_ISDIR(fileinfo.st_mode) || S_ISCHR(fileinfo.st_mode) ||
				S_ISBLK(fileinfo.st_mode)) {
	statusline(ALERT, S_ISDIR(fileinfo.st_mode) ?
			_("\"%s\" is a directory") :
			_("\"%s\" is a device file"), filename);
	free(full_filename);
	return -1;
    }

    /* Try opening the file. */
    fd = open(full_filename, O_RDONLY);

    if (fd == -1)
	statusline(ALERT, _("Error reading %s: %s"), filename, strerror(errno));
    else {
	/* The file is A-OK.  Associate a stream with it. */
>>>>>>> origin/tomato-shibby-RT-AC
	*f = fdopen(fd, "rb");

	if (*f == NULL) {
	    statusbar(_("Error reading %s: %s"), filename,
		strerror(errno));
	    beep();
	    close(fd);
	} else
	    statusbar(_("Reading File"));
    }

    free(full_filename);

    return fd;
}

/* This function will return the name of the first available extension
 * of a filename (starting with [name][suffix], then [name][suffix].1,
 * etc.).  Memory is allocated for the return value.  If no writable
 * extension exists, we return "". */
char *get_next_filename(const char *name, const char *suffix)
{
    static int ulmax_digits = -1;
    unsigned long i = 0;
    char *buf;
    size_t namelen, suffixlen;

    assert(name != NULL && suffix != NULL);

    if (ulmax_digits == -1)
	ulmax_digits = digits(ULONG_MAX);

    namelen = strlen(name);
    suffixlen = strlen(suffix);

    buf = charalloc(namelen + suffixlen + ulmax_digits + 2);
    sprintf(buf, "%s%s", name, suffix);

    while (TRUE) {
	struct stat fs;

	if (stat(buf, &fs) == -1)
	    return buf;
	if (i == ULONG_MAX)
	    break;

	i++;
	sprintf(buf + namelen + suffixlen, ".%lu", i);
    }

    /* There is no possible save file: blank out the filename. */
    *buf = '\0';

    return buf;
}

/* Insert a file into the current buffer, or into a new buffer when
 * the MULTIBUFFER flag is set. */
void do_insertfile(void)
{
    int i;
    const char *msg;
    char *ans = mallocstrcpy(NULL, "");
	/* The last answer the user typed at the statusbar prompt. */
<<<<<<< HEAD
    filestruct *edittop_save = openfile->edittop;
    size_t current_x_save = openfile->current_x;
    ssize_t current_y_save = openfile->current_y;
    bool edittop_inside = FALSE, meta_key = FALSE, func_key = FALSE;
    const sc *s;
=======
>>>>>>> origin/tomato-shibby-RT-AC
#ifndef NANO_TINY
    bool execute = FALSE;
#endif

<<<<<<< HEAD
    currmenu = MINSERTFILE;
=======
    /* Display newlines in filenames as ^J. */
    as_an_at = FALSE;
>>>>>>> origin/tomato-shibby-RT-AC

    while (TRUE) {
#ifndef NANO_TINY
	if (execute) {
<<<<<<< HEAD
	    msg =
#ifdef ENABLE_MULTIBUFFER
		ISSET(MULTIBUFFER) ?
		_("Command to execute in new buffer [from %s] ") :
#endif
		_("Command to execute [from %s] ");
	} else {
#endif
	    msg =
#ifdef ENABLE_MULTIBUFFER
		ISSET(MULTIBUFFER) ?
		_("File to insert into new buffer [from %s] ") :
#endif
		_("File to insert [from %s] ");
#ifndef NANO_TINY
=======
#ifndef DISABLE_MULTIBUFFER
	    if (ISSET(MULTIBUFFER))
		/* TRANSLATORS: The next four messages are prompts. */
		msg = _("Command to execute in new buffer");
	    else
#endif
		msg = _("Command to execute");
	} else
#endif /* NANO_TINY */
	{
#ifndef DISABLE_MULTIBUFFER
	    if (ISSET(MULTIBUFFER))
		msg = _("File to insert into new buffer [from %s]");
	    else
#endif
		msg = _("File to insert [from %s]");
>>>>>>> origin/tomato-shibby-RT-AC
	}
#endif

	i = do_prompt(TRUE, TRUE,
#ifndef NANO_TINY
		execute ? MEXTCMD :
#endif
		MINSERTFILE, ans,
		&meta_key, &func_key,
#ifndef NANO_TINY
		NULL,
#endif
		edit_refresh, msg,
#ifndef DISABLE_OPERATINGDIR
		operating_dir != NULL ? operating_dir :
#endif
		"./");

	/* If we're in multibuffer mode and the filename or command is
<<<<<<< HEAD
	 * blank, open a new buffer instead of canceling.  If the
	 * filename or command begins with a newline (i.e. an encoded
	 * null), treat it as though it's blank. */
	if (i == -1 || ((i == -2 || *answer == '\n')
#ifdef ENABLE_MULTIBUFFER
		&& !ISSET(MULTIBUFFER)
#endif
		)) {
	    statusbar(_("Cancelled"));
	    break;
	} else {
	    size_t pww_save = openfile->placewewant;

	    ans = mallocstrcpy(ans, answer);

	    s = get_shortcut(currmenu, &i, &meta_key, &func_key);

#ifndef NANO_TINY
#ifdef ENABLE_MULTIBUFFER

	    if (s && s->scfunc == NEW_BUFFER_MSG) {
		/* Don't allow toggling if we're in view mode. */
=======
	 * blank, open a new buffer instead of canceling. */
	if (i == -1 || (i == -2 && !ISSET(MULTIBUFFER))) {
	    statusbar(_("Cancelled"));
	    break;
	} else {
	    ssize_t was_current_lineno = openfile->current->lineno;
	    size_t was_current_x = openfile->current_x;
#if !defined(NANO_TINY) || !defined(DISABLE_BROWSER)
	    functionptrtype func = func_from_key(&i);
#endif
	    given = mallocstrcpy(given, answer);

#ifndef NANO_TINY
#ifndef DISABLE_MULTIBUFFER
	    if (func == new_buffer_void) {
		/* Don't allow toggling when in view mode. */
>>>>>>> origin/tomato-shibby-RT-AC
		if (!ISSET(VIEW_MODE))
		    TOGGLE(MULTIBUFFER);
		continue;
	    } else
#endif
	    if (s && s->scfunc == EXT_CMD_MSG) {
		execute = !execute;
		continue;
	    }
#ifndef DISABLE_BROWSER
	    else
#endif
#endif /* !NANO_TINY */

#ifndef DISABLE_BROWSER
	    if (s && s->scfunc == TO_FILES_MSG) {
		char *tmp = do_browse_from(answer);

<<<<<<< HEAD
		if (tmp == NULL)
=======
		/* If no file was chosen, go back to the prompt. */
		if (chosen == NULL)
>>>>>>> origin/tomato-shibby-RT-AC
		    continue;

		free(answer);
		answer = tmp;

		i = 0;
	    }
#endif
<<<<<<< HEAD

	    /* If we don't have a file yet, go back to the statusbar
	     * prompt. */
	    if (i != 0
#ifdef ENABLE_MULTIBUFFER
		&& (i != -2 || !ISSET(MULTIBUFFER))
#endif
		)
		continue;

#ifndef NANO_TINY
		/* Keep track of whether the mark begins inside the
		 * partition and will need adjustment. */
		if (openfile->mark_set) {
		    filestruct *top, *bot;
		    size_t top_x, bot_x;

		    mark_order((const filestruct **)&top, &top_x,
			(const filestruct **)&bot, &bot_x,
			&right_side_up);

		    single_line = (top == bot);
		}
#endif

#ifdef ENABLE_MULTIBUFFER
	    if (!ISSET(MULTIBUFFER)) {
#endif
		/* If we're not inserting into a new buffer, partition
		 * the filestruct so that it contains no text and hence
		 * looks like a new buffer, and keep track of whether
		 * the top of the edit window is inside the
		 * partition. */
		filepart = partition_filestruct(openfile->current,
			openfile->current_x, openfile->current,
			openfile->current_x);
		edittop_inside =
			(openfile->edittop == openfile->fileage);
#ifdef ENABLE_MULTIBUFFER
	    }
#endif

	    /* Convert newlines to nulls, just before we insert the file
	     * or execute the command. */
	    sunder(answer);
	    align(&answer);

#ifndef NANO_TINY
	    if (execute) {
#ifdef ENABLE_MULTIBUFFER
=======
	    /* If we don't have a file yet, go back to the prompt. */
	    if (i != 0 && (!ISSET(MULTIBUFFER) || i != -2))
		continue;

#ifndef NANO_TINY
	    if (execute) {
#ifndef DISABLE_MULTIBUFFER
		/* When in multibuffer mode, first open a blank buffer. */
>>>>>>> origin/tomato-shibby-RT-AC
		if (ISSET(MULTIBUFFER))
		    open_buffer("", FALSE);
#endif
		/* Save the command's output in the current buffer. */
		execute_command(answer);

<<<<<<< HEAD
#ifdef ENABLE_MULTIBUFFER
=======
#ifndef DISABLE_MULTIBUFFER
		/* If this is a new buffer, put the cursor at the top. */
>>>>>>> origin/tomato-shibby-RT-AC
		if (ISSET(MULTIBUFFER)) {
		    openfile->current = openfile->fileage;
		    openfile->current_x = 0;
		    openfile->placewewant = 0;

		    set_modified();
		}
#endif
	    } else {
#endif /* !NANO_TINY */
		/* Make sure the path to the file specified in answer is
		 * tilde-expanded. */
<<<<<<< HEAD
		answer = mallocstrassn(answer,
			real_dir_from_tilde(answer));
=======
		answer = free_and_assign(answer, real_dir_from_tilde(answer));
>>>>>>> origin/tomato-shibby-RT-AC

		/* Save the file specified in answer in the current
		 * buffer. */
		open_buffer(answer, TRUE);
#ifndef NANO_TINY
	    }
#endif
<<<<<<< HEAD

#ifdef ENABLE_MULTIBUFFER
	    if (ISSET(MULTIBUFFER))
		/* Update the screen to account for the current
		 * buffer. */
=======
		    if (has_old_position(answer, &priorline, &priorcol))
			do_gotolinecolumn(priorline, priorcol, FALSE, FALSE);
		}
#endif /* !DISABLE_HISTORIES */
		/* Update stuff to account for the current buffer. */
>>>>>>> origin/tomato-shibby-RT-AC
		display_buffer();
	    else
#endif
	    {
<<<<<<< HEAD
		filestruct *top_save = openfile->fileage;

		/* If we were at the top of the edit window before, set
		 * the saved value of edittop to the new top of the edit
		 * window. */
		if (edittop_inside)
		    edittop_save = openfile->fileage;

		/* Update the current x-coordinate to account for the
		 * number of characters inserted on the current line.
		 * If the mark begins inside the partition, adjust the
		 * mark coordinates to compensate for the change in the
		 * current line. */
		openfile->current_x = strlen(openfile->filebot->data);
		if (openfile->fileage == openfile->filebot) {
#ifndef NANO_TINY
		    if (openfile->mark_set) {
			openfile->mark_begin = openfile->current;
			if (!right_side_up)
			    openfile->mark_begin_x +=
				openfile->current_x;
		    }
#endif
		    openfile->current_x += current_x_save;
		}
#ifndef NANO_TINY
		else if (openfile->mark_set) {
		    if (!right_side_up) {
			if (single_line) {
			    openfile->mark_begin = openfile->current;
			    openfile->mark_begin_x -= current_x_save;
			} else
			    openfile->mark_begin_x -=
				openfile->current_x;
		    }
		}
#endif

		/* Update the current y-coordinate to account for the
		 * number of lines inserted. */
		openfile->current_y += current_y_save;

		/* Unpartition the filestruct so that it contains all
		 * the text again.  Note that we've replaced the
		 * non-text originally in the partition with the text in
		 * the inserted file/executed command output. */
		unpartition_filestruct(&filepart);

		/* Renumber starting with the beginning line of the old
		 * partition. */
		renumber(top_save);

		/* Restore the old edittop. */
		openfile->edittop = edittop_save;

		/* Restore the old place we want. */
		openfile->placewewant = pww_save;

		/* Mark the file as modified. */
		set_modified();
=======
		/* Mark the file as modified if it changed. */
		if (openfile->current->lineno != was_current_lineno ||
			openfile->current_x != was_current_x)
		    set_modified();
>>>>>>> origin/tomato-shibby-RT-AC

		/* Update the cursor position to account for inserted lines. */
		reset_cursor();

		refresh_needed = TRUE;
	    }

	    break;
	}
    }
    shortcut_init(FALSE);

    free(ans);
}

/* If the current mode of operation allows it, go insert a file. */
void do_insertfile_void(void)
{
<<<<<<< HEAD

    if (ISSET(RESTRICTED)) {
        nano_disabled_msg();
	return;
    }

#ifdef ENABLE_MULTIBUFFER
    if (ISSET(VIEW_MODE) && !ISSET(MULTIBUFFER))
=======
    if (ISSET(RESTRICTED))
	show_restricted_warning();
#ifndef DISABLE_MULTIBUFFER
    else if (ISSET(VIEW_MODE) && !ISSET(MULTIBUFFER))
>>>>>>> origin/tomato-shibby-RT-AC
	statusbar(_("Key invalid in non-multibuffer mode"));
#endif
<<<<<<< HEAD
	do_insertfile(
#ifndef NANO_TINY
		FALSE
#endif
		);

    display_main_list();
=======
    else
	do_insertfile();
>>>>>>> origin/tomato-shibby-RT-AC
}

/* When passed "[relative path]" or "[relative path][filename]" in
 * origpath, return "[full path]" or "[full path][filename]" on success,
 * or NULL on error.  Do this if the file doesn't exist but the relative
 * path does, since the file could exist in memory but not yet on disk).
 * Don't do this if the relative path doesn't exist, since we won't be
 * able to go there. */
char *get_full_path(const char *origpath)
{
    struct stat fileinfo;
<<<<<<< HEAD
    char *d_here, *d_there, *d_there_file = NULL;
    const char *last_slash;
=======
    char *currentdir, *d_here, *d_there, *d_there_file = NULL;
    char *last_slash;
>>>>>>> origin/tomato-shibby-RT-AC
    bool path_only;

    if (origpath == NULL)
    	return NULL;

    /* Get the current directory.  If it doesn't exist, back up and try
     * again until we get a directory that does, and use that as the
     * current directory. */
    d_here = charalloc(PATH_MAX + 1);
    d_here = getcwd(d_here, PATH_MAX + 1);

    while (d_here == NULL) {
	if (chdir("..") == -1)
	    break;

	d_here = getcwd(d_here, PATH_MAX + 1);
    }

    /* If we succeeded, canonicalize it in d_here. */
    if (d_here != NULL) {
	/* If the current directory isn't "/", tack a slash onto the end
	 * of it. */
	if (strcmp(d_here, "/") != 0) {
	    d_here = charealloc(d_here, strlen(d_here) + 2);
	    strcat(d_here, "/");
	}
    /* Otherwise, set d_here to "". */
    } else
	d_here = mallocstrcpy(NULL, "");

    d_there = real_dir_from_tilde(origpath);

    /* If stat()ing d_there fails, assume that d_there refers to a new
     * file that hasn't been saved to disk yet.  Set path_only to TRUE
     * if d_there refers to a directory, and FALSE otherwise. */
    path_only = (stat(d_there, &fileinfo) != -1 &&
	S_ISDIR(fileinfo.st_mode));

    /* If path_only is TRUE, make sure d_there ends in a slash. */
    if (path_only) {
	size_t d_there_len = strlen(d_there);

	if (d_there[d_there_len - 1] != '/') {
	    d_there = charealloc(d_there, d_there_len + 2);
	    strcat(d_there, "/");
	}
    }

    /* Search for the last slash in d_there. */
    last_slash = strrchr(d_there, '/');

    /* If we didn't find one, then make sure the answer is in the format
     * "d_here/d_there". */
    if (last_slash == NULL) {
	assert(!path_only);

	d_there_file = d_there;
	d_there = d_here;
    } else {
	/* If path_only is FALSE, then save the filename portion of the
	 * answer (everything after the last slash) in d_there_file. */
	if (!path_only)
	    d_there_file = mallocstrcpy(NULL, last_slash + 1);

	/* Remove the filename portion of the answer from d_there. */
	*(last_slash + 1) = '\0';

	/* Go to the path specified in d_there. */
	if (chdir(d_there) == -1) {
	    free(d_there);
	    d_there = NULL;
	} else {
	    free(d_there);

	    /* Get the full path. */
	    d_there = charalloc(PATH_MAX + 1);
	    d_there = getcwd(d_there, PATH_MAX + 1);

	    /* If we succeeded, canonicalize it in d_there. */
	    if (d_there != NULL) {
		/* If the current directory isn't "/", tack a slash onto
		 * the end of it. */
		if (strcmp(d_there, "/") != 0) {
		    d_there = charealloc(d_there, strlen(d_there) + 2);
		    strcat(d_there, "/");
		}
	    } else
		/* Otherwise, set path_only to TRUE, so that we clean up
		 * correctly, free all allocated memory, and return
		 * NULL. */
		path_only = TRUE;

	    /* Finally, go back to the path specified in d_here,
	     * where we were before.  We don't check for a chdir()
	     * error, since we can do nothing if we get one. */
	    IGNORE_CALL_RESULT(chdir(d_here));

	    /* Free d_here, since we're done using it. */
	    free(d_here);
	}
    }

    /* At this point, if path_only is FALSE and d_there isn't NULL,
     * d_there contains the path portion of the answer and d_there_file
     * contains the filename portion of the answer.  If this is the
     * case, tack the latter onto the end of the former.  d_there will
     * then contain the complete answer. */
    if (!path_only && d_there != NULL) {
	d_there = charealloc(d_there, strlen(d_there) +
		strlen(d_there_file) + 1);
	strcat(d_there, d_there_file);
    }

    /* Free d_there_file, since we're done using it. */
    if (d_there_file != NULL)
	free(d_there_file);

    return d_there;
}

/* Return the full version of path, as returned by get_full_path().  On
 * error, if path doesn't reference a directory, or if the directory
 * isn't writable, return NULL. */
char *check_writable_directory(const char *path)
{
    char *full_path = get_full_path(path);

    /* If get_full_path() fails, return NULL. */
    if (full_path == NULL)
	return NULL;

    /* If we can't write to path or path isn't a directory, return
     * NULL. */
    if (access(full_path, W_OK) != 0 ||
	full_path[strlen(full_path) - 1] != '/') {
	free(full_path);
	return NULL;
    }

    /* Otherwise, return the full path. */
    return full_path;
}

/* This function calls mkstemp(($TMPDIR|P_tmpdir|/tmp/)"nano.XXXXXX").
 * On success, it returns the malloc()ed filename and corresponding FILE
 * stream, opened in "r+b" mode.  On error, it returns NULL for the
 * filename and leaves the FILE stream unchanged. */
char *safe_tempfile(FILE **f)
{
    char *full_tempdir = NULL;
    const char *tmpdir_env;
    int fd;
    mode_t original_umask = 0;

    assert(f != NULL);

    /* If $TMPDIR is set, set tempdir to it, run it through
     * get_full_path(), and save the result in full_tempdir.  Otherwise,
     * leave full_tempdir set to NULL. */
    tmpdir_env = getenv("TMPDIR");
    if (tmpdir_env != NULL)
	full_tempdir = check_writable_directory(tmpdir_env);

    /* If $TMPDIR is unset, empty, or not a writable directory, and
     * full_tempdir is NULL, try P_tmpdir instead. */
    if (full_tempdir == NULL)
	full_tempdir = check_writable_directory(P_tmpdir);

    /* if P_tmpdir is NULL, use /tmp. */
    if (full_tempdir == NULL)
	full_tempdir = mallocstrcpy(NULL, "/tmp/");

    full_tempdir = charealloc(full_tempdir, strlen(full_tempdir) + 12);
    strcat(full_tempdir, "nano.XXXXXX");

    original_umask = umask(0);
    umask(S_IRWXG | S_IRWXO);

    fd = mkstemp(full_tempdir);

    if (fd != -1)
	*f = fdopen(fd, "r+b");
    else {
	free(full_tempdir);
	full_tempdir = NULL;
    }

    umask(original_umask);

    return full_tempdir;
}

#ifndef DISABLE_OPERATINGDIR
/* Initialize full_operating_dir based on operating_dir. */
void init_operating_dir(void)
{
    assert(full_operating_dir == NULL);

    if (operating_dir == NULL)
	return;

    full_operating_dir = get_full_path(operating_dir);

<<<<<<< HEAD
    /* If get_full_path() failed or the operating directory is
     * inaccessible, unset operating_dir. */
    if (full_operating_dir == NULL || chdir(full_operating_dir) == -1) {
	free(full_operating_dir);
	full_operating_dir = NULL;
	free(operating_dir);
	operating_dir = NULL;
    }
=======
    /* If the operating directory is inaccessible, fail. */
    if (full_operating_dir == NULL || chdir(full_operating_dir) == -1)
	die(_("Invalid operating directory\n"));

    snuggly_fit(&full_operating_dir);
>>>>>>> origin/tomato-shibby-RT-AC
}

/* Check to see if we're inside the operating directory.  Return FALSE
 * if we are, or TRUE otherwise.  If allow_tabcomp is TRUE, allow
 * incomplete names that would be matches for the operating directory,
 * so that tab completion will work. */
bool check_operating_dir(const char *currpath, bool allow_tabcomp)
{
    /* full_operating_dir is global for memory cleanup.  It should have
     * already been initialized by init_operating_dir().  Also, a
     * relative operating directory path will only be handled properly
     * if this is done. */

    char *fullpath;
    bool retval = FALSE;
    const char *whereami1, *whereami2 = NULL;

    /* If no operating directory is set, don't bother doing anything. */
    if (operating_dir == NULL)
	return FALSE;

    assert(full_operating_dir != NULL);

    fullpath = get_full_path(currpath);

    /* If fullpath is NULL, it means some directory in the path doesn't
     * exist or is unreadable.  If allow_tabcomp is FALSE, then currpath
     * is what the user typed somewhere.  We don't want to report a
     * non-existent directory as being outside the operating directory,
     * so we return FALSE.  If allow_tabcomp is TRUE, then currpath
     * exists, but is not executable.  So we say it isn't in the
     * operating directory. */
    if (fullpath == NULL)
	return allow_tabcomp;

    whereami1 = strstr(fullpath, full_operating_dir);
    if (allow_tabcomp)
	whereami2 = strstr(full_operating_dir, fullpath);

    /* If both searches failed, we're outside the operating directory.
     * Otherwise, check the search results.  If the full operating
     * directory path is not at the beginning of the full current path
     * (for normal usage) and vice versa (for tab completion, if we're
     * allowing it), we're outside the operating directory. */
    if (whereami1 != fullpath && whereami2 != full_operating_dir)
	retval = TRUE;
    free(fullpath);

    /* Otherwise, we're still inside it. */
    return retval;
}
#endif

#ifndef NANO_TINY
void init_backup_dir(void)
{
    char *full_backup_dir;

    if (backup_dir == NULL)
	return;

    full_backup_dir = get_full_path(backup_dir);

    /* If get_full_path() failed or the backup directory is
     * inaccessible, unset backup_dir. */
    if (full_backup_dir == NULL ||
	full_backup_dir[strlen(full_backup_dir) - 1] != '/') {
	free(full_backup_dir);
	free(backup_dir);
	backup_dir = NULL;
    } else {
	free(backup_dir);
	backup_dir = full_backup_dir;
	snuggly_fit(&backup_dir);
    }
}
#endif

/* Read from inn, write to out.  We assume inn is opened for reading,
 * and out for writing.  We return 0 on success, -1 on read error, or -2
 * on write error. */
int copy_file(FILE *inn, FILE *out)
{
    int retval = 0;
    char buf[BUFSIZ];
    size_t charsread;

    assert(inn != NULL && out != NULL && inn != out);

    do {
	charsread = fread(buf, sizeof(char), BUFSIZ, inn);
	if (charsread == 0 && ferror(inn)) {
	    retval = -1;
	    break;
	}
	if (fwrite(buf, sizeof(char), charsread, out) < charsread) {
	    retval = -2;
	    break;
	}
    } while (charsread > 0);

    if (fclose(inn) == EOF)
	retval = -1;
    if (fclose(out) == EOF)
	retval = -2;

    return retval;
}

/* Write a file out to disk.  If f_open isn't NULL, we assume that it is
 * a stream associated with the file, and we don't try to open it
 * ourselves.  If tmp is TRUE, we set the umask to disallow anyone else
 * from accessing the file, we don't set the filename to its name, and
 * we don't print out how many lines we wrote on the statusbar.
 *
 * tmp means we are writing a temporary file in a secure fashion.  We
 * use it when spell checking or dumping the file on an error.  If
 * method is APPEND, it means we are appending instead of overwriting.
 * If method is PREPEND, it means we are prepending instead of
 * overwriting.  If nonamechange is TRUE, we don't change the current
 * filename.  nonamechange is irrelevant when appending or prepending,
 * or when writing a temporary file.
 *
 * Return TRUE on success or FALSE on error. */
bool write_file(const char *name, FILE *f_open, bool tmp,
	kind_of_writing_type method, bool nonamechange)
{
    bool retval = FALSE;
	/* Instead of returning in this function, you should always
	 * set retval and then goto cleanup_and_exit. */
    size_t lineswritten = 0;
    const filestruct *fileptr = openfile->fileage;
    int fd;
	/* The file descriptor we use. */
    mode_t original_umask = 0;
	/* Our umask, from when nano started. */
    bool realexists;
	/* The result of stat().  TRUE if the file exists, FALSE
	 * otherwise.  If name is a link that points nowhere, realexists
	 * is FALSE. */
    struct stat st;
	/* The status fields filled in by stat(). */
    bool anyexists;
	/* The result of lstat().  The same as realexists, unless name
	 * is a link. */
    struct stat lst;
	/* The status fields filled in by lstat(). */
    char *realname;
	/* name after tilde expansion. */
    FILE *f = NULL;
	/* The actual file, realname, we are writing to. */
    char *tempname = NULL;
	/* The temp file name we write to on prepend. */
    int backup_cflags;

    if (*name == '\0')
	return -1;

    if (f_open != NULL)
	f = f_open;

    if (!tmp)
	titlebar(NULL);

    realname = real_dir_from_tilde(name);

#ifndef DISABLE_OPERATINGDIR
    /* If we're writing a temporary file, we're probably going outside
     * the operating directory, so skip the operating directory test. */
    if (!tmp && check_operating_dir(realname, FALSE)) {
	statusbar(_("Can't write outside of %s"), operating_dir);
	goto cleanup_and_exit;
    }
#endif

    anyexists = (lstat(realname, &lst) != -1);

    /* If the temp file exists and isn't already open, give up. */
    if (tmp && anyexists && f_open == NULL)
	goto cleanup_and_exit;

    /* If NOFOLLOW_SYMLINKS is set, it doesn't make sense to prepend or
     * append to a symlink.  Here we warn about the contradiction. */
    if (ISSET(NOFOLLOW_SYMLINKS) && anyexists && S_ISLNK(lst.st_mode)) {
	statusbar(
		_("Cannot prepend or append to a symlink with --nofollow set"));
	goto cleanup_and_exit;
    }

    /* Save the state of the file at the end of the symlink (if there is
     * one). */
    realexists = (stat(realname, &st) != -1);

#ifndef NANO_TINY
    /* if we have not stat()d this file before (say, the user just
     * specified it interactively), stat and save the value
     * or else we will chase null pointers when we do
     * modtime checks, preserve file times, etc. during backup */
    if (openfile->current_stat == NULL && !tmp && realexists)
	stat(realname, openfile->current_stat);

    /* We backup only if the backup toggle is set, the file isn't
     * temporary, and the file already exists.  Furthermore, if we
     * aren't appending, prepending, or writing a selection, we backup
     * only if the file has not been modified by someone else since nano
     * opened it. */
<<<<<<< HEAD
    if (ISSET(BACKUP_FILE) && !tmp && realexists && ((append !=
	OVERWRITE || openfile->mark_set) || (openfile->current_stat &&
	openfile->current_stat->st_mtime == st.st_mtime))) {
=======
    if (ISSET(BACKUP_FILE) && !tmp && realexists && openfile->current_stat &&
		(method != OVERWRITE || openfile->mark_set ||
		openfile->current_stat->st_mtime == st.st_mtime)) {
>>>>>>> origin/tomato-shibby-RT-AC
	int backup_fd;
	FILE *backup_file;
	char *backupname;
	struct utimbuf filetime;
	int copy_status;

	/* Save the original file's access and modification times. */
	filetime.actime = openfile->current_stat->st_atime;
	filetime.modtime = openfile->current_stat->st_mtime;

	if (f_open == NULL) {
	    /* Open the original file to copy to the backup. */
	    f = fopen(realname, "rb");

	    if (f == NULL) {
		statusbar(_("Error reading %s: %s"), realname,
			strerror(errno));
		beep();
		/* If we can't read from the original file, go on, since
		 * only saving the original file is better than saving
		 * nothing. */
		goto skip_backup;
	    }
	}

	/* If backup_dir is set, we set backupname to
	 * backup_dir/backupname~[.number], where backupname is the
	 * canonicalized absolute pathname of realname with every '/'
	 * replaced with a '!'.  This means that /home/foo/file is
	 * backed up in backup_dir/!home!foo!file~[.number]. */
	if (backup_dir != NULL) {
	    char *backuptemp = get_full_path(realname);

	    if (backuptemp == NULL)
		/* If get_full_path() failed, we don't have a
		 * canonicalized absolute pathname, so just use the
		 * filename portion of the pathname.  We use tail() so
		 * that e.g. ../backupname will be backed up in
		 * backupdir/backupname~ instead of
		 * backupdir/../backupname~. */
		backuptemp = mallocstrcpy(NULL, tail(realname));
	    else {
		size_t i = 0;

		for (; backuptemp[i] != '\0'; i++) {
		    if (backuptemp[i] == '/')
			backuptemp[i] = '!';
		}
	    }

	    backupname = charalloc(strlen(backup_dir) +
		strlen(backuptemp) + 1);
	    sprintf(backupname, "%s%s", backup_dir, backuptemp);
	    free(backuptemp);
	    backuptemp = get_next_filename(backupname, "~");
	    if (*backuptemp == '\0') {
		statusbar(_("Error writing backup file %s: %s"), backupname,
		    _("Too many backup files?"));
		free(backuptemp);
		free(backupname);
		/* If we can't write to the backup, DONT go on, since
		   whatever caused the backup file to fail (e.g. disk
		   full may well cause the real file write to fail, which
		   means we could lose both the backup and the original! */
		goto cleanup_and_exit;
	    } else {
		free(backupname);
		backupname = backuptemp;
	    }
	} else {
	    backupname = charalloc(strlen(realname) + 2);
	    sprintf(backupname, "%s~", realname);
	}

	/* First, unlink any existing backups.  Next, open the backup
	   file with O_CREAT and O_EXCL.  If it succeeds, we
	   have a file descriptor to a new backup file. */
	if (unlink(backupname) < 0 && errno != ENOENT && !ISSET(INSECURE_BACKUP)) {
	    statusbar(_("Error writing backup file %s: %s"), backupname,
			strerror(errno));
	    free(backupname);
	    goto cleanup_and_exit;
	}

	if (ISSET(INSECURE_BACKUP))
	    backup_cflags = O_WRONLY | O_CREAT | O_APPEND;
        else
	    backup_cflags = O_WRONLY | O_CREAT | O_EXCL | O_APPEND;

	backup_fd = open(backupname, backup_cflags,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	/* Now we've got a safe file stream.  If the previous open()
	   call failed, this will return NULL. */
	backup_file = fdopen(backup_fd, "wb");

	if (backup_fd < 0 || backup_file == NULL) {
	    statusbar(_("Error writing backup file %s: %s"), backupname,
			strerror(errno));
	    free(backupname);
	    goto cleanup_and_exit;
	}

        /* We shouldn't worry about chown()ing something if we're not
	   root, since it's likely to fail! */
	if (geteuid() == NANO_ROOT_UID && fchown(backup_fd,
		openfile->current_stat->st_uid, openfile->current_stat->st_gid) == -1
                && !ISSET(INSECURE_BACKUP)) {
	    statusbar(_("Error writing backup file %s: %s"), backupname,
		strerror(errno));
	    free(backupname);
	    fclose(backup_file);
	    goto cleanup_and_exit;
	}

	if (fchmod(backup_fd, openfile->current_stat->st_mode) == -1 && !ISSET(INSECURE_BACKUP)) {
	    statusbar(_("Error writing backup file %s: %s"), backupname,
		strerror(errno));
	    free(backupname);
	    fclose(backup_file);
	    /* If we can't write to the backup, DONT go on, since
	       whatever caused the backup file to fail (e.g. disk
	       full may well cause the real file write to fail, which
	       means we could lose both the backup and the original! */
	    goto cleanup_and_exit;
	}

#ifdef DEBUG
	fprintf(stderr, "Backing up %s to %s\n", realname, backupname);
#endif

	/* Copy the file. */
	copy_status = copy_file(f, backup_file);

	if (copy_status != 0) {
	    statusbar(_("Error reading %s: %s"), realname,
			strerror(errno));
	    beep();
	    goto cleanup_and_exit;
	}

	/* And set its metadata. */
	if (utime(backupname, &filetime) == -1 && !ISSET(INSECURE_BACKUP)) {
	    statusbar(_("Error writing backup file %s: %s"), backupname,
			strerror(errno));
	    /* If we can't write to the backup, DONT go on, since
	       whatever caused the backup file to fail (e.g. disk
	       full may well cause the real file write to fail, which
	       means we could lose both the backup and the original! */
	    goto cleanup_and_exit;
	}

	free(backupname);
    }

    skip_backup:
#endif /* !NANO_TINY */

    /* If NOFOLLOW_SYMLINKS is set and the file is a link, we aren't
     * doing prepend or append.  So we delete the link first, and just
     * overwrite. */
    if (ISSET(NOFOLLOW_SYMLINKS) && anyexists && S_ISLNK(lst.st_mode) &&
	unlink(realname) == -1) {
	statusbar(_("Error writing %s: %s"), realname, strerror(errno));
	goto cleanup_and_exit;
    }

    if (f_open == NULL) {
	original_umask = umask(0);

	/* If we create a temp file, we don't let anyone else access it.
	 * We create a temp file if tmp is TRUE. */
	if (tmp)
	    umask(S_IRWXG | S_IRWXO);
	else
	    umask(original_umask);
    }

    /* If we're prepending, copy the file to a temp file. */
    if (method == PREPEND) {
	int fd_source;
	FILE *f_source = NULL;

	if (f == NULL) {
	    f = fopen(realname, "rb");

	    if (f == NULL) {
		statusbar(_("Error reading %s: %s"), realname,
			strerror(errno));
		beep();
		goto cleanup_and_exit;
	    }
	}

	tempname = safe_tempfile(&f);

	if (tempname == NULL) {
<<<<<<< HEAD
	    statusbar(_("Error writing temp file: %s"),
		strerror(errno));
=======
	    statusline(ALERT, _("Error writing temp file: %s"),
			strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
	    goto cleanup_and_exit;
	}

	if (f_open == NULL) {
	    fd_source = open(realname, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);

	    if (fd_source != -1) {
		f_source = fdopen(fd_source, "rb");
		if (f_source == NULL) {
		    statusbar(_("Error reading %s: %s"), realname,
			strerror(errno));
		    beep();
		    close(fd_source);
		    fclose(f);
		    unlink(tempname);
		    goto cleanup_and_exit;
		}
	    }
	}

<<<<<<< HEAD
	if (copy_file(f_source, f) != 0) {
	    statusbar(_("Error writing %s: %s"), tempname,
		strerror(errno));
=======
	if (f_source == NULL || copy_file(f_source, f) != 0) {
	    statusline(ALERT, _("Error writing temp file: %s"),
			strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
	    unlink(tempname);
	    goto cleanup_and_exit;
	}
    }

    if (f_open == NULL) {
	/* Now open the file in place.  Use O_EXCL if tmp is TRUE.  This
	 * is copied from joe, because wiggy says so *shrug*. */
	fd = open(realname, O_WRONLY | O_CREAT | ((method == APPEND) ?
		O_APPEND : (tmp ? O_EXCL : O_TRUNC)), S_IRUSR |
		S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	/* Set the umask back to the user's original value. */
	umask(original_umask);

	/* If we couldn't open the file, give up. */
	if (fd == -1) {
<<<<<<< HEAD
	    statusbar(_("Error writing %s: %s"), realname,
		strerror(errno));

	    /* tempname has been set only if we're prepending. */
=======
	    statusline(ALERT, _("Error writing %s: %s"), realname,
			strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
	    if (tempname != NULL)
		unlink(tempname);
	    goto cleanup_and_exit;
	}

	f = fdopen(fd, (method == APPEND) ? "ab" : "wb");

	if (f == NULL) {
<<<<<<< HEAD
	    statusbar(_("Error writing %s: %s"), realname,
		strerror(errno));
=======
	    statusline(ALERT, _("Error writing %s: %s"), realname,
			strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
	    close(fd);
	    goto cleanup_and_exit;
	}
    }

    while (fileptr != NULL) {
	size_t data_len = strlen(fileptr->data), size;

	/* Convert newlines to nulls, just before we write to disk. */
	sunder(fileptr->data);

	size = fwrite(fileptr->data, sizeof(char), data_len, f);

	/* Convert nulls to newlines.  data_len is the string's real
	 * length. */
	unsunder(fileptr->data, data_len);

	if (size < data_len) {
<<<<<<< HEAD
	    statusbar(_("Error writing %s: %s"), realname,
		strerror(errno));
=======
	    statusline(ALERT, _("Error writing %s: %s"), realname,
			strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
	    fclose(f);
	    goto cleanup_and_exit;
	}

	/* If we're on the last line of the file, don't write a newline
	 * character after it.  If the last line of the file is blank,
	 * this means that zero bytes are written, in which case we
	 * don't count the last line in the total lines written. */
	if (fileptr == openfile->filebot) {
	    if (fileptr->data[0] == '\0')
		lineswritten--;
	} else {
#ifndef NANO_TINY
	    if (openfile->fmt == DOS_FILE || openfile->fmt ==
		MAC_FILE) {
		if (putc('\r', f) == EOF) {
<<<<<<< HEAD
		    statusbar(_("Error writing %s: %s"), realname,
			strerror(errno));
=======
		    statusline(ALERT, _("Error writing %s: %s"), realname,
				strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
		    fclose(f);
		    goto cleanup_and_exit;
		}
	    }

	    if (openfile->fmt != MAC_FILE) {
#endif
		if (putc('\n', f) == EOF) {
<<<<<<< HEAD
		    statusbar(_("Error writing %s: %s"), realname,
			strerror(errno));
=======
		    statusline(ALERT, _("Error writing %s: %s"), realname,
				strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
		    fclose(f);
		    goto cleanup_and_exit;
		}
#ifndef NANO_TINY
	    }
#endif
	}

	fileptr = fileptr->next;
	lineswritten++;
    }

    /* If we're prepending, open the temp file, and append it to f. */
    if (method == PREPEND) {
	int fd_source;
	FILE *f_source = NULL;

	fd_source = open(tempname, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR);

	if (fd_source != -1) {
	    f_source = fdopen(fd_source, "rb");
	    if (f_source == NULL)
		close(fd_source);
	}

	if (f_source == NULL) {
	    statusbar(_("Error reading %s: %s"), tempname,
		strerror(errno));
	    beep();
	    fclose(f);
	    goto cleanup_and_exit;
	}

<<<<<<< HEAD
	if (copy_file(f_source, f) == -1 || unlink(tempname) == -1) {
	    statusbar(_("Error writing %s: %s"), realname,
		strerror(errno));
=======
	if (copy_file(f_source, f) == -1) {
	    statusline(ALERT, _("Error writing %s: %s"), realname,
			strerror(errno));
>>>>>>> origin/tomato-shibby-RT-AC
	    goto cleanup_and_exit;
	}
    } else if (fclose(f) != 0) {
<<<<<<< HEAD
	    statusbar(_("Error writing %s: %s"), realname,
		strerror(errno));
	    goto cleanup_and_exit;
    }

    if (!tmp && append == OVERWRITE) {
	if (!nonamechange) {
	    openfile->filename = mallocstrcpy(openfile->filename,
		realname);
#ifdef ENABLE_COLOR
	    /* We might have changed the filename, so update the colors
	     * to account for it, and then make sure we're using
	     * them. */
	    color_update();
	    color_init();

	    /* If color syntaxes are available and turned on, we need to
	     * call edit_refresh(). */
	    if (openfile->colorstrings != NULL &&
		!ISSET(NO_COLOR_SYNTAX))
		edit_refresh();
=======
	statusline(ALERT, _("Error writing %s: %s"), realname,
			strerror(errno));
	goto cleanup_and_exit;
    }

    if (method == OVERWRITE && !tmp) {
	/* If we must set the filename, and it changed, adjust things. */
	if (!nonamechange && strcmp(openfile->filename, realname) != 0) {
#ifndef DISABLE_COLOR
	    char *oldname = openfile->syntax ? openfile->syntax->name : "";
	    filestruct *line = openfile->fileage;
#endif
	    openfile->filename = mallocstrcpy(openfile->filename, realname);

#ifndef DISABLE_COLOR
	    /* See if the applicable syntax has changed. */
	    color_update();
	    color_init();

	    char *newname = openfile->syntax ? openfile->syntax->name : "";

	    /* If the syntax changed, discard and recompute the multidata. */
	    if (strcmp(oldname, newname) != 0) {
		for (; line != NULL; line = line->next) {
		    free(line->multidata);
		    line->multidata = NULL;
		}
		precalc_multicolorinfo();
		refresh_needed = TRUE;
	    }
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	}

#ifndef NANO_TINY
	/* Update current_stat to reference the file as it is now. */
	if (openfile->current_stat == NULL)
	    openfile->current_stat =
		(struct stat *)nmalloc(sizeof(struct stat));
	stat(realname, openfile->current_stat);
#endif

	statusbar(P_("Wrote %lu line", "Wrote %lu lines",
		(unsigned long)lineswritten),
		(unsigned long)lineswritten);
	openfile->modified = FALSE;
	titlebar(NULL);
    }

    retval = TRUE;

  cleanup_and_exit:
    free(realname);
    if (tempname != NULL)
	free(tempname);

    return retval;
}

#ifndef NANO_TINY
/* Write a marked selection from a file out to disk.  Return TRUE on
 * success or FALSE on error. */
bool write_marked_file(const char *name, FILE *f_open, bool tmp,
	kind_of_writing_type method)
{
    bool retval;
    bool old_modified = openfile->modified;
	/* write_file() unsets the modified flag. */
    bool added_magicline = FALSE;
	/* Whether we added a magicline after filebot. */
    filestruct *top, *bot;
    size_t top_x, bot_x;

<<<<<<< HEAD
    assert(openfile->mark_set);

    /* Partition the filestruct so that it contains only the marked
     * text. */
=======
    /* Partition the filestruct so that it contains only the marked text. */
>>>>>>> origin/tomato-shibby-RT-AC
    mark_order((const filestruct **)&top, &top_x,
	(const filestruct **)&bot, &bot_x, NULL);
    filepart = partition_filestruct(top, top_x, bot, bot_x);

    /* Handle the magicline if the NO_NEWLINES flag isn't set.  If the
     * line at filebot is blank, treat it as the magicline and hence the
     * end of the file.  Otherwise, add a magicline and treat it as the
     * end of the file. */
    if (!ISSET(NO_NEWLINES) &&
	(added_magicline = (openfile->filebot->data[0] != '\0')))
	new_magicline();

    retval = write_file(name, f_open, tmp, method, TRUE);

    /* If the NO_NEWLINES flag isn't set, and we added a magicline,
     * remove it now. */
    if (!ISSET(NO_NEWLINES) && added_magicline)
	remove_magicline();

    /* Unpartition the filestruct so that it contains all the text
     * again. */
    unpartition_filestruct(&filepart);

    if (old_modified)
	set_modified();

    return retval;
}
#endif /* !NANO_TINY */

/* Write the current file to disk.  If the mark is on, write the current
 * marked selection to disk.  If exiting is TRUE, write the file to disk
 * regardless of whether the mark is on, and without prompting if the
 * TEMP_FILE flag is set.  Return TRUE on success or FALSE on error. */
bool do_writeout(bool exiting)
{
    int i;
<<<<<<< HEAD
    append_type append = OVERWRITE;
    char *ans;
	/* The last answer the user typed at the statusbar prompt. */
#ifdef NANO_EXTRA
=======
    bool result = FALSE;
    kind_of_writing_type method = OVERWRITE;
    char *given;
	/* The filename we offer, or what the user typed so far. */
    bool maychange = (openfile->filename[0] == '\0');
	/* Whether it's okay to save the file under a different name. */
#ifndef DISABLE_EXTRA
>>>>>>> origin/tomato-shibby-RT-AC
    static bool did_credits = FALSE;
#endif
    bool retval = FALSE, meta_key = FALSE, func_key = FALSE;
    const sc *s;

    currmenu = MWRITEFILE;

<<<<<<< HEAD
    if (exiting && openfile->filename[0] != '\0' && ISSET(TEMP_FILE)) {
	retval = write_file(openfile->filename, NULL, FALSE, OVERWRITE,
		FALSE);

	/* Write succeeded. */
	if (retval)
	    return retval;
=======
    /* Display newlines in filenames as ^J. */
    as_an_at = FALSE;

    if (exiting && ISSET(TEMP_FILE) && openfile->filename[0] != '\0') {
	if (write_file(openfile->filename, NULL, FALSE, OVERWRITE, FALSE))
	    return 1;
	/* If writing the file failed, go on to prompt for a new name. */
>>>>>>> origin/tomato-shibby-RT-AC
    }

    ans = mallocstrcpy(NULL,
#ifndef NANO_TINY
	(!exiting && openfile->mark_set) ? "" :
#endif
	openfile->filename);

    while (TRUE) {
	const char *msg;
#ifndef NANO_TINY
	const char *formatstr, *backupstr;

	formatstr = (openfile->fmt == DOS_FILE) ?
		_(" [DOS Format]") : (openfile->fmt == MAC_FILE) ?
		_(" [Mac Format]") : "";

	backupstr = ISSET(BACKUP_FILE) ? _(" [Backup]") : "";

	/* If we're using restricted mode, don't display the "Write
	 * Selection to File" prompt.  This function is disabled, since
	 * it allows reading from or writing to files not specified on
	 * the command line. */
<<<<<<< HEAD
	if (!ISSET(RESTRICTED) && !exiting && openfile->mark_set)
	    msg = (append == PREPEND) ?
		_("Prepend Selection to File") : (append == APPEND) ?
		_("Append Selection to File") :
		_("Write Selection to File");
	else
#endif /* !NANO_TINY */
	    msg = (append == PREPEND) ? _("File Name to Prepend to") :
		(append == APPEND) ? _("File Name to Append to") :
		_("File Name to Write");

	/* If we're using restricted mode, the filename isn't blank,
	 * and we're at the "Write File" prompt, disable tab
	 * completion. */
	i = do_prompt(!ISSET(RESTRICTED) ||
		openfile->filename[0] == '\0',
#ifndef DISABLE_TABCOMP
		TRUE,
#endif
		MWRITEFILE, ans,
		&meta_key, &func_key,
#ifndef NANO_TINY
=======
	if (openfile->mark_set && !exiting && !ISSET(RESTRICTED))
	    /* TRANSLATORS: The next six strings are prompts. */
	    msg = (method == PREPEND) ? _("Prepend Selection to File") :
			(method == APPEND) ? _("Append Selection to File") :
			_("Write Selection to File");
	else
#endif /* !NANO_TINY */
	    msg = (method == PREPEND) ? _("File Name to Prepend to") :
			(method == APPEND) ? _("File Name to Append to") :
			_("File Name to Write");

	present_path = mallocstrcpy(present_path, "./");

	/* If we're using restricted mode, and the filename isn't blank,
	 * disable tab completion. */
	i = do_prompt(!ISSET(RESTRICTED) || openfile->filename[0] == '\0',
		TRUE, MWRITEFILE, given,
#ifndef DISABLE_HISTORIES
>>>>>>> origin/tomato-shibby-RT-AC
		NULL,
#endif
		edit_refresh, "%s%s%s", msg,
#ifndef NANO_TINY
		formatstr, backupstr
#else
		"", ""
#endif
		);

	if (i < 0) {
	    statusbar(_("Cancelled"));
	    retval = FALSE;
	    break;
	} else {
	    ans = mallocstrcpy(ans, answer);
            s = get_shortcut(currmenu, &i, &meta_key, &func_key);

#ifndef DISABLE_BROWSER
	    if (s && s->scfunc == TO_FILES_MSG) {
		char *tmp = do_browse_from(answer);

		if (tmp == NULL)
		    continue;

		/* We have a file now.  Indicate this. */
		free(answer);
		answer = tmp;
	    } else
#endif /* !DISABLE_BROWSER */
#ifndef NANO_TINY
	    if (s && s->scfunc ==  DOS_FORMAT_MSG) {
		openfile->fmt = (openfile->fmt == DOS_FILE) ? NIX_FILE :
			DOS_FILE;
		continue;
	    } else if (s && s->scfunc ==  MAC_FORMAT_MSG) {
		openfile->fmt = (openfile->fmt == MAC_FILE) ? NIX_FILE :
			MAC_FILE;
		continue;
	    } else if (s && s->scfunc ==  BACKUP_FILE_MSG) {
		TOGGLE(BACKUP_FILE);
		continue;
	    } else
#endif /* !NANO_TINY */
<<<<<<< HEAD
	    if (s && s->scfunc ==  PREPEND_MSG) {
		append = (append == PREPEND) ? OVERWRITE : PREPEND;
		continue;
	    } else if (s && s->scfunc ==  APPEND_MSG) {
		append = (append == APPEND) ? OVERWRITE : APPEND;
=======
	    if (func == prepend_void) {
		method = (method == PREPEND) ? OVERWRITE : PREPEND;
		continue;
	    } else if (func == append_void) {
		method = (method == APPEND) ? OVERWRITE : APPEND;
>>>>>>> origin/tomato-shibby-RT-AC
		continue;
	    } else if (s && s->scfunc == DO_HELP_VOID) {
		continue;
	    }

#ifdef DEBUG
	    fprintf(stderr, "filename is %s\n", answer);
#endif

#ifdef NANO_EXTRA
	    /* If the current file has been modified, we've pressed
	     * Ctrl-X at the edit window to exit, we've pressed "y" at
	     * the "Save modified buffer" prompt to save, we've entered
	     * "zzy" as the filename to save under (hence "xyzzy"), and
	     * this is the first time we've done this, show an Easter
	     * egg.  Display the credits. */
	    if (!did_credits && exiting && !ISSET(TEMP_FILE) &&
		strcasecmp(answer, "zzy") == 0) {
		do_credits();
		did_credits = TRUE;
		retval = FALSE;
		break;
	    }
#endif

	    if (method == OVERWRITE) {
		bool name_exists, do_warning;
		char *full_answer, *full_filename;
		struct stat st;

		full_answer = get_full_path(answer);
		full_filename = get_full_path(openfile->filename);
		name_exists = (stat((full_answer == NULL) ? answer :
			full_answer, &st) != -1);
		if (openfile->filename[0] == '\0')
		    do_warning = name_exists;
		else
		    do_warning = (strcmp((full_answer == NULL) ?
			answer : full_answer, (full_filename == NULL) ?
			openfile->filename : full_filename) != 0);

<<<<<<< HEAD
		/* Convert nulls to newlines.  answer_len is the
		 * string's real length. */
		unsunder(answer, answer_len);

		if (full_filename != NULL)
		    free(full_filename);
		if (full_answer != NULL)
		    free(full_answer);
=======
		free(full_filename);
		free(full_answer);
>>>>>>> origin/tomato-shibby-RT-AC

		if (do_warning) {
		    /* When in restricted mode, we aren't allowed to overwrite
		     * an existing file with the current buffer, nor to change
		     * the name of the current file if it already has one. */
		    if (ISSET(RESTRICTED)) {
			/* TRANSLATORS: Restricted mode forbids overwriting. */
			warn_and_shortly_pause(_("File exists -- "
					"cannot overwrite"));
			continue;
		    }

		    if (name_exists) {
			i = do_yesno_prompt(FALSE,
				_("File exists, OVERWRITE ? "));
			if (i == 0 || i == -1)
			    continue;
		    } else
#ifndef NANO_TINY
		    if (exiting || !openfile->mark_set)
#endif
		    {
			i = do_yesno_prompt(FALSE,
				_("Save file under DIFFERENT NAME ? "));
			if (i == 0 || i == -1)
			    continue;
		    }
		}
#ifndef NANO_TINY
		/* Complain if the file exists, the name hasn't changed, and the
		    stat information we had before does not match what we have now */
		else if (name_exists && openfile->current_stat && (openfile->current_stat->st_mtime < st.st_mtime ||
                    openfile->current_stat->st_dev != st.st_dev || openfile->current_stat->st_ino != st.st_ino)) {
		    i = do_yesno_prompt(FALSE,
			_("File was modified since you opened it, continue saving ? "));
		    if (i == 0 || i == -1)
			continue;
		}
#endif

	    }

<<<<<<< HEAD
	    /* Convert newlines to nulls, just before we save the
	     * file. */
	    sunder(answer);
	    align(&answer);

=======
>>>>>>> origin/tomato-shibby-RT-AC
	    /* Here's where we allow the selected text to be written to
	     * a separate file.  If we're using restricted mode, this
	     * function is disabled, since it allows reading from or
	     * writing to files not specified on the command line. */
	    retval =
#ifndef NANO_TINY
<<<<<<< HEAD
		(!ISSET(RESTRICTED) && !exiting && openfile->mark_set) ?
		write_marked_file(answer, NULL, FALSE, append) :
#endif
		write_file(answer, NULL, FALSE, append, FALSE);
=======
	    if (openfile->mark_set && !exiting && !ISSET(RESTRICTED))
		result = write_marked_file(answer, NULL, FALSE, method);
	    else
#endif
		result = write_file(answer, NULL, FALSE, method, FALSE);
>>>>>>> origin/tomato-shibby-RT-AC

	    break;
	}
    }

    free(ans);

    return retval;
}

/* Write the current file to disk.  If the mark is on, write the current
 * marked selection to disk. */
void do_writeout_void(void)
{
    do_writeout(FALSE);
    display_main_list();
}

/* Return a malloc()ed string containing the actual directory, used to
 * convert ~user/ and ~/ notation. */
char *real_dir_from_tilde(const char *buf)
{
    char *retval;

    if (*buf == '~') {
	size_t i = 1;
	char *tilde_dir;

	/* Figure out how much of the string we need to compare. */
	for (; buf[i] != '/' && buf[i] != '\0'; i++)
	    ;

	/* Get the home directory. */
	if (i == 1) {
	    get_homedir();
	    tilde_dir = mallocstrcpy(NULL, homedir);
	} else {
#ifdef HAVE_PWD_H
	    const struct passwd *userdata;

	    tilde_dir = mallocstrncpy(NULL, buf, i + 1);
	    tilde_dir[i] = '\0';

	    do {
		userdata = getpwent();
	    } while (userdata != NULL && strcmp(userdata->pw_name,
		tilde_dir + 1) != 0);
	    endpwent();
	    if (userdata != NULL)
		tilde_dir = mallocstrcpy(tilde_dir, userdata->pw_dir);
#else
	    tilde_dir = strdup("");
#endif
	}

	retval = charalloc(strlen(tilde_dir) + strlen(buf + i) + 1);
	sprintf(retval, "%s%s", tilde_dir, buf + i);

	free(tilde_dir);
    } else
	retval = mallocstrcpy(NULL, buf);

    return retval;
}

#if !defined(DISABLE_TABCOMP) || !defined(DISABLE_BROWSER)
/* Our sort routine for file listings.  Sort alphabetically and
 * case-insensitively, and sort directories before filenames. */
int diralphasort(const void *va, const void *vb)
{
    struct stat fileinfo;
    const char *a = *(const char *const *)va;
    const char *b = *(const char *const *)vb;
    bool aisdir = stat(a, &fileinfo) != -1 && S_ISDIR(fileinfo.st_mode);
    bool bisdir = stat(b, &fileinfo) != -1 && S_ISDIR(fileinfo.st_mode);

    if (aisdir && !bisdir)
	return -1;
    if (!aisdir && bisdir)
	return 1;

    /* Standard function brain damage: We should be sorting
     * alphabetically and case-insensitively according to the current
     * locale, but there's no standard strcasecoll() function, so we
     * have to use multibyte strcasecmp() instead, */
    return mbstrcasecmp(a, b);
}

/* Free the memory allocated for array, which should contain len
 * elements. */
void free_chararray(char **array, size_t len)
{
    assert(array != NULL);

    for (; len > 0; len--)
	free(array[len - 1]);
    free(array);
}
#endif

#ifndef DISABLE_TABCOMP
/* Is the given path a directory? */
bool is_dir(const char *buf)
{
    char *dirptr;
    struct stat fileinfo;
    bool retval;

    dirptr = real_dir_from_tilde(buf);

    retval = (stat(dirptr, &fileinfo) != -1 &&
	S_ISDIR(fileinfo.st_mode));

    free(dirptr);

    return retval;
}

/* These functions, username_tab_completion(), cwd_tab_completion()
 * (originally exe_n_cwd_tab_completion()), and input_tab(), were
 * adapted from busybox 0.46 (cmdedit.c).  Here is the notice from that
 * file, with the copyright years updated:
 *
 * Termios command line History and Editting, originally
 * intended for NetBSD sh (ash)
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007
 *      Main code:            Adam Rogoyski <rogoyski@cs.utexas.edu>
 *      Etc:                  Dave Cinege <dcinege@psychosis.com>
 *  Majorly adjusted/re-written for busybox:
 *                            Erik Andersen <andersee@debian.org>
 *
 * You may use this code as you wish, so long as the original author(s)
 * are attributed in any redistributions of the source code.
 * This code is 'as is' with no warranty.
 * This code may safely be consumed by a BSD or GPL license. */

/* We consider the first buf_len characters of buf for ~username tab
 * completion. */
char **username_tab_completion(const char *buf, size_t *num_matches,
	size_t buf_len)
{
    char **matches = NULL;

    assert(buf != NULL && num_matches != NULL && buf_len > 0);

    *num_matches = 0;

#ifdef HAVE_PWD_H
    const struct passwd *userdata;

    while ((userdata = getpwent()) != NULL) {
	if (strncmp(userdata->pw_name, buf + 1, buf_len - 1) == 0) {
	    /* Cool, found a match.  Add it to the list.  This makes a
	     * lot more sense to me (Chris) this way... */

#ifndef DISABLE_OPERATINGDIR
	    /* ...unless the match exists outside the operating
	     * directory, in which case just go to the next match. */
	    if (check_operating_dir(userdata->pw_dir, TRUE))
		continue;
#endif

	    matches = (char **)nrealloc(matches, (*num_matches + 1) *
		sizeof(char *));
	    matches[*num_matches] =
		charalloc(strlen(userdata->pw_name) + 2);
	    sprintf(matches[*num_matches], "~%s", userdata->pw_name);
	    ++(*num_matches);
	}
    }
    endpwent();
#endif

    return matches;
}

/* We consider the first buf_len characters of buf for filename tab
 * completion. */
char **cwd_tab_completion(const char *buf, bool allow_files, size_t
	*num_matches, size_t buf_len)
{
    char *dirname = mallocstrcpy(NULL, buf), *filename;
#ifndef DISABLE_OPERATINGDIR
    size_t dirnamelen;
#endif
    size_t filenamelen;
    char **matches = NULL;
    DIR *dir;
    const struct dirent *nextdir;

    assert(dirname != NULL && num_matches != NULL);

    *num_matches = 0;
    dirname[buf_len] = '\0';

    /* Okie, if there's a / in the buffer, strip out the directory
     * part. */
    filename = strrchr(dirname, '/');
    if (filename != NULL) {
	char *tmpdirname = filename + 1;

	filename = mallocstrcpy(NULL, tmpdirname);
	*tmpdirname = '\0';
	tmpdirname = dirname;
	dirname = real_dir_from_tilde(dirname);
	free(tmpdirname);
    } else {
	filename = dirname;
	dirname = mallocstrcpy(NULL, "./");
    }

    assert(dirname[strlen(dirname) - 1] == '/');

    dir = opendir(dirname);

    if (dir == NULL) {
	/* Don't print an error, just shut up and return. */
	beep();
	free(filename);
	free(dirname);
	return NULL;
    }

#ifndef DISABLE_OPERATINGDIR
    dirnamelen = strlen(dirname);
#endif
    filenamelen = strlen(filename);

    while ((nextdir = readdir(dir)) != NULL) {
	bool skip_match = FALSE;

#ifdef DEBUG
	fprintf(stderr, "Comparing \'%s\'\n", nextdir->d_name);
#endif
	/* See if this matches. */
	if (strncmp(nextdir->d_name, filename, filenamelen) == 0 &&
		(*filename == '.' || (strcmp(nextdir->d_name, ".") !=
		0 && strcmp(nextdir->d_name, "..") != 0))) {
	    /* Cool, found a match.  Add it to the list.  This makes a
	     * lot more sense to me (Chris) this way... */

	    char *tmp = charalloc(strlen(dirname) +
		strlen(nextdir->d_name) + 1);
	    sprintf(tmp, "%s%s", dirname, nextdir->d_name);

#ifndef DISABLE_OPERATINGDIR
	    /* ...unless the match exists outside the operating
	     * directory, in which case just go to the next match. */
	    if (check_operating_dir(tmp, TRUE))
		skip_match = TRUE;
#endif

	    /* ...or unless the match isn't a directory and allow_files
	     * isn't set, in which case just go to the next match. */
	    if (!allow_files && !is_dir(tmp))
		skip_match = TRUE;

	    free(tmp);

	    if (skip_match)
		continue;

	    matches = (char **)nrealloc(matches, (*num_matches + 1) *
		sizeof(char *));
	    matches[*num_matches] = mallocstrcpy(NULL, nextdir->d_name);
	    ++(*num_matches);
	}
    }

    closedir(dir);
    free(dirname);
    free(filename);

    return matches;
}

/* Do tab completion.  place refers to how much the statusbar cursor
 * position should be advanced.  refresh_func is the function we will
 * call to refresh the edit window. */
char *input_tab(char *buf, bool allow_files, size_t *place, bool
	*lastwastab, void (*refresh_func)(void), bool *list)
{
    size_t num_matches = 0, buf_len;
    char **matches = NULL;

    assert(buf != NULL && place != NULL && *place <= strlen(buf) && lastwastab != NULL && refresh_func != NULL && list != NULL);

    *list = FALSE;

    /* If the word starts with `~' and there is no slash in the word,
     * then try completing this word as a username. */
    if (*place > 0 && *buf == '~') {
	const char *bob = strchr(buf, '/');

	if (bob == NULL || bob >= buf + *place)
	    matches = username_tab_completion(buf, &num_matches,
		*place);
    }

    /* Match against files relative to the current working directory. */
    if (matches == NULL)
	matches = cwd_tab_completion(buf, allow_files, &num_matches,
		*place);

    buf_len = strlen(buf);

    if (num_matches == 0 || *place != buf_len)
	beep();
    else {
	size_t match, common_len = 0;
	char *mzero;
	const char *lastslash = revstrstr(buf, "/", buf + *place);
<<<<<<< HEAD
	size_t lastslash_len = (lastslash == NULL) ? 0 :
		lastslash - buf + 1;
	char *match1_mb = charalloc(mb_cur_max() + 1);
	char *match2_mb = charalloc(mb_cur_max() + 1);
	int match1_mb_len, match2_mb_len;
=======
	size_t lastslash_len = (lastslash == NULL) ? 0 : lastslash - buf + 1;
	char char1[mb_cur_max()], char2[mb_cur_max()];
	int len1, len2;
>>>>>>> origin/tomato-shibby-RT-AC

	while (TRUE) {
<<<<<<< HEAD
	    for (match = 1; match < num_matches; match++) {
		/* Get the number of single-byte characters that all the
		 * matches have in common. */
		match1_mb_len = parse_mbchar(matches[0] + common_len,
			match1_mb, NULL);
		match2_mb_len = parse_mbchar(matches[match] +
			common_len, match2_mb, NULL);
		match1_mb[match1_mb_len] = '\0';
		match2_mb[match2_mb_len] = '\0';
		if (strcmp(match1_mb, match2_mb) != 0)
=======
	    len1 = parse_mbchar(matches[0] + common_len, char1, NULL);

	    for (match = 1; match < num_matches; match++) {
		len2 = parse_mbchar(matches[match] + common_len, char2, NULL);

		if (len1 != len2 || strncmp(char1, char2, len2) != 0)
>>>>>>> origin/tomato-shibby-RT-AC
		    break;
	    }

	    if (match < num_matches || matches[0][common_len] == '\0')
		break;

<<<<<<< HEAD
	    common_len += parse_mbchar(buf + common_len, NULL, NULL);
	}

	free(match1_mb);
	free(match2_mb);

=======
	    common_len += len1;
	}

>>>>>>> origin/tomato-shibby-RT-AC
	mzero = charalloc(lastslash_len + common_len + 1);

	strncpy(mzero, buf, lastslash_len);
	strncpy(mzero + lastslash_len, matches[0], common_len);

	common_len += lastslash_len;
	mzero[common_len] = '\0';

	assert(common_len >= *place);

	if (num_matches == 1 && is_dir(mzero)) {
	    mzero[common_len++] = '/';

	    assert(common_len > *place);
	}

	if (num_matches > 1 && (common_len != *place || !*lastwastab))
	    beep();

	/* If there is more of a match to display on the statusbar, show
	 * it.  We reset lastwastab to FALSE: it requires pressing Tab
	 * twice in succession with no statusbar changes to see a match
	 * list. */
	if (common_len != *place) {
	    *lastwastab = FALSE;
	    buf = charealloc(buf, common_len + buf_len - *place + 1);
	    charmove(buf + common_len, buf + *place, buf_len -
		*place + 1);
	    strncpy(buf, mzero, common_len);
	    *place = common_len;
	} else if (!*lastwastab || num_matches < 2)
	    *lastwastab = TRUE;
	else {
	    int longest_name = 0, ncols, editline = 0;

	    /* Now we show a list of the available choices. */
	    assert(num_matches > 1);

	    /* Sort the list. */
	    qsort(matches, num_matches, sizeof(char *), diralphasort);

	    for (match = 0; match < num_matches; match++) {
		common_len = strnlenpt(matches[match], COLS - 1);

		if (common_len > COLS - 1) {
		    longest_name = COLS - 1;
		    break;
		}

		if (common_len > longest_name)
		    longest_name = common_len;
	    }

	    assert(longest_name <= COLS - 1);

	    /* Each column will be (longest_name + 2) columns wide, i.e.
	     * two spaces between columns, except that there will be
	     * only one space after the last column. */
	    ncols = (COLS + 1) / (longest_name + 2);

	    /* Blank the edit window, and print the matches out
	     * there. */
	    blank_edit();
	    wmove(edit, 0, 0);

	    /* Disable el cursor. */
	    curs_set(0);

	    for (match = 0; match < num_matches; match++) {
		char *disp;

		wmove(edit, editline, (longest_name + 2) *
			(match % ncols));

		if (match % ncols == 0 &&
			editline == editwinrows - 1 &&
			num_matches - match > ncols) {
		    waddstr(edit, _("(more)"));
		    break;
		}

		disp = display_string(matches[match], 0, longest_name,
			FALSE);
		waddstr(edit, disp);
		free(disp);

		if ((match + 1) % ncols == 0)
		    editline++;
	    }

	    wnoutrefresh(edit);
	    *list = TRUE;
	}

	free(mzero);
    }

    free_chararray(matches, num_matches);

    /* Only refresh the edit window if we don't have a list of filename
     * matches on it. */
    if (!*list)
	refresh_func();

    /* Enable el cursor. */
    curs_set(1);

    return buf;
}
#endif /* !DISABLE_TABCOMP */

/* Only print the last part of a path.  Isn't there a shell command for
 * this? */
const char *tail(const char *foo)
{
    const char *tmp = strrchr(foo, '/');

    if (tmp == NULL)
	tmp = foo;
    else
	tmp++;

    return tmp;
}

#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
/* Return $HOME/.nano_history, or NULL if we can't find the home
 * directory.  The string is dynamically allocated, and should be
 * freed. */
char *histfilename(void)
{
<<<<<<< HEAD
    char *nanohist = NULL;
=======
    return construct_filename("/.nano/search_history");
}

/* Construct the legacy history filename. */
/* (To be removed in 2018.) */
char *legacyhistfilename(void)
{
    return construct_filename("/.nano_history");
}

char *poshistfilename(void)
{
    return construct_filename("/.nano/filepos_history");
}
>>>>>>> origin/tomato-shibby-RT-AC

    if (homedir != NULL) {
	size_t homelen = strlen(homedir);

	nanohist = charalloc(homelen + 15);
	strcpy(nanohist, homedir);
	strcpy(nanohist + homelen, "/.nano_history");
    }

    return nanohist;
}

/* Load histories from ~/.nano_history. */
void load_history(void)
{
    char *nanohist = histfilename();
<<<<<<< HEAD
=======
    char *legacyhist = legacyhistfilename();
    struct stat hstat;

    /* If there is an old history file, migrate it. */
    /* (To be removed in 2018.) */
    if (stat(legacyhist, &hstat) != -1 && stat(nanohist, &hstat) == -1) {
	if (rename(legacyhist, nanohist) == -1)
	    history_error(N_("Detected a legacy nano history file (%s) which I tried to move\n"
			     "to the preferred location (%s) but encountered an error: %s"),
				legacyhist, nanohist, strerror(errno));
	else
	    history_error(N_("Detected a legacy nano history file (%s) which I moved\n"
			     "to the preferred location (%s)\n(see the nano FAQ about this change)"),
				legacyhist, nanohist);
    }
>>>>>>> origin/tomato-shibby-RT-AC

    /* Assume do_rcfile() has reported a missing home directory. */
    if (nanohist != NULL) {
	FILE *hist = fopen(nanohist, "rb");

	if (hist == NULL) {
	    if (errno != ENOENT) {
		/* Don't save history when we quit. */
		UNSET(HISTORYLOG);
		rcfile_error(N_("Error reading %s: %s"), nanohist,
			strerror(errno));
		fprintf(stderr,
			_("\nPress Enter to continue starting nano.\n"));
		while (getchar() != '\n')
		    ;
	    }
	} else {
	    /* Load a history list (first the search history, then the
	     * replace history) from the oldest entry to the newest.
	     * Assume the last history entry is a blank line. */
	    filestruct **history = &search_history;
	    char *line = NULL;
	    size_t buf_len = 0;
	    ssize_t read;

	    while ((read = getline(&line, &buf_len, hist)) > 0) {
		line[--read] = '\0';
		if (read > 0) {
		    /* Encode any embedded NUL as 0x0A. */
		    unsunder(line, read);
		    update_history(history, line);
		} else
		    history = &replace_history;
	    }

	    fclose(hist);
	    free(line);
	}
	free(nanohist);
    }
}

/* Write the lines of a history list, starting with the line at head, to
 * the open file at hist.  Return TRUE if the write succeeded, and FALSE
 * otherwise. */
bool writehist(FILE *hist, const filestruct *head)
{
    const filestruct *item;

    /* Write a history list, from the oldest item to the newest. */
    for (item = head; item != NULL; item = item->next) {
	size_t length = strlen(item->data);

	/* Decode 0x0A bytes as embedded NULs. */
	sunder(item->data);

	if (fwrite(item->data, sizeof(char), length, hist) < length)
	    return FALSE;
	if (putc('\n', hist) == EOF)
	    return FALSE;
    }

    return TRUE;
}

/* Save histories to ~/.nano_history. */
void save_history(void)
{
    char *nanohist;

    /* Don't save unchanged or empty histories. */
    if (!history_has_changed() || (searchbot->lineno == 1 &&
	replacebot->lineno == 1))
	return;

    nanohist = histfilename();

    if (nanohist != NULL) {
	FILE *hist = fopen(nanohist, "wb");

	if (hist == NULL)
	    rcfile_error(N_("Error writing %s: %s"), nanohist,
		strerror(errno));
	else {
	    /* Make sure no one else can read from or write to the
	     * history file. */
	    chmod(nanohist, S_IRUSR | S_IWUSR);

	    if (!writehist(hist, searchage) || !writehist(hist,
		replaceage))
		rcfile_error(N_("Error writing %s: %s"), nanohist,
			strerror(errno));

	    fclose(hist);
	}

	free(nanohist);
    }
}
<<<<<<< HEAD
#endif /* !NANO_TINY && ENABLE_NANORC */
=======

/* Save the recorded last file positions to ~/.nano/filepos_history. */
void save_poshistory(void)
{
    char *poshist = poshistfilename();
    poshiststruct *posptr;
    FILE *hist;

    if (poshist == NULL)
	return;

    hist = fopen(poshist, "wb");

    if (hist == NULL)
	fprintf(stderr, _("Error writing %s: %s\n"), poshist, strerror(errno));
    else {
	/* Don't allow others to read or write the history file. */
	chmod(poshist, S_IRUSR | S_IWUSR);

	for (posptr = position_history; posptr != NULL; posptr = posptr->next) {
	    char *path_and_place;
	    size_t length;

	    /* Assume 20 decimal positions each for line and column number,
	     * plus two spaces, plus the line feed, plus the null byte. */
	    path_and_place = charalloc(strlen(posptr->filename) + 44);
	    sprintf(path_and_place, "%s %ld %ld\n", posptr->filename,
			(long)posptr->lineno, (long)posptr->xno);
	    length = strlen(path_and_place);

	    /* Encode newlines in filenames as nulls. */
	    sunder(path_and_place);
	    /* Restore the terminating newline. */
	    path_and_place[length - 1] = '\n';

	    if (fwrite(path_and_place, sizeof(char), length, hist) < length)
		fprintf(stderr, _("Error writing %s: %s\n"),
					poshist, strerror(errno));
	    free(path_and_place);
	}
	fclose(hist);
    }
    free(poshist);
}

/* Update the recorded last file positions, given a filename, a line
 * and a column.  If no entry is found, add a new one at the end. */
void update_poshistory(char *filename, ssize_t lineno, ssize_t xpos)
{
    poshiststruct *posptr, *theone, *posprev = NULL;
    char *fullpath = get_full_path(filename);

    if (fullpath == NULL || fullpath[strlen(fullpath) - 1] == '/') {
	free(fullpath);
	return;
    }

    /* Look for a matching filename in the list. */
    for (posptr = position_history; posptr != NULL; posptr = posptr->next) {
	if (!strcmp(posptr->filename, fullpath))
	    break;
	posprev = posptr;
    }

    /* Don't record files that have the default cursor position. */
    if (lineno == 1 && xpos == 1) {
	if (posptr != NULL) {
	    if (posprev == NULL)
		position_history = posptr->next;
	    else
		posprev->next = posptr->next;
	    free(posptr->filename);
	    free(posptr);
	}
	free(fullpath);
	return;
    }

    theone = posptr;

    /* If we didn't find it, make a new node; otherwise, if we're
     * not at the end, move the matching one to the end. */
    if (theone == NULL) {
	theone = (poshiststruct *)nmalloc(sizeof(poshiststruct));
	theone->filename = mallocstrcpy(NULL, fullpath);
	if (position_history == NULL)
	    position_history = theone;
	else
	    posprev->next = theone;
    } else if (posptr->next != NULL) {
	if (posprev == NULL)
	    position_history = posptr->next;
	else
	    posprev->next = posptr->next;
	while (posptr->next != NULL)
	    posptr = posptr->next;
	posptr->next = theone;
    }

    /* Store the last cursor position. */
    theone->lineno = lineno;
    theone->xno = xpos;
    theone->next = NULL;

    free(fullpath);
}

/* Check whether the given file matches an existing entry in the recorded
 * last file positions.  If not, return FALSE.  If yes, return TRUE and
 * set line and column to the retrieved values. */
bool has_old_position(const char *file, ssize_t *line, ssize_t *column)
{
    poshiststruct *posptr = position_history;
    char *fullpath = get_full_path(file);

    if (fullpath == NULL)
	return FALSE;

    while (posptr != NULL && strcmp(posptr->filename, fullpath) != 0)
	posptr = posptr->next;

    free(fullpath);

    if (posptr == NULL)
        return FALSE;

    *line = posptr->lineno;
    *column = posptr->xno;
    return TRUE;
}

/* Load the recorded file positions from ~/.nano/filepos_history. */
void load_poshistory(void)
{
    char *poshist = poshistfilename();
    FILE *hist;

    /* If the home directory is missing, do_rcfile() will have reported it. */
    if (poshist == NULL)
	return;

    hist = fopen(poshist, "rb");

    if (hist == NULL) {
	if (errno != ENOENT) {
	    /* When reading failed, don't save history when we quit. */
	    UNSET(POS_HISTORY);
	    history_error(N_("Error reading %s: %s"), poshist, strerror(errno));
	}
    } else {
	char *line = NULL, *lineptr, *xptr;
	size_t buf_len = 0;
	ssize_t read, count = 0;
	poshiststruct *record_ptr = NULL, *newrecord;

	/* Read and parse each line, and store the extracted data. */
	while ((read = getline(&line, &buf_len, hist)) > 5) {
	    /* Decode nulls as embedded newlines. */
	    unsunder(line, read);

	    /* Find where the x index and line number are in the line. */
	    xptr = revstrstr(line, " ", line + read - 3);
	    if (xptr == NULL)
		continue;
	    lineptr = revstrstr(line, " ", xptr - 2);
	    if (lineptr == NULL)
		continue;

	    /* Now separate the three elements of the line. */
	    *(xptr++) = '\0';
	    *(lineptr++) = '\0';

	    /* Create a new position record. */
	    newrecord = (poshiststruct *)nmalloc(sizeof(poshiststruct));
	    newrecord->filename = mallocstrcpy(NULL, line);
	    newrecord->lineno = atoi(lineptr);
	    newrecord->xno = atoi(xptr);
	    newrecord->next = NULL;

	    /* Add the record to the list. */
	    if (position_history == NULL)
		position_history = newrecord;
	    else
		record_ptr->next = newrecord;

	    record_ptr = newrecord;

	    /* Impose a limit, so the file will not grow indefinitely. */
	    if (++count > 200) {
		poshiststruct *drop_record = position_history;

		position_history = position_history->next;

		free(drop_record->filename);
		free(drop_record);
	    }
	}
	fclose(hist);
	free(line);
    }
    free(poshist);
}
#endif /* !DISABLE_HISTORIES */
>>>>>>> origin/tomato-shibby-RT-AC
