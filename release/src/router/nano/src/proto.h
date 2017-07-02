/* $Id: proto.h 4460 2009-12-09 16:51:43Z astyanax $ */
/**************************************************************************
 *   proto.h  --  This file is part of GNU nano.                          *
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

#ifndef PROTO_H
#define PROTO_H 1

#include "nano.h"

/* All external variables.  See global.c for their descriptions. */
#ifndef NANO_TINY
<<<<<<< HEAD
extern sigjmp_buf jump_buf;
extern bool jump_buf_main;
extern bool use_undo;
=======
extern volatile sig_atomic_t the_window_resized;
#endif

#ifdef __linux__
extern bool console;
#endif

extern bool meta_key;
extern bool shift_held;

extern bool focusing;

extern bool as_an_at;

extern int margin;
extern int editwincols;

extern message_type lastmessage;

extern filestruct *pletion_line;

extern int controlleft;
extern int controlright;
extern int controlup;
extern int controldown;
#ifndef NANO_TINY
extern int shiftcontrolleft;
extern int shiftcontrolright;
extern int shiftcontrolup;
extern int shiftcontroldown;
extern int shiftaltleft;
extern int shiftaltright;
extern int shiftaltup;
extern int shiftaltdown;
>>>>>>> origin/tomato-shibby-RT-AC
#endif

#ifndef DISABLE_WRAPJUSTIFY
extern ssize_t fill;
extern ssize_t wrap_at;
#endif

extern char *last_search;
extern char *last_replace;

extern unsigned flags[4];
extern WINDOW *topwin;
extern WINDOW *edit;
extern WINDOW *bottomwin;
extern int editwinrows;

extern filestruct *cutbuffer;
extern filestruct *cutbottom;
extern partition *filepart;
extern openfilestruct *openfile;

#ifndef NANO_TINY
extern char *matchbrackets;
#endif

#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
extern char *whitespace;
extern int whitespace_len[2];
extern undo_type last_action;
#endif

#ifndef DISABLE_JUSTIFY
extern char *punct;
extern char *brackets;
extern char *quotestr;
extern regex_t quotereg;
extern int quoterc;
extern char *quoteerr;
<<<<<<< HEAD
#else
extern size_t quotelen;
#endif
#endif
=======
#endif /* !DISABLE_JUSTIFY */

extern char *word_chars;

>>>>>>> origin/tomato-shibby-RT-AC
extern bool nodelay_mode;

extern char *answer;

extern ssize_t tabsize;

#ifndef NANO_TINY
extern char *backup_dir;
#endif
#ifndef DISABLE_OPERATINGDIR
extern char *operating_dir;
extern char *full_operating_dir;
#endif

#ifndef DISABLE_SPELLER
extern char *alt_speller;
#endif

extern sc *sclist;
extern subnfunc *allfuncs;
#ifdef ENABLE_COLOR
extern syntaxtype *syntaxes;
extern char *syntaxstr;
#endif

extern bool edit_refresh_needed;
extern const shortcut *currshortcut;
extern int currmenu;

#ifndef NANO_TINY
extern filestruct *search_history;
extern filestruct *searchage;
extern filestruct *searchbot;
extern filestruct *replace_history;
extern filestruct *replaceage;
extern filestruct *replacebot;
#endif

extern regex_t search_regexp;
extern regmatch_t regmatches[10];

<<<<<<< HEAD
extern int reverse_attr;
=======
extern int hilite_attribute;
#ifndef DISABLE_COLOR
extern char* specified_color_combo[NUMBER_OF_ELEMENTS];
#endif
extern int interface_color_pair[NUMBER_OF_ELEMENTS];
>>>>>>> origin/tomato-shibby-RT-AC

extern char *homedir;

/* All functions in browser.c. */
#ifndef DISABLE_BROWSER
char *do_browser(char *path, DIR *dir);
char *do_browse_from(const char *inpath);
void browser_init(const char *path, DIR *dir);
void parse_browser_input(int *kbinput, bool *meta_key, bool *func_key);
void browser_refresh(void);
bool browser_select_filename(const char *needle);
int filesearch_init(void);
bool findnextfile(bool no_sameline, size_t begin, const char *needle);
void findnextfile_wrap_reset(void);
void filesearch_abort(void);
void do_filesearch(void);
void do_fileresearch(void);
void do_first_file(void);
void do_last_file(void);
char *strip_last_component(const char *path);
#endif

