/*

	NVRAM Utility
	Copyright (C) 2006-2009 Jonathan Zarate
	              2017 Tomasz Słodkowicz

*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#include <bcmdevs.h>
#include <bcmnvram.h>
#include <utils.h>
#include <shutils.h>
#include <shared.h>

#include "defaults.h"

#define reterr(...) do { fprintf(stderr, __VA_ARGS__); return 1; } while (0)
static void help(const char *applet) __attribute__ ((noreturn));

static void getall(char *buffer) {
	if (nvram_getall(buffer, NVRAM_SPACE)) {
		fputs("Error reading NVRAM\n", stderr);
		exit(1);
	}
}

static int set_main(int argc, char **argv) {
	char *k, *v = argv[1];

	k = strsep(&v, "=");
	if (v) return nvram_set(k, v);
	help(*argv);
}

static int get_main(int argc, char **argv) {
	char *p;

	if ((p = nvram_get(argv[1])) != NULL) {
		puts(p);
		return 0;
	}
	return 1;
}

static int unset_main(int argc, char **argv) {
	return nvram_unset(argv[1]);
}

static int ren_main(int argc, char **argv) {
	char *p;

	if (strcmp(argv[1], argv[2])) {
		if ((p = nvram_get(argv[1])) == NULL) reterr("Unable to find %s\n", argv[1]);
		if (nvram_set(argv[2], p) == 0) return nvram_unset(argv[1]);
		reterr("Unable to set %s\n", argv[2]);
	}
	help(*argv);
}

extern int nvram_file2nvram(const char *name, const char *filename);
extern int nvram_nvram2file(const char *name, const char *filename);

static int f2n_main(int argc, char **argv) {
	return (nvram_file2nvram(argv[1], argv[2]));
}

static int n2f_main(int argc, char **argv) {
	return (nvram_nvram2file(argv[1], argv[2]));
}

static int save2f_main(int argc, char **argv) {
	char name[128] = "FILE:";

	strcpy(name+5, argv[1]);
	return (nvram_file2nvram(name, argv[1]));
}

static int cmpstringp(const void *a, const void *b) { return strcmp(*(const char**)a, *(const char**)b); }

static int show_main(int argc, char **argv) {
	char *p, *q;
	const char **index = NULL, *find = NULL;
	char buffer[NVRAM_SPACE];
	int n, count = 0;
	int index_size = 0;
	int stat = 1;
	int sort = 1;
	int ignore_case = 0;

	for (n = 1; n < argc; ++n)
		if (strcmp(argv[n], "--nostat") == 0) stat = 0;
		else if (strcmp(argv[n], "--nosort") == 0) sort = 0;
		else if (strcmp(argv[n], "--ignore-case") == 0) ignore_case = 1;
		else if (strcmp(argv[n], "--find") == 0) {
		   if (n+1 == argc) help(*argv);
		   find = argv[++n];
		} else help(*argv);
	if (ignore_case && !find) help(*argv);

	getall(buffer);
	for (n = 0, p = buffer; *p; p += strlen(p)+1) {
		for (q = p; *q; ++q) if (!isprint(*q)) *q = ' ';
		if (!find || (ignore_case && strcasestr(p, find)) || strstr(p, find)) {
			count++;
			if (stat) n += strlen(p)+1;
			if (sort) {
				if (count > index_size) {
					index_size += index_size ? 100 : 1000 ;
					if ((index = (const char **)realloc(index, index_size*sizeof(char *))) == NULL )
						reterr("Not enough memory\n");
				}
				index[count-1] = p;
			} else puts(p);
		}
	}

	if (sort) {
		int i;
		qsort(index, count, sizeof(char *), cmpstringp);
		for (i = 0; i < count; i++) puts(index[i]);
		if (index) free(index);
	}

	if (stat) {
		int used = sizeof(struct nvram_header) + (p - buffer);
		puts("---");
		if (find)
			if (count) printf("%d entr%s (%d bytes) matched", count, count==1?"y":"ies", n);
			else printf("0 entries matched");
		else printf ("%d entries", count);
		printf(", total %d bytes used, %d bytes free.\n", used, NVRAM_SPACE-used);
	}
	return count ? 0 : 1;
}

static int find_main(int argc, char **argv) {
	int n, nargc;
	const char *cmd[] = { argv[0], "--nostat", "--find", NULL, NULL, NULL, NULL };

	for (n = 1, nargc = 4; n < argc && nargc < sizeof(cmd)/sizeof(*cmd); ++n)
		if (!strcmp(argv[n], "--nosort") || !strcmp(argv[n], "--ignore-case")) cmd[nargc++] = argv[n];
		else if (!cmd[3]) cmd[3]=argv[n];
		else help(*argv);
	if (!cmd[3] || nargc == sizeof(cmd)/sizeof(*cmd)) help(*argv);

	return show_main(nargc, cmd);
}

static const char *default_value_for_model(const defaults_t *t) {
	int model = get_model();

	if (strcmp(t->key, "wl_txpwr") == 0) {
		switch (model) {
		case MODEL_WHRG54S:
			return "28";
#ifdef CONFIG_BCMWL5
		case MODEL_RTN10:
		case MODEL_RTN12:
		case MODEL_RTN16:
			return "17";
#endif
		}
	}
#ifdef TCONFIG_USB
	else if (strcmp(t->key, "usb_enable") == 0) {
		switch (model) {
		case MODEL_WRTSL54GS:
		case MODEL_WL500W:
		case MODEL_WL500GP:
		case MODEL_WL500GPv2:
		case MODEL_WL500GE:
		case MODEL_WL500GD:
		case MODEL_WL520GU:
		case MODEL_DIR320:
		case MODEL_H618B:
		case MODEL_WNR3500L:
		case MODEL_RTN16:
		case MODEL_WRT610Nv2:
		case MODEL_F7D3301:
		case MODEL_F7D3302:
		case MODEL_F7D4301:
		case MODEL_F7D4302:
		case MODEL_F5D8235v3:
			return "1";
		}
	}
#endif
	return t->value;
}

static void nv_fix_wl(const char *oldnv, const char *newnv) {
	char *p;

	if (nvram_get(wl_nvname(newnv, 0, 0)) == NULL) {
		p = nvram_get(oldnv);
		if (p) nvram_set(wl_nvname(newnv, -1, 0), p);
		nvram_unset(oldnv);
	}
}

static int validate_main(int argc, char **argv) {
	const defaults_t *t;
	char *p;
	int i;
	int force = 0;
	int unit = 0;

	for (i = 1; i < argc; ++i)
		if (strcmp(argv[i], "--restore") == 0) force = 1;
		else if (strncmp(argv[i], "--wl", 4) == 0) unit = atoi(argv[i] + 4);

	for (t = defaults; t->key; t++)
		if (strncmp(t->key, "wl_", 3) == 0) {
			// sync wl_ and wlX_
			p = wl_nvname(t->key + 3, unit, 0);
			if (force || nvram_get(p) == NULL)
				nvram_set(p, default_value_for_model(t));
		}

	return 0;
}

static int defaults_main(int argc, char **argv) {
	const defaults_t *t;
	char *p;
	char s[256];
	int i, j;
	int force = 0;
	int commit = 0;

	if (strcmp(argv[1], "--yes") == 0) force = 1;
	else if (strcmp(argv[1], "--initcheck") != 0) help(*argv);

	if (!nvram_match("restore_defaults", "0") || !nvram_match("os_name", "linux"))
		force = 1;

#if 0	// --need to test--
	// prevent lockout if upgrading from DD-WRT v23 SP2+ w/ encrypted password
	if (nvram_match("nvram_ver", "2")) {
		nvram_unset("nvram_ver");

		// If username is "", root or admin, then nvram_ver was probably a left-over
		// from an old DD-WRT installation (ex: DD-WRT -> Linksys (reset) ->
		// Tomato) and we don't need to do anything. Otherwise, reset.
		if ((p = nvram_get("httpd_username")) != NULL) {
			if ((*p != 0) && (strcmp(p, "root") != 0) && (strcmp(p, "admin") != 0)) {
				nvram_unset("httpd_passwd");
				nvram_unset("httpd_username");	// not used here, but dd will try to re-encrypt this
				// filled below
			}
		}
	}
#else
	if (force) nvram_unset("nvram_ver");	// prep to prevent problems later
#endif

	// keep the compatibility with old security_mode2, wds_enable, mac_wl - removeme after 1/1/2012
	nv_fix_wl("security_mode2", "security_mode");
	nv_fix_wl("wds_enable", "wds_enable");
	nv_fix_wl("mac_wl", "macaddr");

	for (t = defaults; t->key; t++)
		if (((p = nvram_get(t->key)) == NULL) || force) {
			if (default_value_for_model(t) == NULL) {
				if (p != NULL) {
					nvram_unset(t->key);
					if (!force) _dprintf("%s=%s is not the default (NULL) - resetting\n", t->key, p);
					commit = 1;
				}
			} else {
				nvram_set(t->key, default_value_for_model(t));
				if (!force) _dprintf("%s=%s is not the default (%s) - resetting\n", t->key, p ? p : "(NULL)", default_value_for_model(t));
				commit = 1;
			}
		} else if (strncmp(t->key, "wl_", 3) == 0) {
			// sync wl_ and wl0_
			strcpy(s, "wl0_");
			strcat(s, t->key + 3);
			if (nvram_get(s) == NULL) nvram_set(s, nvram_safe_get(t->key));
		}

	// todo: moveme
	if ((strtoul(nvram_safe_get("boardflags"), NULL, 0) & BFL_ENETVLAN) || (check_hw_type() == HW_BCM4712))
		t = if_vlan;
	else t = if_generic;
	for (; t->key; t++)
		if (((p = nvram_get(t->key)) == NULL) || (*p == 0) || (force)) {
			nvram_set(t->key, t->value);
			commit = 1;
			if (!force) _dprintf("%s=%s is not the default (%s) - resetting\n", t->key, p ? p : "(NULL)", t->value);
		}

	if (force) {
		for (j = 0; j < 2; j++)
			for (i = 0; i < 20; i++) {
				sprintf(s, "wl%d_wds%d", j, i);
				nvram_unset(s);
			}

		for (i = 0; i < LED_COUNT; ++i) {
			sprintf(s, "led_%s", led_names[i]);
			nvram_unset(s);
		}

		// 0 = example
		for (i = 1; i < 50; i++) {
			sprintf(s, "rrule%d", i);
			nvram_unset(s);
		}
	}

	if (!nvram_match("boot_wait", "on")) {
		nvram_set("boot_wait", "on");
		commit = 1;
	}

	nvram_set("os_name", "linux");
	nvram_set("os_version", tomato_version);
	nvram_set("os_date", tomato_buildtime);

	if (commit || force) {
		puts("Saving...");
		return nvram_commit() ? 1 : 0;
	}
	puts("No change was necessary");
	return 0;
}

static int commit_main(int argc, char **argv) {
	int r;

	printf("Commit... ");
	fflush(stdout);
	r = nvram_commit();
	puts("done");
	return r ? 1 : 0;
}

static int erase_main(int argc, char **argv) {
	puts("Erasing nvram...");
	fflush(stdout);
	return eval("mtd-erase", "-d", "nvram");
}

/*
 * Find nvram param name; return pointer which should be treated as const
 * return NULL if not found.
 *
 * NOTE:  This routine special-cases the variable wl_bss_enabled.  It will
 * return the normal default value if asked for wl_ or wl0_.  But it will
 * return 0 if asked for a virtual BSS reference like wl0.1_.
 */
