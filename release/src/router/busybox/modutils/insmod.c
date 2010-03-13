/* vi: set sw=4 ts=4: */
/*
 * Mini insmod implementation for busybox
 *
 * Copyright (C) 2008 Timo Teras <timo.teras@iki.fi>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include "modutils.h"
#include <fnmatch.h>

static char *m_filename;

static int FAST_FUNC check_module_name_match(const char *filename,
		struct stat *statbuf UNUSED_PARAM,
		void *userdata, int depth UNUSED_PARAM)
{
	char *fullname = (char *) userdata;
	char *tmp;

	if (fullname[0] == '\0')
		return FALSE;

	tmp = bb_get_last_path_component_nostrip(filename);
	if (strcmp(tmp, fullname) == 0) {
		/* Stop searching if we find a match */
		m_filename = xstrdup(filename);
		return FALSE;
	}
	return TRUE;
}

int insmod_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int insmod_main(int argc UNUSED_PARAM, char **argv)
{
	struct stat st;
	char *filename;
	FILE *fp = NULL;
	int rc;

	/* Compat note:
	 * 2.6 style insmod has no options and required filename
	 * (not module name - .ko can't be omitted).
	 * 2.4 style insmod can take module name without .o
	 * and performs module search in default directories
	 * or in $MODPATH.
	 */

	USE_FEATURE_2_4_MODULES(
		getopt32(argv, INSMOD_OPTS INSMOD_ARGS);
		argv += optind - 1;
	);

	filename = *++argv;
	if (!filename)
		bb_show_usage();

	m_filename = NULL;
	/* Get a filedesc for the module.  Check if we have a complete path */
	if (stat(filename, &st) < 0 || !S_ISREG(st.st_mode) ||
		(fp = fopen_for_read(filename)) == NULL) {
		/* Hmm.  Could not open it. Search /lib/modules/ */
		int r, len;
		char *module_dir;

		len = strlen(filename);
		if (get_linux_version_code() < KERNEL_VERSION(2,6,0)) {
			if (len >= 2 && strncmp(&filename[len - 2], ".o", 2) !=0)
				filename = xasprintf("%s.o", filename);
		} else {
			if (len >= 3 && strncmp(&filename[len - 3], ".ko", 3) !=0)
				filename = xasprintf("%s.ko", filename);
		}

		module_dir = xmalloc_readlink(CONFIG_DEFAULT_MODULES_DIR);
		if (!module_dir)
			module_dir = xstrdup(CONFIG_DEFAULT_MODULES_DIR);
		r = recursive_action(module_dir, ACTION_RECURSE,
			check_module_name_match, NULL, filename, 0);
		free(module_dir);
		if (r)
			bb_error_msg_and_die("%s: module not found", filename);
		if (m_filename == NULL || ((fp = fopen_for_read(m_filename)) == NULL)) {
			bb_error_msg_and_die("%s: module not found", filename);
		}
		filename = m_filename;
	}
	if (fp != NULL)
		fclose(fp);

	rc = bb_init_module(filename, parse_cmdline_module_options(argv));
	if (rc)
		bb_error_msg("can't insert '%s': %s", filename, moderror(rc));
	
	free(m_filename);
	return rc;
}
