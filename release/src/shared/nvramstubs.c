/*
 * Stubs for NVRAM functions for platforms without flash
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmutils.h>
#undef strcmp
#define strcmp(s1,s2)	0	/* always match */
#include <bcmnvram.h>

int
nvram_init(void *sbh)
{
	return 0;
}

int
nvram_append(void *sb, char *vars, uint varsz)
{
	return 0;
}

void
nvram_exit(void *sbh)
{
}

char *
nvram_get(const char *name)
{
	return (char *) 0;
}

int
nvram_set(const char *name, const char *value)
{
	return 0;
}

int
nvram_unset(const char *name)
{
	return 0;
}

int
nvram_commit(void)
{
	return 0;
}

int
nvram_getall(char *buf, int count)
{
	/* add null string as terminator */
	if (count < 1)
		return BCME_BUFTOOSHORT;
	*buf = '\0';
	return 0;
}