static const char *get_default_value(const char *name) {
	char *p;
	const defaults_t *t;
	char fixed_name[NVRAM_MAX_PARAM_LEN + 1];

	if (strncmp(name, "wl", 2) == 0 && isdigit(name[2]) && ((p = strchr(name, '_'))))
		snprintf(fixed_name, sizeof(fixed_name) - 1, "wl%s", p);
	else strncpy(fixed_name, name, sizeof(fixed_name));

	if (strcmp(fixed_name, "wl_bss_enabled") == 0)
		if (name[3] == '.' || name[4] == '.') /* Virtual interface */
			return "0";

	for (t = defaults; t->key; t++)
		if (strcmp(t->key, name) == 0 || strcmp(t->key, fixed_name) == 0)
			return (default_value_for_model(t) ? : "");

	return NULL;
}

static int default_get_main(int argc, char **argv) {
	char *p;

	if ((p = (char *)get_default_value(argv[1])) != NULL) {
		puts(p);
		return 0;
	}
	return 1;
}

#define X_QUOTE		0
#define X_SET		1
#define X_C		2
#define X_TAB		3

static int export_main(int argc, char **argv) {
	char *p;
	char buffer[NVRAM_SPACE];
	int eq;
	int mode;
	int all, n, skip;
	char *bk, *bv, *v;

	// C, set, quote
	static const char *start[4] = { "\"", "nvram set ", "{ \"", "" };
	static const char *stop[4] = { "\"", "\"", "\" },", "" };


	getall(buffer);
	p = buffer;

	all = 1;
	for (n = 1; n < argc; ++n)
		if (strcmp(argv[n], "--nodefaults") == 0) {
			if (argc < 3) help("export");
			all = 0;
			if (n == 1) ++argv;
			break;
		}

	if (strcmp(argv[1], "--dump") == 0) {
		if (!all) help("export");
		for (p = buffer; *p; p += strlen(p) + 1)
			puts(p);
		return 0;
	}
	if (strcmp(argv[1], "--dump0") == 0) {
		if (!all) help("export");
		for (p = buffer; *p; p += strlen(p) + 1) { }
		fwrite(buffer, p - buffer, 1, stdout);
		return 0;
	}

	if (strcmp(argv[1], "--c") == 0) mode = X_C;
	else if (strcmp(argv[1], "--set") == 0) mode = X_SET;
	else if (strcmp(argv[1], "--tab") == 0) mode = X_TAB;
	else if (strcmp(argv[1], "--quote") == 0) mode = X_QUOTE;
	else help("export");

	while (*p) {
		eq = 0;

		if (!all) {
			skip = 0;
			bk = p;
			p += strlen(p) + 1;
			bv = strchr(bk, '=');
			*bv++ = 0;

			if ((v = (char *)get_default_value(bk)) != NULL)
				skip = (strcmp(bv, v) == 0);

			*(bv - 1) = '=';
			if (skip) continue;
			else p = bk;
		}

		printf("%s", start[mode]);
		do {
			switch (*p) {
			case 9:
				if (mode == X_SET) putchar(*p);
				else printf("\\t");
				break;
			case 13:
				if (mode == X_SET) putchar(*p);
				else printf("\\r");
				break;
			case 10:
				if (mode == X_SET) putchar(*p);
				else printf("\\n");
				break;
			case '"':
			case '\\':
				printf("\\%c", *p);
				break;
			case '$':
			case '`':
				if (mode != X_SET) putchar(*p);
				else printf("\\%c", *p);
				break;
			case '=':
				if ((eq == 0) && (mode > X_QUOTE)) {
					printf((mode == X_C) ? "\", \"" :
						((mode == X_SET) ? "=\"" : "\t"));
					eq = 1;
					break;
				}
				eq = 1;
			default:
				if (!isprint(*p)) printf("\\x%02x", *p);
				else putchar(*p);
				break;
			}
			++p;
		} while (*p);
		printf("%s\n", stop[mode]);
		++p;
	}
	return 0;
}

