/*
 * NDIS Error codes
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */
#ifndef NDIS
#include <bcm_ndis.h>
#endif
#include <typedefs.h>
#include <osl.h>
#include <epivers.h>
#include <bcmutils.h>

#include <ndiserrmap.h>


static NDIS_STATUS ndisbcmerrormap[] =  \
{	NDIS_STATUS_SUCCESS, 			/* 0 */
	NDIS_STATUS_FAILURE,			/* BCME_ERROR */
	NDIS_STATUS_INVALID_DATA,		/* BCME_BADARG */
	NDIS_STATUS_INVALID_DATA,		/* BCME_BADOPTION */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOTUP */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOTDOWN */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOTAP */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOTSTA */
	NDIS_STATUS_INVALID_DATA,		/* BCME_BADKEYIDX */
	NDIS_STATUS_INVALID_DATA,		/* BCME_RADIOOFF */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOTBANDLOCKED */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_NOCLK */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_BADRATESET */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_BADBAND */
	NDIS_STATUS_INVALID_LENGTH,		/* BCME_BUFTOOSHORT */
	NDIS_STATUS_INVALID_LENGTH,		/* BCME_BUFTOOLONG */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_BUSY */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_NOTASSOCIATED */
	NDIS_STATUS_INVALID_LENGTH, 		/* BCME_BADSSIDLEN */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_OUTOFRANGECHAN */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_BADCHAN */
	NDIS_STATUS_INVALID_DATA, 		/* BCME_BADADDR */
	NDIS_STATUS_RESOURCES, 			/* BCME_NORESOURCE */
	NDIS_STATUS_NOT_SUPPORTED,		/* BCME_UNSUPPORTED */
	NDIS_STATUS_INVALID_LENGTH,		/* BCME_BADLENGTH */
	NDIS_STATUS_ADAPTER_NOT_READY,		/* BCME_NOTREADY */
	NDIS_STATUS_FAILURE,			/* BCME_NOTPERMITTED */
	NDIS_STATUS_RESOURCES, 			/* BCME_NOMEM */
	NDIS_STATUS_INVALID_DATA,		/* BCME_ASSOCIATED */
	NDIS_STATUS_INVALID_DATA,		/* BCME_RANGE */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOTFOUND */
	NDIS_STATUS_INVALID_DATA,		/* BCME_WME_NOT_ENABLED */
	NDIS_STATUS_INVALID_DATA,		/* BCME_TSPEC_NOTFOUND */
	NDIS_STATUS_INVALID_DATA,		/* BCME_ACM_NOTSUPPORTED */
	NDIS_STATUS_INVALID_DATA,		/* BCME_NOT_WME_ASSOCIATION */
	NDIS_STATUS_FAILURE,			/* BCME_SDIO_ERROR */
	NDIS_STATUS_FAILURE,			/* BCME_DONGLE_DOWN */
	NDIS_STATUS_FAILURE				/* BCME_VERSION */

/* When an new error code is added to bcmutils.h, add os 
 * spcecific error translation here as well
 */
/* check if BCME_LAST changed since the last time this function was updated */
#if BCME_LAST != -37
#error "You need to add a OS error translation in the ndisbcmerrormap \
	for new error code defined in bcmuitls.h"
#endif /* BCME_LAST != -37 */
	};

int
ndisstatus2bcmerror(NDIS_STATUS status)
{
	int i, array_size = ARRAYSIZE(ndisbcmerrormap);

	ASSERT(ABS(BCME_LAST) == (array_size - 1));

	for (i = 0; i < array_size; i++)
		if (ndisbcmerrormap[i] == status)
			return -i;

	if (status == NDIS_STATUS_INVALID_OID)
		return BCME_UNSUPPORTED;

	return BCME_ERROR;
}

NDIS_STATUS
bcmerror2ndisstatus(int bcmerror)
{
	if (bcmerror > 0)
		bcmerror = 0;
	else if (bcmerror < BCME_LAST)
		bcmerror = BCME_ERROR;

	/* Array bounds covered by ASSERT in osl_attach */
	return ndisbcmerrormap[-bcmerror];
}