/* All functions in chars.c. */
#ifdef ENABLE_UTF8
void utf8_init(void);
bool using_utf8(void);
#endif
<<<<<<< HEAD
#ifndef HAVE_ISBLANK
bool nisblank(int c);
#endif
#if !defined(HAVE_ISWBLANK) && defined(ENABLE_UTF8)
bool niswblank(wchar_t wc);
#endif
=======
char *addstrings(char* str1, size_t len1, char* str2, size_t len2);
>>>>>>> origin/tomato-shibby-RT-AC
bool is_byte(int c);
bool is_alpha_mbchar(const char *c);
bool is_alnum_mbchar(const char *c);
bool is_blank_mbchar(const char *c);
bool is_ascii_cntrl_char(int c);
bool is_cntrl_char(int c);
bool is_cntrl_mbchar(const char *c);
bool is_punct_mbchar(const char *c);
bool is_word_mbchar(const char *c, bool allow_punct);
<<<<<<< HEAD
char control_rep(char c);
#ifdef ENABLE_UTF8
wchar_t control_wrep(wchar_t c);
#endif
char *control_mbrep(const char *c, char *crep, int *crep_len);
char *mbrep(const char *c, char *crep, int *crep_len);
=======
char control_rep(const signed char c);
char control_mbrep(const char *c, bool isdata);
int length_of_char(const char *c, int *width);
>>>>>>> origin/tomato-shibby-RT-AC
int mbwidth(const char *c);
int mb_cur_max(void);
char *make_mbchar(long chr, int *chr_mb_len);
int parse_mbchar(const char *buf, char *chr, size_t *col);
size_t move_mbleft(const char *buf, size_t pos);
size_t move_mbright(const char *buf, size_t pos);
int mbstrcasecmp(const char *s1, const char *s2);
int mbstrncasecmp(const char *s1, const char *s2, size_t n);
char *mbstrcasestr(const char *haystack, const char *needle);
char *revstrstr(const char *haystack, const char *needle,
	const char *pointer);
char *revstrcasestr(const char *haystack, const char *needle, const char
	*rev_start);
char *mbrevstrcasestr(const char *haystack, const char *needle, const
	char *rev_start);
size_t mbstrlen(const char *s);
size_t mbstrnlen(const char *s, size_t maxlen);
#if !defined(NANO_TINY) || !defined(DISABLE_JUSTIFY)
char *mbstrchr(const char *s, const char *c);
#endif
#ifndef NANO_TINY
char *mbstrpbrk(const char *s, const char *accept);
char *revstrpbrk(const char *s, const char *accept, const char
	*rev_start);
char *mbrevstrpbrk(const char *s, const char *accept, const char
	*rev_start);
#endif
#if defined(ENABLE_NANORC) && (!defined(NANO_TINY) || !defined(DISABLE_JUSTIFY))
bool has_blank_chars(const char *s);
bool has_blank_mbchars(const char *s);
#endif
#ifdef ENABLE_UTF8
bool is_valid_unicode(wchar_t wc);
#endif
#ifdef ENABLE_NANORC
bool is_valid_mbstring(const char *s);
#endif

/* All functions in color.c. */
#ifdef ENABLE_COLOR
void set_colorpairs(void);
void color_init(void);
void color_update(void);
<<<<<<< HEAD
=======
void check_the_multis(filestruct *line);
void alloc_multidata_if_needed(filestruct *fileptr);
void precalc_multicolorinfo(void);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

/* All functions in cut.c. */
void cutbuffer_reset(void);
void cut_line(void);
#ifndef NANO_TINY
void cut_marked(bool *right_side_up);
void cut_to_eol(void);
void cut_to_eof(void);
#endif
<<<<<<< HEAD
void do_cut_text(
#ifndef NANO_TINY
	bool copy_text, bool cut_till_end, bool undoing
#else
	void
#endif
	);
=======
void do_cut_text(bool copy_text, bool cut_till_eof);
>>>>>>> origin/tomato-shibby-RT-AC
void do_cut_text_void(void);
#ifndef NANO_TINY
void do_copy_text(void);
void do_cut_till_end(void);
#endif
void do_uncut_text(void);

/* All functions in files.c. */
void make_new_buffer(void);
void initialize_buffer(void);
void initialize_buffer_text(void);
void open_buffer(const char *filename, bool undoable);
#ifndef DISABLE_SPELLER
void replace_buffer(const char *filename);
#ifndef NANO_TINY
void replace_marked_buffer(const char *filename, filestruct *top, size_t top_x,
	filestruct *bot, size_t bot_x);