/*
 * Check if nvram variable is used in this firmware version, using defined
 * defaults as a list of used variables in this firmware build.
 * Used to skip unused (old, used-defined, from different firmware etc.)
 * variables during import or restore operations, if --forceall not used.
 *
 * NOTE: This routine special-cases the variable rruleNN (NN > 0).
 * It will return 1 even if not defined in defaults.
 */
static int used_in_this_version(const char *key) {
	const defaults_t *t;
	int n;

	for (t = defaults; t->key; t++)
		if (!strcmp(t->key, key)) return 1;

	if ((strncmp(key, "rrule", 5) == 0) && ((n = atoi(key + 5)) > 0) && (n < 50)) return 1;
	return 0;
}

static int import_main(int argc, char **argv) {
	FILE *f;
	char s[10240];
	int n;
	char *k, *v;
	char *p, *q;
	int forceall = 0;
	int same, skip, set;

	if (strcmp(argv[1], "--forceall") == 0) {
		forceall = 1;
		++argv;
	}

	if ((f = fopen(argv[1], "r")) == NULL) reterr("Error opening file\n");

	same = skip = set = 0;
	while (fgets(s, sizeof(s), f) != NULL) {
		n = strlen(s);
		while ((--n > 0) && (isspace(s[n]))) ;
		if ((n <= 0) || (s[n] != '"')) continue;
		s[n] = 0;

		k = s;
		while (isspace(*k)) ++k;
		if (*k != '"') continue;
		++k;

		if ((v = strchr(k, '=')) == NULL) continue;
		*v++ = 0;

		p = q = v;
		while (*p) {
			if (*p == '\\') {
				++p;
				switch (*p) {
				case 't':
					*q++ = '\t';
					break;
				case 'r':
					*q++ = '\n';
					break;
				case 'n':
					*q++ = '\n';
					break;
				case '\\':
				case '"':
					*q++ = *p;
					break;
				default:
					reterr("Error unescaping %s=%s\n", k, v);
				}
			} else *q++ = *p;
			++p;
		}
		*q = 0;

		if (forceall || used_in_this_version(k)) {
			if (nvram_match(k, v)) {
				++same;
//				printf("SAME: %s=%s\n", k, v);
			}
			else {
				++set;
				printf("%s=%s\n", k, v);
				nvram_set(k, v);
			}
		} else {
			++skip;
//			printf("SKIP: %s=%s\n", k, v);
		}
	}

	fclose(f);

	printf("---\n%d skipped, %d same, %d set\n", skip, same, set);
	return 0;
}

