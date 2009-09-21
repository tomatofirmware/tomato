/* vi: set sw=4 ts=4: */
/*
 * Support functions for mounting devices by label/uuid
 *
 * Copyright (C) 2006 by Jason Schoon <floydpink@gmail.com>
 * Some portions cribbed from e2fsprogs, util-linux, dosfstools
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "volume_id_internal.h"

//#define BLKGETSIZE64 _IOR(0x12,114,size_t)

static struct uuidCache_s {
	struct uuidCache_s *next;
//	int major, minor;
	char *device;
	char *label;
	char *uc_uuid; /* prefix makes it easier to grep for */
} *uuidCache;

/* Returns !0 on error.
 * Otherwise, returns malloc'ed strings for label and uuid
 * (and they can't be NULL, although they can be "").
 * NB: closes fd. */
static int
get_label_uuid(int fd, char **label, char **uuid)
{
	int rv = 1;
	uint64_t size;
	struct volume_id *vid;

	/* fd is owned by vid now */
	vid = volume_id_open_node(fd);

	if (ioctl(/*vid->*/fd, BLKGETSIZE64, &size) != 0)
		size = 0;

	if (volume_id_probe_all(vid, /*0,*/ size) != 0)
		goto ret;

	if (vid->label[0] != '\0' || vid->uuid[0] != '\0') {
		*label = xstrndup(vid->label, sizeof(vid->label));
		*uuid  = xstrndup(vid->uuid, sizeof(vid->uuid));
		dbg("found label '%s', uuid '%s' on %s", *label, *uuid, device);
		rv = 0;
	}
 ret:
	free_volume_id(vid); /* also closes fd */
	return rv;
}

/* NB: we take ownership of (malloc'ed) label and uuid */
static void
uuidcache_addentry(char *device, /*int major, int minor,*/ char *label, char *uuid)
{
	struct uuidCache_s *last;

	if (!uuidCache) {
		last = uuidCache = xzalloc(sizeof(*uuidCache));
	} else {
		for (last = uuidCache; last->next; last = last->next)
			continue;
		last->next = xzalloc(sizeof(*uuidCache));
		last = last->next;
	}
	/*last->next = NULL; - xzalloc did it*/
//	last->major = major;
//	last->minor = minor;
	last->device = device;
	last->label = label;
	last->uc_uuid = uuid;
}

/* If get_label_uuid() on device_name returns success,
 * add a cache entry for this device.
 * If device node does not exist, it will be temporarily created. */
static int FAST_FUNC
uuidcache_check_device(const char *device,
		struct stat *statbuf,
		void *userData UNUSED_PARAM,
		int depth UNUSED_PARAM)
{
	char *uuid = uuid; /* for compiler */
	char *label = label;
	int fd;

	if (!S_ISBLK(statbuf->st_mode))
		return TRUE;

	fd = open(device, O_RDONLY);
	if (fd < 0)
		return TRUE;

	/* get_label_uuid() closes fd in all cases (success & failure) */
	if (get_label_uuid(fd, &label, &uuid) == 0) {
		/* uuidcache_addentry() takes ownership of all three params */
		uuidcache_addentry(xstrdup(device), /*ma, mi,*/ label, uuid);
	}
	return TRUE;
}

static void
uuidcache_init(void)
{
	if (uuidCache)
		return;

	/* We were scanning /proc/partitions
	 * and /proc/sys/dev/cdrom/info here.
	 * Missed volume managers. I see that "standard" blkid uses these:
	 * /dev/mapper/control
	 * /proc/devices
	 * /proc/evms/volumes
	 * /proc/lvm/VGs
	 * This is unacceptably complex. Let's just scan /dev.
	 * (Maybe add scanning of /sys/block/XXX/dev for devices
	 * somehow not having their /dev/XXX entries created?) */

	recursive_action("/dev", ACTION_RECURSE,
		uuidcache_check_device, /* file_action */
		NULL, /* dir_action */
		NULL, /* userData */
		0 /* depth */);
}

#define UUID   1
#define VOL    2

#ifdef UNUSED
static char *
get_spec_by_x(int n, const char *t, int *majorPtr, int *minorPtr)
{
	struct uuidCache_s *uc;

	uuidcache_init();
	uc = uuidCache;

	while (uc) {
		switch (n) {
		case UUID:
			if (strcmp(t, uc->uc_uuid) == 0) {
				*majorPtr = uc->major;
				*minorPtr = uc->minor;
				return uc->device;
			}
			break;
		case VOL:
			if (strcmp(t, uc->label) == 0) {
				*majorPtr = uc->major;
				*minorPtr = uc->minor;
				return uc->device;
			}
			break;
		}
		uc = uc->next;
	}
	return NULL;
}

static unsigned char
fromhex(char c)
{
	if (isdigit(c))
		return (c - '0');
	return ((c|0x20) - 'a' + 10);
}

static char *
get_spec_by_uuid(const char *s, int *major, int *minor)
{
	unsigned char uuid[16];
	int i;

	if (strlen(s) != 36 || s[8] != '-' || s[13] != '-'
	 || s[18] != '-' || s[23] != '-'
	) {
		goto bad_uuid;
	}
	for (i = 0; i < 16; i++) {
		if (*s == '-')
			s++;
		if (!isxdigit(s[0]) || !isxdigit(s[1]))
			goto bad_uuid;
		uuid[i] = ((fromhex(s[0]) << 4) | fromhex(s[1]));
		s += 2;
	}
	return get_spec_by_x(UUID, (char *)uuid, major, minor);

 bad_uuid:
	fprintf(stderr, _("mount: bad UUID"));
	return 0;
}

static char *
get_spec_by_volume_label(const char *s, int *major, int *minor)
{
	return get_spec_by_x(VOL, s, major, minor);
}
#endif // UNUSED

/* Used by blkid */
void display_uuid_cache(void)
{
	struct uuidCache_s *u;

	uuidcache_init();
	u = uuidCache;
	while (u) {
		printf("%s:", u->device);
		if (u->label[0])
			printf(" LABEL=\"%s\"", u->label);
		if (u->uc_uuid[0])
			printf(" UUID=\"%s\"", u->uc_uuid);
		bb_putchar('\n');
		u = u->next;
	}
}

/* Used by mount and findfs */

char *get_devname_from_label(const char *spec)
{
	struct uuidCache_s *uc;

	uuidcache_init();
	uc = uuidCache;
	while (uc) {
		if (uc->label[0] && strcmp(spec, uc->label) == 0) {
			return xstrdup(uc->device);
		}
		uc = uc->next;
	}
	return NULL;
}

char *get_devname_from_uuid(const char *spec)
{
	struct uuidCache_s *uc;

	uuidcache_init();
	uc = uuidCache;
	while (uc) {
		/* case of hex numbers doesn't matter */
		if (strcasecmp(spec, uc->uc_uuid) == 0) {
			return xstrdup(uc->device);
		}
		uc = uc->next;
	}
	return NULL;
}