#endif
#endif
void display_buffer(void);
#ifdef ENABLE_MULTIBUFFER
void switch_to_prevnext_buffer(bool next);
void switch_to_prev_buffer_void(void);
void switch_to_next_buffer_void(void);
bool close_buffer(void);
#endif
<<<<<<< HEAD
filestruct *read_line(char *buf, filestruct *prevnode, bool
	*first_line_ins, size_t buf_len);
void read_file(FILE *f, int fd, const char *filename, bool undoable, bool checkwritable);
int open_file(const char *filename, bool newfie, FILE **f);
=======
char *encode_data(char *buf, size_t buf_len);
void read_file(FILE *f, int fd, const char *filename, bool undoable,
		bool checkwritable);
int open_file(const char *filename, bool newfie, bool quiet, FILE **f);
>>>>>>> origin/tomato-shibby-RT-AC
char *get_next_filename(const char *name, const char *suffix);
void do_insertfile_void(void);
char *get_full_path(const char *origpath);
char *check_writable_directory(const char *path);
char *safe_tempfile(FILE **f);
#ifndef DISABLE_OPERATINGDIR
void init_operating_dir(void);
bool check_operating_dir(const char *currpath, bool allow_tabcomp);
#endif
#ifndef NANO_TINY
void init_backup_dir(void);
#endif
int copy_file(FILE *inn, FILE *out);
bool write_file(const char *name, FILE *f_open, bool tmp,
	kind_of_writing_type method, bool nonamechange);
#ifndef NANO_TINY
bool write_marked_file(const char *name, FILE *f_open, bool tmp,
	kind_of_writing_type method);
#endif
bool do_writeout(bool exiting);
void do_writeout_void(void);
char *real_dir_from_tilde(const char *buf);
#if !defined(DISABLE_TABCOMP) || !defined(DISABLE_BROWSER)
int diralphasort(const void *va, const void *vb);
void free_chararray(char **array, size_t len);
#endif
#ifndef DISABLE_TABCOMP
bool is_dir(const char *buf);
char **username_tab_completion(const char *buf, size_t *num_matches,
	size_t buf_len);
char **cwd_tab_completion(const char *buf, bool allow_files, size_t
	*num_matches, size_t buf_len);
char *input_tab(char *buf, bool allow_files, size_t *place, bool
	*lastwastab, void (*refresh_func)(void), bool *list);
#endif
const char *tail(const char *foo);
#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
char *histfilename(void);
void load_history(void);
bool writehist(FILE *hist, const filestruct *head);
void save_history(void);
<<<<<<< HEAD
=======
int check_dotnano(void);
void load_poshistory(void);
void save_poshistory(void);
void update_poshistory(char *filename, ssize_t lineno, ssize_t xpos);
bool has_old_position(const char *file, ssize_t *line, ssize_t *column);
>>>>>>> origin/tomato-shibby-RT-AC
#endif

/* All functions in global.c. */
size_t length_of_list(int menu);
<<<<<<< HEAD
#ifndef NANO_TINY
void toggle_init_one(int val
#ifndef DISABLE_HELP
	, const char *desc, bool blank_after
=======
const sc *first_sc_for(int menu, void (*func)(void));
int the_code_for(void (*func)(void), int defaultval);
functionptrtype func_from_key(int *kbinput);
void assign_keyinfo(sc *s, const char *keystring, const int keycode);
void print_sclist(void);
void shortcut_init(void);
#ifndef DISABLE_COLOR
void set_lint_or_format_shortcuts(void);
void set_spell_shortcuts(void);
>>>>>>> origin/tomato-shibby-RT-AC
#endif
	, long flag);
void toggle_init(void);
#endif
void sc_init_one(shortcut **shortcutage, int ctrlval, const char *desc
#ifndef DISABLE_HELP
	, const char *help, bool blank_after
#endif
	, int metaval, int funcval, int miscval, bool view, void
	(*func)(void));
void shortcut_init(bool unjustify);
void free_shortcutage(shortcut **shortcutage);
#ifdef DEBUG
void thanks_for_all_the_fish(void);
#endif