#define V1	0x31464354L

typedef struct {
	unsigned long sig;
	unsigned long hwid;
	char buffer[NVRAM_SPACE];
} backup_t;

static int backup_main(int argc, char **argv) {
	backup_t data;
	unsigned int size;
	char *p;
	char s[512];
	char tmp[128];
	int r;

	getall(data.buffer);

	data.sig = V1;
	data.hwid = check_hw_type();

	p = data.buffer;
	while (*p != 0) p += strlen(p) + 1;

	size = (sizeof(data) - sizeof(data.buffer)) + (p - data.buffer) + 1;

	strcpy(tmp, "/tmp/nvramXXXXXX");
	mktemp(tmp);
	if (f_write(tmp, &data, size, 0, 0) != size) reterr("Error saving file\n");
	sprintf(s, "gzip < %s > %s", tmp, argv[1]);
	r = system(s);
	unlink(tmp);

	if (r != 0) {
		unlink(argv[1]);
		reterr("Error compressing file\n");
	}

	puts("Saved");
	return 0;
}

static int restore_main(int argc, char **argv) {
	char *name;
	int test;
	int force, forceall;
	int commit;
	backup_t data;
	unsigned int size;
	char s[512];
	char tmp[128];
	unsigned long hw;
	char current[NVRAM_SPACE];
	char *b, *bk, *bv;
	char *c, *ck, *cv;
	int nset;
	int nunset;
	int nsame;
	int cmp;
	int i;

	test = 0;
	force = forceall = 0;
	commit = 1;
	name = NULL;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "--test") == 0) test = 1;
			else if (strcmp(argv[i], "--force") == 0) force = 1;
			else if (strcmp(argv[i], "--forceall") == 0) forceall = 1;
			else if (strcmp(argv[i], "--nocommit") == 0) commit = 0;
			else help(*argv);
		} else name = argv[i];
	}
	if (!name) help(*argv);

	strcpy(tmp, "/tmp/nvramXXXXXX");
	mktemp(tmp);
	sprintf(s, "gzip -d < %s > %s", name, tmp);
	if (system(s) != 0) {
		unlink(tmp);
		reterr("Error decompressing file\n");
	}

	size = f_size(tmp);
	if ((size <= (sizeof(data) - sizeof(data.buffer))) || (size > sizeof(data))
		|| (f_read(tmp, &data, sizeof(data)) != size)) {
		unlink(tmp);
		reterr("Invalid data size or read error\n");
	}

	unlink(tmp);

	if (data.sig != V1) reterr("Invalid signature: %08lX / %08lX\n", data.sig, V1);

	hw = check_hw_type();
	if ((data.hwid != hw) && (!force)) reterr("Invalid hardware type: %08lX / %08lX\n", data.hwid, hw);

	// 1 - check data

	size -= sizeof(data) - sizeof(data.buffer);
	if ((data.buffer[size - 1] != 0) || (data.buffer[size - 2] != 0))
