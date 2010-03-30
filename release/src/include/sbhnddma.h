/*
 * Generic Broadcom Home Networking Division (HND) DMA engine HW interface
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#ifndef	_sbhnddma_h_
#define	_sbhnddma_h_

/* DMA structure:
 *  support two DMA engines: 32 bits address or 64 bit addressing
 *  basic DMA register set is per channel(transmit or receive)
 *  a pair of channels is defined for convenience
 */


/* 32 bits addressing */

/* dma registers per channel(xmt or rcv) */
typedef volatile struct {
	uint32	control;		/* enable, et al */
	uint32	addr;			/* descriptor ring base address (4K aligned) */
	uint32	ptr;			/* last descriptor posted to chip */
	uint32	status;			/* current active descriptor, et al */
} dma32regs_t;

typedef volatile struct {
	dma32regs_t	xmt;		/* dma tx channel */
	dma32regs_t	rcv;		/* dma rx channel */
} dma32regp_t;

typedef volatile struct {	/* diag access */
	uint32	fifoaddr;		/* diag address */
	uint32	fifodatalow;		/* low 32bits of data */
	uint32	fifodatahigh;		/* high 32bits of data */
	uint32	pad;			/* reserved */
} dma32diag_t;

/*
 * DMA Descriptor
 * Descriptors are only read by the hardware, never written back.
 */
typedef volatile struct {
	uint32	ctrl;		/* misc control bits & bufcount */
	uint32	addr;		/* data buffer address */
} dma32dd_t;

/*
 * Each descriptor ring must be 4096byte aligned, and fit within a single 4096byte page.
 */
#define	D32MAXRINGSZ	4096
#define	D32RINGALIGN	4096
#define	D32MAXDD	(D32MAXRINGSZ / sizeof (dma32dd_t))

/* transmit channel control */
#define	XC_XE		((uint32)1 << 0)	/* transmit enable */
#define	XC_SE		((uint32)1 << 1)	/* transmit suspend request */
#define	XC_LE		((uint32)1 << 2)	/* loopback enable */
#define	XC_FL		((uint32)1 << 4)	/* flush request */
#define	XC_AE		((uint32)3 << 16)	/* address extension bits */
#define	XC_AE_SHIFT	16

/* transmit descriptor table pointer */
#define	XP_LD_MASK	0xfff			/* last valid descriptor */

/* transmit channel status */
#define	XS_CD_MASK	0x0fff			/* current descriptor pointer */
#define	XS_XS_MASK	0xf000			/* transmit state */
#define	XS_XS_SHIFT	12
#define	XS_XS_DISABLED	0x0000			/* disabled */
#define	XS_XS_ACTIVE	0x1000			/* active */
#define	XS_XS_IDLE	0x2000			/* idle wait */
#define	XS_XS_STOPPED	0x3000			/* stopped */
#define	XS_XS_SUSP	0x4000			/* suspend pending */
#define	XS_XE_MASK	0xf0000			/* transmit errors */
#define	XS_XE_SHIFT	16
#define	XS_XE_NOERR	0x00000			/* no error */
#define	XS_XE_DPE	0x10000			/* descriptor protocol error */
#define	XS_XE_DFU	0x20000			/* data fifo underrun */
#define	XS_XE_BEBR	0x30000			/* bus error on buffer read */
#define	XS_XE_BEDA	0x40000			/* bus error on descriptor access */
#define	XS_AD_MASK	0xfff00000		/* active descriptor */
#define	XS_AD_SHIFT	20

/* receive channel control */
#define	RC_RE		((uint32)1 << 0)	/* receive enable */
#define	RC_RO_MASK	0xfe			/* receive frame offset */
#define	RC_RO_SHIFT	1
#define	RC_FM		((uint32)1 << 8)	/* direct fifo receive (pio) mode */
#define	RC_AE		((uint32)3 << 16)	/* address extension bits */
#define	RC_AE_SHIFT	16

/* receive descriptor table pointer */
#define	RP_LD_MASK	0xfff			/* last valid descriptor */

/* receive channel status */
#define	RS_CD_MASK	0x0fff			/* current descriptor pointer */
#define	RS_RS_MASK	0xf000			/* receive state */
#define	RS_RS_SHIFT	12
#define	RS_RS_DISABLED	0x0000			/* disabled */
#define	RS_RS_ACTIVE	0x1000			/* active */
#define	RS_RS_IDLE	0x2000			/* idle wait */
#define	RS_RS_STOPPED	0x3000			/* reserved */
#define	RS_RE_MASK	0xf0000			/* receive errors */
#define	RS_RE_SHIFT	16
#define	RS_RE_NOERR	0x00000			/* no error */
#define	RS_RE_DPE	0x10000			/* descriptor protocol error */
#define	RS_RE_DFO	0x20000			/* data fifo overflow */
#define	RS_RE_BEBW	0x30000			/* bus error on buffer write */
#define	RS_RE_BEDA	0x40000			/* bus error on descriptor access */
#define	RS_AD_MASK	0xfff00000		/* active descriptor */
#define	RS_AD_SHIFT	20

