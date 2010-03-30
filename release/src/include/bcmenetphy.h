/*
 * Misc Broadcom BCM47XX MDC/MDIO enet phy definitions.
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 * $Id$
 */

#ifndef	_bcmenetphy_h_
#define	_bcmenetphy_h_

/* phy address */
#define	MAXEPHY		32			/* mdio phy addresses are 5bit quantities */
#define	EPHY_MASK	0x1f			/* phy mask */
#define	EPHY_NONE	31			/* nvram: no phy present at all */
#define	EPHY_NOREG	30			/* nvram: no local phy regs */

/* just a few phy registers */
#define	CTL_RESET	(1 << 15)		/* reset */
#define	CTL_LOOP	(1 << 14)		/* loopback */
#define	CTL_SPEED	(1 << 13)		/* speed selection 0=10, 1=100 */
#define	CTL_ANENAB	(1 << 12)		/* autonegotiation enable */
#define	CTL_RESTART	(1 << 9)		/* restart autonegotiation */
#define	CTL_DUPLEX	(1 << 8)		/* duplex mode 0=half, 1=full */

#define	ADV_10FULL	(1 << 6)		/* autonegotiate advertise 10full */
#define	ADV_10HALF	(1 << 5)		/* autonegotiate advertise 10half */
#define	ADV_100FULL	(1 << 8)		/* autonegotiate advertise 100full */
#define	ADV_100HALF	(1 << 7)		/* autonegotiate advertise 100half */

/* link partner ability register */
#define LPA_SLCT	0x001f			/* same as advertise selector */
#define LPA_10HALF	0x0020			/* can do 10mbps half-duplex */
#define LPA_10FULL	0x0040			/* can do 10mbps full-duplex */
#define LPA_100HALF	0x0080			/* can do 100mbps half-duplex */
#define LPA_100FULL	0x0100			/* can do 100mbps full-duplex */
#define LPA_100BASE4	0x0200			/* can do 100mbps 4k packets */
#define LPA_RESV	0x1c00			/* unused */
#define LPA_RFAULT	0x2000			/* link partner faulted */
#define LPA_LPACK	0x4000			/* link partner acked us */
#define LPA_NPAGE	0x8000			/* next page bit */

#define LPA_DUPLEX	(LPA_10FULL | LPA_100FULL)
#define LPA_100		(LPA_100FULL | LPA_100HALF | LPA_100BASE4)

#define	STAT_REMFAULT	(1 << 4)		/* remote fault */
#define	STAT_LINK	(1 << 2)		/* link status */
#define	STAT_JAB	(1 << 1)		/* jabber detected */
#define	AUX_FORCED	(1 << 2)		/* forced 10/100 */
#define	AUX_SPEED	(1 << 1)		/* speed 0=10mbps 1=100mbps */
#define	AUX_DUPLEX	(1 << 0)		/* duplex 0=half 1=full */

#endif	/* _bcmenetphy_h_ */