CORRUPT:
		reterr("Corrupted data area\n");

	b = data.buffer;
	while (*b) {
		bk = b;
		b += strlen(b) + 1;
		if ((bv = strchr(bk, '=')) == NULL)
			goto CORRUPT;
		*bv = 0;
		if ((strcmp(bk, "et0macaddr") == 0) && !nvram_match(bk, bv + 1) && !force)
			reterr("Cannot restore on a different router\n");
		*bv = '=';
	}
	if (((b - data.buffer) + 1) != size) reterr("Extra data found at the end\n");


	// 2 - set

	if (!test) {
		if (!wait_action_idle(10)) reterr("System busy\n");
		set_action(ACT_SW_RESTORE);
		led(LED_DIAG, 1);
	}

	nset = nunset = nsame = 0;

	b = data.buffer;
	while (*b) {
		bk = b;
		b += strlen(b) + 1;
		bv = strchr(bk, '=');
		*bv++ = 0;

		if (forceall || used_in_this_version(bk)) {
			if (!nvram_match(bk, bv)) {
				if (test) printf("nvram set \"%s=%s\"\n", bk, bv);
				else nvram_set(bk, bv);
				++nset;
			} else ++nsame;
		}

		*(bv - 1) = '=';
	}


	// 3 - unset

	getall(current);
	c = current;
	while (*c) {
		ck = c;
		c += strlen(c) + 1;
		if ((cv = strchr(ck, '=')) == NULL) {
			fprintf(stderr, "Invalid data in NVRAM: %s", ck);
			continue;
		}
		*cv++ = 0;

		if (forceall || used_in_this_version(ck)) {
			cmp = 1;
			b = data.buffer;
			while (*b) {
				bk = b;
				b += strlen(b) + 1;
				bv = strchr(bk, '=');
				*bv++ = 0;
				cmp = strcmp(bk, ck);
				*(bv - 1) = '=';
				if (cmp == 0) break;
			}
			if (cmp != 0) {
				++nunset;
				if (test) printf("nvram unset \"%s\"\n", ck);
				else nvram_unset(ck);
			}
		}
	}

	if (!nset && !nunset) commit = 0;
	printf("\nPerformed %d set and %d unset operations. %d required no changes\n%s\n",
		nset, nunset, nsame, commit ? "Committing..." : "Not commiting");
	fflush(stdout);

	if (!test) {
		set_action(ACT_IDLE);
		if (commit) return nvram_commit();
	}
	return 0;
}

