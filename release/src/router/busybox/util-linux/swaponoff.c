/* vi: set sw=4 ts=4: */
/*
 * Mini swapon/swapoff implementation for busybox
 *
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the GPL version 2, see the file LICENSE in this tarball.
 */

#include "libbb.h"
#include <mntent.h>
#include <sys/swap.h>

#if ENABLE_FEATURE_SWAPON_PRI
struct globals {
	int flags;
};
#define G (*(struct globals*)&bb_common_bufsiz1)
#define g_flags (G.flags)
#else
#define g_flags 0
#endif

static int swap_enable_disable(char *device)
{
	int status;
	struct stat st;

	xstat(device, &st);

#if ENABLE_DESKTOP
	/* test for holes */
	if (S_ISREG(st.st_mode))
		if (st.st_blocks * (off_t)512 < st.st_size)
			bb_error_msg("warning: swap file has holes");
#endif

	if (applet_name[5] == 'n')
		status = swapon(device, g_flags);
	else
		status = swapoff(device);

	if (status != 0) {
		bb_simple_perror_msg(device);
		return 1;
	}

	return 0;
}

static int do_em_all(void)
{
	struct mntent *m;
	FILE *f;
	int err;

	f = setmntent("/etc/fstab", "r");
	if (f == NULL)
		bb_perror_msg_and_die("/etc/fstab");

	err = 0;
	while ((m = getmntent(f)) != NULL)
		if (strcmp(m->mnt_type, MNTTYPE_SWAP) == 0)
			err += swap_enable_disable(m->mnt_fsname);

	endmntent(f);

	return err;
}

int swap_on_off_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int swap_on_off_main(int argc UNUSED_PARAM, char **argv)
{
	int ret;

#if !ENABLE_FEATURE_SWAPON_PRI
	ret = getopt32(argv, "a");
#else
	opt_complementary = "p+";
	ret = getopt32(argv, (applet_name[5] == 'n') ? "ap:" : "a", &g_flags);

	if (ret & 2) { // -p
		g_flags = SWAP_FLAG_PREFER |
			((g_flags & SWAP_FLAG_PRIO_MASK) << SWAP_FLAG_PRIO_SHIFT);
		ret &= 1;
	}
#endif

	if (ret /* & 1: not needed */) // -a
		return do_em_all();

	argv += optind;
	if (!*argv)
		bb_show_usage();

	/* ret = 0; redundant */
	do {
		ret += swap_enable_disable(*argv);
	} while (*++argv);

	return ret;
}