/* All functions in help.c. */
#ifndef DISABLE_BROWSER
void do_browser_help(void);
#endif
void do_help_void(void);
#ifndef DISABLE_HELP
void do_help(void (*refresh_func)(void));
void help_init(void);
void parse_help_input(int *kbinput, bool *meta_key, bool *func_key);
size_t help_line_len(const char *ptr);
#endif

/* All functions in move.c. */
void do_first_line(void);
void do_last_line(void);
void do_page_up(void);
void do_page_down(void);
#ifndef DISABLE_JUSTIFY
void do_para_begin(bool allow_update);
void do_para_begin_void(void);
void do_para_end(bool allow_update);
void do_para_end_void(void);
#endif
<<<<<<< HEAD
#ifndef NANO_TINY
bool do_next_word(bool allow_punct, bool allow_update);
void do_next_word_void(void);
bool do_prev_word(bool allow_punct, bool allow_update);
void do_prev_word_void(void);
#endif
void do_home(void);
void do_end(void);
void do_up(
#ifndef NANO_TINY
	bool scroll_only
#else
	void
#endif
	);
=======
void do_prev_block(void);
void do_next_block(void);
void do_prev_word(bool allow_punct, bool allow_update);
void do_prev_word_void(void);
bool do_next_word(bool allow_punct, bool allow_update);
void do_next_word_void(void);
void do_home(bool be_clever);
void do_home_void(void);
void do_end(bool be_clever);
void do_end_void(void);
void do_up(bool scroll_only);
>>>>>>> origin/tomato-shibby-RT-AC
void do_up_void(void);
void do_down(bool scroll_only);
void do_down_void(void);
#ifndef NANO_TINY
void do_scroll_up(void);
void do_scroll_down(void);
#endif
void do_left(void);
void do_right(void);

/* All functions in nano.c. */
filestruct *make_new_node(filestruct *prevnode);
filestruct *copy_node(const filestruct *src);
void splice_node(filestruct *begin, filestruct *newnode, filestruct
	*end);
void unlink_node(const filestruct *fileptr);
void delete_node(filestruct *fileptr);
filestruct *copy_filestruct(const filestruct *src);
void free_filestruct(filestruct *src);
void renumber(filestruct *fileptr);
partition *partition_filestruct(filestruct *top, size_t top_x,
	filestruct *bot, size_t bot_x);
void unpartition_filestruct(partition **p);
void extract_buffer(filestruct **file_top, filestruct **file_bot,
	filestruct *top, size_t top_x, filestruct *bot, size_t bot_x);
<<<<<<< HEAD
void copy_from_filestruct(filestruct *file_top, filestruct *file_bot);
=======
void ingraft_buffer(filestruct *somebuffer);
void copy_from_buffer(filestruct *somebuffer);
>>>>>>> origin/tomato-shibby-RT-AC
openfilestruct *make_new_opennode(void);
void splice_opennode(openfilestruct *begin, openfilestruct *newnode,
	openfilestruct *end);
void unlink_opennode(openfilestruct *fileptr);
void delete_opennode(openfilestruct *fileptr);
#ifdef DEBUG
void free_openfilestruct(openfilestruct *src);
#endif
void print_view_warning(void);
void finish(void);
void die(const char *msg, ...);
void die_save_file(const char *die_filename, struct stat *die_stat);
void window_init(void);
#ifndef DISABLE_MOUSE
void disable_mouse_support(void);
void enable_mouse_support(void);
void mouse_init(void);
#endif
void print_opt(const char *shortflag, const char *longflag, const char *desc);
void usage(void);
void version(void);
<<<<<<< HEAD
int no_more_space(void);
int no_help(void);
void nano_disabled_msg(void);
=======
>>>>>>> origin/tomato-shibby-RT-AC
void do_exit(void);
void signal_init(void);
RETSIGTYPE handle_hupterm(int signal);
RETSIGTYPE do_suspend(int signal);
RETSIGTYPE do_continue(int signal);
#ifndef NANO_TINY
RETSIGTYPE handle_sigwinch(int signal);
void allow_pending_sigwinch(bool allow);
#endif
#ifndef NANO_TINY
void do_toggle(int flag);
<<<<<<< HEAD
#endif
void disable_extended_io(void);
#ifdef USE_SLANG
void disable_signals(void);
#endif
#ifndef NANO_TINY
=======
void do_toggle_void(void);
>>>>>>> origin/tomato-shibby-RT-AC
void enable_signals(void);
#endif
void disable_flow_control(void);
void enable_flow_control(void);
void terminal_init(void);
int do_input(bool *meta_key, bool *func_key, bool *have_shortcut, bool
	*ran_func, bool *finished, bool allow_funcs);