#if 0
static int test_main(int argc, char **argv) {
/*
	static const char *extra[] = {
		"clkfreq", "pa0b0", "pa0b1", "pa0b2", "pa0itssit", "pa0maxpwr",
		"sdram_config", "sdram_init", "sdram_ncdl", "vlan0ports", NULL };
	const char **x;
*/
	char buffer[NVRAM_SPACE];
	char *k, *v, *e;
	const defaults_t *rest;

	printf("Unknown keys:\n");

	getall(buffer);
	k = buffer;
	while (*k) {
		e = k + strlen(k) + 1;
		if ((v = strchr(k, '=')) != NULL) {
			*v = 0;
			for (rest = defaults; rest->key; ++rest) {
				if (strcmp(k, rest->key) == 0) break;
			}
			if (rest->key == NULL) {
				if (strcmp(k, "wl0_ifname") && strcmp(k, "wl_ifname")) {
					printf("%s=%s\n", k, v + 1);
/*
					for (x = extra; *x; ++x) {
						if (strcmp(k, *x) == 0) break;
					}
					if (*x == NULL) {
						printf("nvram unset \"%s\"\n", k);
					}
*/
				}
			}
		} else printf("WARNING: '%s' doesn't have a '=' delimiter\n", k);
		k = e;
	}

	return 0;
}
#endif