/* fifoaddr */
#define	FA_OFF_MASK	0xffff			/* offset */
#define	FA_SEL_MASK	0xf0000			/* select */
#define	FA_SEL_SHIFT	16
#define	FA_SEL_XDD	0x00000			/* transmit dma data */
#define	FA_SEL_XDP	0x10000			/* transmit dma pointers */
#define	FA_SEL_RDD	0x40000			/* receive dma data */
#define	FA_SEL_RDP	0x50000			/* receive dma pointers */
#define	FA_SEL_XFD	0x80000			/* transmit fifo data */
#define	FA_SEL_XFP	0x90000			/* transmit fifo pointers */
#define	FA_SEL_RFD	0xc0000			/* receive fifo data */
#define	FA_SEL_RFP	0xd0000			/* receive fifo pointers */
#define	FA_SEL_RSD	0xe0000			/* receive frame status data */
#define	FA_SEL_RSP	0xf0000			/* receive frame status pointers */

/* descriptor control flags */
#define	CTRL_BC_MASK	0x1fff			/* buffer byte count */
#define	CTRL_AE		((uint32)3 << 16)	/* address extension bits */
#define	CTRL_AE_SHIFT	16
#define	CTRL_EOT	((uint32)1 << 28)	/* end of descriptor table */
#define	CTRL_IOC	((uint32)1 << 29)	/* interrupt on completion */
#define	CTRL_EOF	((uint32)1 << 30)	/* end of frame */
#define	CTRL_SOF	((uint32)1 << 31)	/* start of frame */

/* control flags in the range [27:20] are core-specific and not defined here */
#define	CTRL_CORE_MASK	0x0ff00000

/* 64 bits addressing */

/* dma registers per channel(xmt or rcv) */
typedef volatile struct {
	uint32	control;		/* enable, et al */
	uint32	ptr;			/* last descriptor posted to chip */
	uint32	addrlow;		/* descriptor ring base address low 32-bits (8K aligned) */
	uint32	addrhigh;		/* descriptor ring base address bits 63:32 (8K aligned) */
	uint32	status0;		/* current descriptor, xmt state */
	uint32	status1;		/* active descriptor, xmt error */
} dma64regs_t;

typedef volatile struct {
	dma64regs_t	tx;		/* dma64 tx channel */
	dma64regs_t	rx;		/* dma64 rx channel */
} dma64regp_t;

typedef volatile struct {		/* diag access */
	uint32	fifoaddr;		/* diag address */
	uint32	fifodatalow;		/* low 32bits of data */
	uint32	fifodatahigh;		/* high 32bits of data */
	uint32	pad;			/* reserved */
} dma64diag_t;

/*
 * DMA Descriptor
 * Descriptors are only read by the hardware, never written back.
 */
typedef volatile struct {
	uint32	ctrl1;		/* misc control bits & bufcount */
	uint32	ctrl2;		/* buffer count and address extension */
	uint32	addrlow;	/* memory address of the date buffer, bits 31:0 */
	uint32	addrhigh;	/* memory address of the date buffer, bits 63:32 */
} dma64dd_t;

/*
 * Each descriptor ring must be 8kB aligned, and fit within a contiguous 8kB physical addresss.
 */
#define	D64MAXRINGSZ	8192
#define	D64RINGALIGN	8192
#define	D64MAXDD	(D64MAXRINGSZ / sizeof (dma64dd_t))

/* transmit channel control */
#define	D64_XC_XE		0x00000001	/* transmit enable */
#define	D64_XC_SE		0x00000002	/* transmit suspend request */
#define	D64_XC_LE		0x00000004	/* loopback enable */
#define	D64_XC_FL		0x00000010	/* flush request */
#define	D64_XC_AE		0x00030000	/* address extension bits */
#define	D64_XC_AE_SHIFT		16

/* transmit descriptor table pointer */
#define	D64_XP_LD_MASK		0x00000fff	/* last valid descriptor */

/* transmit channel status */
#define	D64_XS0_CD_MASK		0x00001fff	/* current descriptor pointer */
#define	D64_XS0_XS_MASK		0xf0000000     	/* transmit state */
#define	D64_XS0_XS_SHIFT		28
#define	D64_XS0_XS_DISABLED	0x00000000	/* disabled */
#define	D64_XS0_XS_ACTIVE	0x10000000	/* active */
#define	D64_XS0_XS_IDLE		0x20000000	/* idle wait */
#define	D64_XS0_XS_STOPPED	0x30000000	/* stopped */
#define	D64_XS0_XS_SUSP		0x40000000	/* suspend pending */