#ifndef DISABLE_MOUSE
int do_mouse(void);
#endif
void do_output(char *output, size_t output_len, bool allow_cntrls);

<<<<<<< HEAD
/* All functions in prompt.c. */
int do_statusbar_input(bool *meta_key, bool *func_key, bool *have_shortcut,
	bool *ran_func, bool *finished, bool allow_funcs, void
	(*refresh_func)(void));
#ifndef DISABLE_MOUSE
int do_statusbar_mouse(void);
#endif
void do_statusbar_output(char *output, size_t output_len, bool
	*got_enter, bool allow_cntrls);
=======
/* Most functions in prompt.c. */
#ifndef DISABLE_MOUSE
int do_statusbar_mouse(void);
#endif
void do_statusbar_output(int *the_input, size_t input_len,
	bool filtering);
>>>>>>> origin/tomato-shibby-RT-AC
void do_statusbar_home(void);
void do_statusbar_end(void);
void do_statusbar_left(void);
void do_statusbar_right(void);
void do_statusbar_backspace(void);
void do_statusbar_delete(void);
void do_statusbar_cut_text(void);
#ifndef NANO_TINY
bool do_statusbar_next_word(bool allow_punct);
bool do_statusbar_prev_word(bool allow_punct);
#endif
<<<<<<< HEAD
void do_statusbar_verbatim_input(bool *got_enter);
#ifndef NANO_TINY
bool find_statusbar_bracket_match(bool reverse, const char
	*bracket_set);
void do_statusbar_find_bracket(void);
#endif
=======
void do_statusbar_verbatim_input(void);
>>>>>>> origin/tomato-shibby-RT-AC
size_t statusbar_xplustabs(void);
size_t get_statusbar_page_start(size_t start_col, size_t column);
void reset_statusbar_cursor(void);
<<<<<<< HEAD
void update_statusbar_line(const char *curranswer, size_t index);
bool need_statusbar_horizontal_update(size_t pww_save);
void total_statusbar_refresh(void (*refresh_func)(void));
const sc *get_prompt_string(int *value, bool allow_tabs,
#ifndef DISABLE_TABCOMP
	bool allow_files,
#endif
	const char *curranswer,
	bool *meta_key, bool *func_key,
#ifndef NANO_TINY
	filestruct **history_list,
#endif
	void (*refresh_func)(void), int menu
#ifndef DISABLE_TABCOMP
	, bool *list
#endif
	);
