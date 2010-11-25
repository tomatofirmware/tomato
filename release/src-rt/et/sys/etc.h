/*
 * Common [OS-independent] header file for
 * Broadcom BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: etc.h,v 1.66.96.4 2009/07/07 19:25:16 Exp $
 */

#ifndef _etc_h_
#define _etc_h_

#include <etioctl.h>
#include <proto/802.1d.h>

#define	MAXMULTILIST	32

#ifndef ch_t
#define	ch_t void
#endif

#define NUMTXQ		4

#define TXREC_THR	8

struct etc_info;	/* forward declaration */
struct bcmstrbuf;	/* forward declaration */

/* each chip type supports a set of chip-type-specific ops */
struct chops {
	bool (*id)(uint vendor, uint device);		/* return true if match */
	void *(*attach)(struct etc_info *etc, void *dev, void *regs);
	void (*detach)(ch_t *ch);			/* free chip private state */
	void (*reset)(ch_t *ch);			/* chip reset */
	void (*init)(ch_t *ch, uint options);		/* chip init */
	bool (*tx)(ch_t *ch, void *p);			/* transmit frame */
	void *(*rx)(ch_t *ch);				/* receive frame */
	void (*rxfill)(ch_t *ch);			/* post dma rx buffers */
	int (*getintrevents)(ch_t *ch, bool in_isr);	/* return intr events */
	bool (*errors)(ch_t *ch);			/* handle chip errors */
	void (*intrson)(ch_t *ch);			/* enable chip interrupts */
	void (*intrsoff)(ch_t *ch);			/* disable chip interrupts */
	void (*txreclaim)(ch_t *ch, bool all);		/* reclaim transmit resources */
	void (*rxreclaim)(ch_t *ch);			/* reclaim receive resources */
	void (*statsupd)(ch_t *ch);			/* update sw stat counters */
	void (*dumpmib)(ch_t *ch, struct bcmstrbuf *);	/* get sw mib counters */
	void (*enablepme)(ch_t *ch);			/* enable PME */
	void (*disablepme)(ch_t *ch);			/* disable PME */
	void (*phyreset)(ch_t *ch, uint phyaddr);	/* reset phy */
	uint16 (*phyrd)(ch_t *ch, uint phyaddr, uint reg);	/* read phy register */
	void (*phywr)(ch_t *ch, uint phyaddr, uint reg, uint16 val);	/* write phy register */
	void (*dump)(ch_t *ch, struct bcmstrbuf *b);		/* debugging output */
	void (*longname)(ch_t *ch, char *buf, uint bufsize);	/* return descriptive name */
	void (*duplexupd)(ch_t *ch);			/* keep mac duplex consistent */
};

/*
 * "Common" os-independent software state structure.
 */
typedef struct etc_info {
	void		*et;		/* pointer to os-specific private state */
	uint		unit;		/* device instance number */
	void 		*osh; 		/* pointer to os handler */
	void		*mib;		/* pointer to s/w maintained mib counters */
	bool		up;		/* interface up and running */
	bool		promisc;	/* promiscuous destination address */
	bool		qos;		/* QoS priority determination on rx */
	bool		loopbk;		/* loopback override mode */

	int		forcespeed;	/* disable autonegotiation and force speed/duplex */
	uint		advertise;	/* control speed/duplex advertised caps */
	uint		advertise2;	/* control gige speed/duplex advertised caps */
	bool		needautoneg;	/* request restart autonegotiation */
	int		speed;		/* current speed: 10, 100 */
	int		duplex;		/* current duplex: 0=half, 1=full */

	bool		piomode;	/* enable programmed io (!dma) */
	void		*pioactive;	/* points to pio packet being transmitted */
	volatile uint	*txavail[NUMTXQ];	/* dma: # tx descriptors available */

	uint16		vendorid;	/* pci function vendor id */
	uint16		deviceid;	/* pci function device id */
	uint		chip;		/* chip number */
	uint		chiprev;	/* chip revision */
	uint		coreid;		/* core id */
	uint		corerev;	/* core revision */

	bool		nicmode;	/* is this core using its own pci i/f */

	struct chops	*chops;		/* pointer to chip-specific opsvec */
	void		*ch;		/* pointer to chip-specific state */
	void		*robo;		/* optional robo private data */

	uint		txq_state;	/* tx queues state bits */
	uint		coreunit;	/* sb chips: chip enet instance # */
	uint		phyaddr;	/* sb chips: mdio 5-bit phy address */
	uint		mdcport;	/* sb chips: which mii to use (enet core #) to access phy */

	struct ether_addr cur_etheraddr; /* our local ethernet address */
	struct ether_addr perm_etheraddr; /* original sprom local ethernet address */

	struct ether_addr multicast[MAXMULTILIST];
	uint		nmulticast;
	bool		allmulti;	/* enable all multicasts */

	bool		linkstate;	/* link integrity state */
	bool		pm_modechange;	/* true if mode change is to due pm */

	uint32		now;		/* elapsed seconds */

	uint32		boardflags;	/* board flags */
	uint32		txrec_thresh;	/* # of tx frames after which reclaim is done */

	/* sw-maintained stat counters */
	uint32		txframes[NUMTXQ];	/* transmitted frames on each tx fifo */
	uint32		txframe;	/* transmitted frames */
	uint32		txbyte;		/* transmitted bytes */
	uint32		rxframe;	/* received frames */
	uint32		rxbyte;		/* received bytes */
	uint32		txerror;	/* total tx errors */
	uint32		txnobuf;	/* tx out-of-buffer errors */
	uint32		rxerror;	/* total rx errors */
	uint32		rxgiants;	/* total rx giant frames */
	uint32		rxnobuf;	/* rx out-of-buffer errors */
	uint32		reset;		/* reset count */
	uint32		dmade;		/* pci descriptor errors */
	uint32		dmada;		/* pci data errors */
	uint32		dmape;		/* descriptor protocol error */
	uint32		rxdmauflo;	/* receive descriptor underflow */
	uint32		rxoflo;		/* receive fifo overflow */
	uint32		txuflo;		/* transmit fifo underflow */
	uint32		rxoflodiscards;	/* frames discarded during rx fifo overflow */
	uint32		rxbadlen;	/* 802.3 len field != read length */
} etc_info_t;