static int setfb64_main(int argc, char **argv) {
	if (!nvram_set_file(argv[1], argv[2], 10240))
		reterr("Unable to set %s or read %s\n", argv[1], argv[2]);
	return 0;
}

static int getfb64_main(int argc, char **argv) {
	if (!nvram_get_file(argv[1], argv[2], 10240))
		reterr(stderr, "Unable to get %s or write %s\n", argv[1], argv[2]);
	return 0;
}

// -----------------------------------------------------------------------------

typedef struct {
	const char *name;
	int args;
	int (*main)(int argc, char *argv[]);
	const char *usage;
} applets_t;

static const applets_t applets[] = {
	{ "set",		3,	set_main,		"<key=value>" },
	{ "get",		3,	get_main,		"<key>" },
	{ "unset",		3,	unset_main,		"<key>" },
	{ "ren",		4,	ren_main,		"<key_old> <key_new>" },
	{ "commit",		2,	commit_main,		"" },
	{ "show",		-2,	show_main,		"[--nosort] [--nostat] [[--ignore-case] --find <text>]" },
	{ "find",		-3,	find_main,		"[--nosort] [--ignore-case] <text>" },
	{ "erase",		2,	erase_main,		"" },
	{ "defaults",		3,	defaults_main,		"<--yes|--initcheck>" },
	{ "export",		-3,	export_main,		"<--quote|--c|--set|--tab> [--nodefaults]\n<--dump|--dump0>" },
	{ "import",		-3,	import_main,		"[--forceall] <filename>" },
	{ "backup",		3,	backup_main,		"<filename>" },
	{ "restore",		-3,	restore_main,		"<filename> [--test] [--force] [--forceall] [--nocommit]" },
	{ "setfb64",		4,	setfb64_main,		"<key> <filename>" },
	{ "getfb64",		4,	getfb64_main,		"<key> <filename>" },
	{ "setfile",		4,	f2n_main,		"<key> <filename>" },
	{ "getfile",		4,	n2f_main,		"<key> <filename>" },
	{ "setfile2nvram",	3,	save2f_main,		"<filename>" },
	{ "default_get",	3,	default_get_main,	"<key>" },
	{ "validate",		-3,	validate_main,		"[--restore] [--wl<N>]" },
//	{ "test",		2,	test_main,		NULL },
	{ NULL, 		0,	NULL,			NULL }
};

static const char *exec_name;

static void help(const char const *applet) {
	const applets_t *a;
	char *t, *p;

	puts(	"NVRAM Utility\n"
		"Copyright (C) 2006-2009 Jonathan Zarate\n\n"
		"Usage:");
	for (a = applets; a->name; ++a)
		if ((!applet || !strcmp(applet, a->name)) && a->usage) {
			for (t = strdup(a->usage), p = strtok(t, "\n"); p; p = strtok(NULL, "\n"))
				printf("  %s %s %s\n", exec_name, a->name, p);
			free(t);
		}
	exit(1);
}

int main(int argc, char **argv) {
	const applets_t *a;

	if ((exec_name = strrchr(*argv, '/')) != NULL) ++exec_name;
	else exec_name = *argv;

	if (argc >= 2)
		for (a = applets; a->name; ++a)
			if (!strcmp(argv[1], a->name)) {
				if ((argc != a->args) && ((a->args > 0) || (argc < -(a->args)))) help(a->name);
				return a->main(argc-1, argv+1);
			}
	help(NULL);
}