int do_prompt(bool allow_tabs,
#ifndef DISABLE_TABCOMP
	bool allow_files,
#endif
=======
void update_the_statusbar(void);
int do_prompt(bool allow_tabs, bool allow_files,
>>>>>>> origin/tomato-shibby-RT-AC
	int menu, const char *curranswer,
	bool *meta_key, bool *func_key,
#ifndef NANO_TINY
	filestruct **history_list,
#endif
	void (*refresh_func)(void), const char *msg, ...);
void do_prompt_abort(void);
int do_yesno_prompt(bool all, const char *msg);

<<<<<<< HEAD
/* All functions in rcfile.c. */
#ifdef ENABLE_NANORC
void rcfile_error(const char *msg, ...);
char *parse_next_word(char *ptr);
char *parse_argument(char *ptr);
#ifdef ENABLE_COLOR
char *parse_next_regex(char *ptr);
bool nregcomp(const char *regex, int eflags);
void parse_syntax(char *ptr);
void parse_include(char *ptr);
short color_to_short(const char *colorname, bool *bright);
void parse_colors(char *ptr, bool icase);
void reset_multis(filestruct *fileptr, bool force);
void alloc_multidata_if_needed(filestruct *fileptr);
#endif
void parse_rcfile(FILE *rcstream
#ifdef ENABLE_COLOR
	, bool syntax_only
#endif
	);
void do_rcfile(void);
#endif
=======
/* Most functions in rcfile.c. */
#if !defined(DISABLE_NANORC) || !defined(DISABLE_HISTORIES)
char *parse_next_word(char *ptr);
#endif
#ifndef DISABLE_NANORC
#ifndef DISABLE_COLOR
bool parse_color_names(char *combostr, short *fg, short *bg, bool *bright);
void grab_and_store(const char *kind, char *ptr, regexlisttype **storage);
#endif
void parse_rcfile(FILE *rcstream, bool syntax_only);
void do_rcfiles(void);
#endif /* !DISABLE_NANORC */
>>>>>>> origin/tomato-shibby-RT-AC

/* All functions in search.c. */
bool regexp_init(const char *regexp);
void regexp_cleanup(void);
void not_found_msg(const char *str);
void search_replace_abort(void);
void search_init_globals(void);
int search_init(bool replacing, bool use_answer);
<<<<<<< HEAD
bool findnextstr(
#ifndef DISABLE_SPELLER
	bool whole_word,
#endif
	bool no_sameline, const filestruct *begin, size_t begin_x, const
	char *needle, size_t *needle_len);
void findnextstr_wrap_reset(void);
void do_search(void);
#ifndef NANO_TINY
void do_research(void);
#endif
#ifdef HAVE_REGEX_H
=======
int findnextstr(const char *needle, bool whole_word_only, bool have_region,
	size_t *match_len, bool skipone, const filestruct *begin, size_t begin_x);
void do_search(void);
#ifndef NANO_TINY
void do_findprevious(void);
void do_findnext(void);
#endif
void do_research(void);
void go_looking(void);
>>>>>>> origin/tomato-shibby-RT-AC
int replace_regexp(char *string, bool create);
char *replace_line(const char *needle);
<<<<<<< HEAD
ssize_t do_replace_loop(
#ifndef DISABLE_SPELLER
	bool whole_word,
#endif
	bool *canceled, const filestruct *real_current, size_t
	*real_current_x, const char *needle);
=======
ssize_t do_replace_loop(const char *needle, bool whole_word_only,
	const filestruct *real_current, size_t *real_current_x);
>>>>>>> origin/tomato-shibby-RT-AC
void do_replace(void);
void do_gotolinecolumn(ssize_t line, ssize_t column, bool use_answer,
	bool interactive, bool save_pos, bool allow_update);
void do_gotolinecolumn_void(void);
#ifndef DISABLE_SPELLER
void do_gotopos(ssize_t pos_line, size_t pos_x, ssize_t pos_y, size_t
	pos_pww);
#endif
#ifndef NANO_TINY
bool find_bracket_match(bool reverse, const char *bracket_set);
void do_find_bracket(void);
#ifdef ENABLE_NANORC
bool history_has_changed(void);
#endif
void history_init(void);
void history_reset(const filestruct *h);
filestruct *find_history(const filestruct *h_start, const filestruct
	*h_end, const char *s, size_t len);
void update_history(filestruct **h, const char *s);
char *get_history_older(filestruct **h);
char *get_history_newer(filestruct **h);
#ifndef DISABLE_TABCOMP
char *get_history_completion(filestruct **h, const char *s, size_t len);
#endif
#endif

/* All functions in text.c. */
#ifndef NANO_TINY
void do_mark(void);
#endif
void do_delete(void);
void do_backspace(void);
void do_tab(void);
#ifndef NANO_TINY
void do_indent(ssize_t cols);
void do_indent_void(void);
void do_unindent(void);
#endif
<<<<<<< HEAD
void do_enter(bool undoing);
=======
bool white_string(const char *s);
#ifdef ENABLE_COMMENT
void do_comment(void);
bool comment_line(undo_type action, filestruct *f, const char *comment_seq);
#endif
void do_undo(void);
void do_redo(void);
void do_enter(void);
>>>>>>> origin/tomato-shibby-RT-AC
#ifndef NANO_TINY
RETSIGTYPE cancel_command(int signal);
bool execute_command(const char *command);
void discard_until(const undo *thisitem, openfilestruct *thefile);
void add_undo(undo_type action);
#ifndef DISABLE_COMMENT
void update_comment_undo(ssize_t lineno);
#endif
void update_undo(undo_type action);
#endif /* !NANO_TINY */
#ifndef DISABLE_WRAPPING
void wrap_reset(void);
bool do_wrap(filestruct *line, bool undoing);
#endif
#if !defined(DISABLE_HELP) || !defined(DISABLE_WRAPJUSTIFY)
ssize_t break_line(const char *line, ssize_t goal, bool snap_at_nl);
#endif
#if !defined(NANO_TINY) || !defined(DISABLE_JUSTIFY)
size_t indent_length(const char *line);
#endif
#ifndef DISABLE_JUSTIFY
void justify_format(filestruct *paragraph, size_t skip);
size_t quote_length(const char *line);
bool quotes_match(const char *a_line, size_t a_quote, const char
	*b_line);
bool indents_match(const char *a_line, size_t a_indent, const char
	*b_line, size_t b_indent);
bool begpar(const filestruct *const foo);
bool inpar(const filestruct *const foo);
void backup_lines(filestruct *first_line, size_t par_len);
bool find_paragraph(size_t *const quote, size_t *const par);
void do_justify(bool full_justify);
void do_justify_void(void);
void do_full_justify(void);
#endif
#ifndef DISABLE_SPELLER
bool do_int_spell_fix(const char *word);
const char *do_int_speller(const char *tempfile_name);
const char *do_alt_speller(char *tempfile_name);
void do_spell(void);
#endif
#ifndef NANO_TINY
void do_wordlinechar_count(void);
#endif
void do_verbatim_input(void);
void complete_a_word(void);

/* All functions in utils.c. */
int digits(size_t n);
void get_homedir(void);
#ifdef ENABLE_LINENUMBERS
int digits(ssize_t n);
#endif
bool parse_num(const char *str, ssize_t *val);
bool parse_line_column(const char *str, ssize_t *line, ssize_t *column);
void snuggly_fit(char **str);
void null_at(char **data, size_t index);
void unsunder(char *str, size_t true_len);
void sunder(char *str);
<<<<<<< HEAD
#if !defined(NANO_TINY) && defined(ENABLE_NANORC)
#ifndef HAVE_GETLINE
ssize_t ngetline(char **lineptr, size_t *n, FILE *stream);
#endif
#ifndef HAVE_GETDELIM
ssize_t ngetdelim(char **lineptr, size_t *n, int delim, FILE *stream);
#endif
#endif
#ifdef HAVE_REGEX_H
bool regexp_bol_or_eol(const regex_t *preg, const char *string);
=======
>>>>>>> origin/tomato-shibby-RT-AC
const char *fixbounds(const char *r);
#ifndef DISABLE_SPELLER
bool is_whole_word(size_t pos, const char *buf, const char *word);
#endif
const char *strstrwrapper(const char *haystack, const char *needle,
	const char *start);
void nperror(const char *s);
void *nmalloc(size_t howmuch);
void *nrealloc(void *ptr, size_t howmuch);
char *mallocstrncpy(char *dest, const char *src, size_t n);
char *mallocstrcpy(char *dest, const char *src);
char *free_and_assign(char *dest, char *src);
size_t get_page_start(size_t column);
size_t xplustabs(void);
size_t actual_x(const char *s, size_t column);
size_t strnlenpt(const char *s, size_t maxlen);
size_t strlenpt(const char *s);
void new_magicline(void);
#ifndef NANO_TINY
void remove_magicline(void);
void mark_order(const filestruct **top, size_t *top_x, const filestruct
	**bot, size_t *bot_x, bool *right_side_up);
<<<<<<< HEAD
void add_undo(undo_type current_action);
void update_undo(undo_type action);
=======
>>>>>>> origin/tomato-shibby-RT-AC
#endif
size_t get_totsize(const filestruct *begin, const filestruct *end);
#ifndef NANO_TINY
filestruct *fsfromline(ssize_t lineno);
#endif
#ifdef DEBUG
void dump_filestruct(const filestruct *inptr);
void dump_filestruct_reverse(void);
#endif

/* All functions in winio.c. */
void get_key_buffer(WINDOW *win);
size_t get_key_buffer_len(void);
void unget_input(int *input, size_t input_len);
<<<<<<< HEAD
void unget_kbinput(int kbinput, bool meta_key, bool func_key);
=======
void unget_kbinput(int kbinput, bool metakey);
>>>>>>> origin/tomato-shibby-RT-AC
int *get_input(WINDOW *win, size_t input_len);
int get_kbinput(WINDOW *win, bool *meta_key, bool *func_key);
int parse_kbinput(WINDOW *win, bool *meta_key, bool *func_key);
int get_escape_seq_kbinput(const int *seq, size_t seq_len);
int get_escape_seq_abcd(int kbinput);
int parse_escape_seq_kbinput(WINDOW *win, int kbinput);
int get_byte_kbinput(int kbinput);
#ifdef ENABLE_UTF8
long add_unicode_digit(int kbinput, long factor, long *uni);
long get_unicode_kbinput(WINDOW *win, int kbinput);
#endif
int get_control_kbinput(int kbinput);
void unparse_kbinput(char *output, size_t output_len);
int *get_verbatim_kbinput(WINDOW *win, size_t *kbinput_len);
int *parse_verbatim_kbinput(WINDOW *win, size_t *count);
#ifndef DISABLE_MOUSE
int get_mouseinput(int *mouse_x, int *mouse_y, bool allow_shortcuts);
#endif
<<<<<<< HEAD
const sc *get_shortcut(int menu, int *kbinput, bool
	*meta_key, bool *func_key);
const sc *first_sc_for(int menu, short func);
void blank_line(WINDOW *win, int y, int x, int n);
=======
const sc *get_shortcut(int *kbinput);
void blank_row(WINDOW *win, int y, int x, int n);
>>>>>>> origin/tomato-shibby-RT-AC
void blank_titlebar(void);
void blank_edit(void);
void blank_statusbar(void);
void blank_bottombars(void);
void check_statusblank(void);
char *display_string(const char *buf, size_t start_col, size_t span,
	bool dollars);
void titlebar(const char *path);
<<<<<<< HEAD
void set_modified(void);
void statusbar(const char *msg, ...);
=======
extern void set_modified(void);
void statusbar(const char *msg);
void warn_and_shortly_pause(const char *msg);
void statusline(message_type importance, const char *msg, ...);
>>>>>>> origin/tomato-shibby-RT-AC
void bottombars(int menu);
void onekey(const char *keystroke, const char *desc, size_t len);
void reset_cursor(void);
void edit_draw(filestruct *fileptr, const char *converted,
	int line, size_t from_col);
int update_line(filestruct *fileptr, size_t index);
<<<<<<< HEAD
bool need_horizontal_update(size_t pww_save);
bool need_vertical_update(size_t pww_save);
void edit_scroll(scroll_dir direction, ssize_t nlines);
void edit_redraw(filestruct *old_current, size_t pww_save);
=======
#ifndef NANO_TINY
int update_softwrapped_line(filestruct *fileptr);
#endif
bool line_needs_update(const size_t old_column, const size_t new_column);
int go_back_chunks(int nrows, filestruct **line, size_t *leftedge);
int go_forward_chunks(int nrows, filestruct **line, size_t *leftedge);
bool less_than_a_screenful(size_t was_lineno, size_t was_leftedge);
void edit_scroll(scroll_dir direction, int nrows);
void ensure_firstcolumn_is_aligned(void);
bool current_is_above_screen(void);
bool current_is_below_screen(void);
bool current_is_offscreen(void);
void edit_redraw(filestruct *old_current);
>>>>>>> origin/tomato-shibby-RT-AC
void edit_refresh(void);
void adjust_viewport(update_type location);
void total_redraw(void);
void total_refresh(void);
void display_main_list(void);
void do_cursorpos(bool force);
void do_cursorpos_void(void);
void do_replace_highlight(bool highlight, const char *word);
const char *flagtostr(int flag);
const subnfunc *sctofunc(sc *s);
const subnfunc *getfuncfromkey(WINDOW *win);
void print_sclist(void);
sc *strtosc(int menu, char *input);
function_type strtokeytype(const char *str);
int strtomenu(char *input);
void assign_keyinfo(sc *s);
void xon_complaint(void);
void xoff_complaint(void);
int sc_seq_or (short func, int defaultval);
void do_suspend_void(void);

extern const char *cancel_msg;
#ifndef NANO_TINY
extern const char *case_sens_msg;
extern const char *backwards_msg;
extern const char *prev_history_msg;
extern const char *next_history_msg;
#endif
extern const char *replace_msg;
extern const char *no_replace_msg;
extern const char *go_to_line_msg;
extern const char *whereis_next_msg;
extern const char *first_file_msg;
extern const char *last_file_msg;
extern const char *goto_dir_msg;
extern const char *ext_cmd_msg;
extern const char *to_files_msg;
extern const char *dos_format_msg;
extern const char *mac_format_msg;
extern const char *append_msg;
extern const char *prepend_msg;
extern const char *backup_file_msg;
extern const char *gototext_msg;
extern const char *new_buffer_msg;

void iso_me_harder_funcmap(short func);
void enable_nodelay(void);
void disable_nodelay(void);

#ifdef HAVE_REGEX_H
extern const char *regexp_msg;
#endif

#ifdef NANO_EXTRA
void do_credits(void);
#endif

#endif /* !PROTO_H */