/* interrupt event bitvec */
#define	INTR_TX		0x1
#define	INTR_RX		0x2
#define	INTR_ERROR	0x4
#define	INTR_TO		0x8
#define	INTR_NEW	0x10

/* forcespeed values */
#define	ET_AUTO		-1
#define	ET_10HALF	0
#define	ET_10FULL	1
#define	ET_100HALF	2
#define	ET_100FULL	3
#define	ET_1000HALF	4
#define	ET_1000FULL	5

/* init options */
#define ET_INIT_FULL     0x1
#define ET_INIT_INTRON   0x2

/* Specific init options for et_init */
#define ET_INIT_DEF_OPTIONS   (ET_INIT_FULL | ET_INIT_INTRON)
#define ET_INIT_INTROFF       (ET_INIT_FULL)
#define ET_INIT_PARTIAL      (0)

/* macro to safely clear the UP flag */
#define ET_FLAG_DOWN(x)   (*(x)->chops->intrsoff)((x)->ch);  \
			  (x)->up = FALSE;

/*
 * Least-common denominator rxbuf start-of-data offset:
 * Must be >= size of largest rxhdr
 * Must be 2-mod-4 aligned so IP is 0-mod-4
 */
#define	HWRXOFF		30

#define TC_BK		0	/* background traffic class */
#define TC_BE		1	/* best effort traffic class */
#define TC_CL		2	/* controlled load traffic class */
#define TC_VO		3	/* voice traffic class */
#define TC_NONE		-1	/* traffic class none */

#define RX_Q0		0	/* receive DMA queue */
#define NUMRXQ		1	/* gmac has one rx queue */

#define TX_Q0		TC_BK	/* DMA txq 0 */
#define TX_Q1		TC_BE	/* DMA txq 1 */
#define TX_Q2		TC_CL	/* DMA txq 2 */
#define TX_Q3		TC_VO	/* DMA txq 3 */

static inline uint32
etc_up2tc(uint32 up)
{
	extern uint32 up2tc[];

	return (up2tc[(up) & MAXPRIO]);
}

static inline uint32
etc_priq(uint32 txq_state)
{
	extern uint32 priq_selector[];

	return (priq_selector[txq_state]);
}

/* rx header flags bits */
#define RXH_FLAGS(etc, rxh) (((etc)->coreid == GMAC_CORE_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & (GRXF_CRC | GRXF_OVF | GRXF_OVERSIZE)) : \
	(ltoh16(((bcmenetrxh_t *)(rxh))->flags) & (RXF_NO | RXF_RXER | RXF_CRC | RXF_OV)))

#define RXH_OVERSIZE(etc, rxh) (((etc)->coreid == GMAC_CORE_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & GRXF_OVERSIZE) : FALSE)

#define RXH_CRC(etc, rxh) (((etc)->coreid == GMAC_CORE_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & GRXF_CRC) : \
	(ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_CRC))

#define RXH_OVF(etc, rxh) (((etc)->coreid == GMAC_CORE_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & GRXF_OVF) : \
	(ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_OV))

#define RXH_RXER(etc, rxh) (((etc)->coreid == GMAC_CORE_ID) ? \
	FALSE : (ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_RXER))

#define RXH_NO(etc, rxh) (((etc)->coreid == GMAC_CORE_ID) ? \
	FALSE : (ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_NO))

#ifdef	CFG_GMAC
#define ET_GMAC(etc)	((etc)->coreid == GMAC_CORE_ID)
#else
#define ET_GMAC(etc)	(0)
#endif	/* CFG_GMAC */

/* exported prototypes */
extern struct chops *etc_chipmatch(uint vendor, uint device);
extern void *etc_attach(void *et, uint vendor, uint device, uint unit, void *dev, void *regsva);
extern void etc_detach(etc_info_t *etc);
extern void etc_reset(etc_info_t *etc);
extern void etc_init(etc_info_t *etc, uint options);
extern void etc_up(etc_info_t *etc);
extern uint etc_down(etc_info_t *etc, int reset);
extern int etc_ioctl(etc_info_t *etc, int cmd, void *arg);
extern int etc_iovar(etc_info_t *etc, uint cmd, uint set, void *arg);
extern void etc_promisc(etc_info_t *etc, uint on);
extern void etc_qos(etc_info_t *etc, uint on);
extern void etc_dump(etc_info_t *etc, struct bcmstrbuf *b);
extern void etc_watchdog(etc_info_t *etc);
extern uint etc_totlen(etc_info_t *etc, void *p);

#endif	/* _etc_h_ */