#define	D64_XS1_AD_MASK		0x0001ffff	/* active descriptor */
#define	D64_XS1_XE_MASK		0xf0000000     	/* transmit errors */
#define	D64_XS1_XE_SHIFT		28
#define	D64_XS1_XE_NOERR	0x00000000	/* no error */
#define	D64_XS1_XE_DPE		0x10000000	/* descriptor protocol error */
#define	D64_XS1_XE_DFU		0x20000000	/* data fifo underrun */
#define	D64_XS1_XE_DTE		0x30000000	/* data transfer error */
#define	D64_XS1_XE_DESRE	0x40000000	/* descriptor read error */
#define	D64_XS1_XE_COREE	0x50000000	/* core error */

/* receive channel control */
#define	D64_RC_RE		0x00000001	/* receive enable */
#define	D64_RC_RO_MASK		0x000000fe	/* receive frame offset */
#define	D64_RC_RO_SHIFT		1
#define	D64_RC_FM		0x00000100	/* direct fifo receive (pio) mode */
#define	D64_RC_AE		0x00030000	/* address extension bits */
#define	D64_RC_AE_SHIFT		16

/* receive descriptor table pointer */
#define	D64_RP_LD_MASK		0x00000fff	/* last valid descriptor */

/* receive channel status */
#define	D64_RS0_CD_MASK		0x00001fff	/* current descriptor pointer */
#define	D64_RS0_RS_MASK		0xf0000000     	/* receive state */
#define	D64_RS0_RS_SHIFT		28
#define	D64_RS0_RS_DISABLED	0x00000000	/* disabled */
#define	D64_RS0_RS_ACTIVE	0x10000000	/* active */
#define	D64_RS0_RS_IDLE		0x20000000	/* idle wait */
#define	D64_RS0_RS_STOPPED	0x30000000	/* stopped */
#define	D64_RS0_RS_SUSP		0x40000000	/* suspend pending */

#define	D64_RS1_AD_MASK		0x0001ffff	/* active descriptor */
#define	D64_RS1_RE_MASK		0xf0000000     	/* receive errors */
#define	D64_RS1_RE_SHIFT		28
#define	D64_RS1_RE_NOERR	0x00000000	/* no error */
#define	D64_RS1_RE_DPO		0x10000000	/* descriptor protocol error */
#define	D64_RS1_RE_DFU		0x20000000	/* data fifo overflow */
#define	D64_RS1_RE_DTE		0x30000000	/* data transfer error */
#define	D64_RS1_RE_DESRE	0x40000000	/* descriptor read error */
#define	D64_RS1_RE_COREE	0x50000000	/* core error */

/* fifoaddr */
#define	D64_FA_OFF_MASK		0xffff		/* offset */
#define	D64_FA_SEL_MASK		0xf0000		/* select */
#define	D64_FA_SEL_SHIFT	16
#define	D64_FA_SEL_XDD		0x00000		/* transmit dma data */
#define	D64_FA_SEL_XDP		0x10000		/* transmit dma pointers */
#define	D64_FA_SEL_RDD		0x40000		/* receive dma data */
#define	D64_FA_SEL_RDP		0x50000		/* receive dma pointers */
#define	D64_FA_SEL_XFD		0x80000		/* transmit fifo data */
#define	D64_FA_SEL_XFP		0x90000		/* transmit fifo pointers */
#define	D64_FA_SEL_RFD		0xc0000		/* receive fifo data */
#define	D64_FA_SEL_RFP		0xd0000		/* receive fifo pointers */
#define	D64_FA_SEL_RSD		0xe0000		/* receive frame status data */
#define	D64_FA_SEL_RSP		0xf0000		/* receive frame status pointers */

/* descriptor control flags 1 */
#define	D64_CTRL1_EOT		((uint32)1 << 28)	/* end of descriptor table */
#define	D64_CTRL1_IOC		((uint32)1 << 29)	/* interrupt on completion */
#define	D64_CTRL1_EOF		((uint32)1 << 30)	/* end of frame */
#define	D64_CTRL1_SOF		((uint32)1 << 31)	/* start of frame */

/* descriptor control flags 2 */
#define	D64_CTRL2_BC_MASK	0x00007fff	/* buffer byte count mask */
#define	D64_CTRL2_AE		0x00030000	/* address extension bits */
#define	D64_CTRL2_AE_SHIFT	16

/* control flags in the range [27:20] are core-specific and not defined here */
#define	D64_CTRL_CORE_MASK	0x0ff00000


#endif	/* _sbhnddma_h_ */
