/*
 * OTP support.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcmotp.c,v 1.54.2.58 2010/01/28 03:48:24 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmotp.h>


#define OTP_ERR_VAL	0x0001
#define OTP_MSG_VAL	0x0002
#define OTP_DBG_VAL	0x0004

uint32 otp_msg_level = OTP_ERR_VAL;

#define OTP_ERR(args)
#define OTP_MSG(args)
#define OTP_DBG(args)

/*
 * Common Code: Compiled for IPX / HND / AUTO
 */
#define HWSW_RGN(rgn) (((rgn) == OTP_HW_RGN) ? "h/w" : "s/w")

#define OTPP_TRIES	10000000	/* # of tries for OTPP */

/* Maximum OTP redundancy entries.  */
#define MAXNUMRDES	9

/* OTP common function type */
typedef int	(*otp_status_t)(void *oh);
typedef int	(*otp_size_t)(void *oh);
typedef void*	(*otp_init_t)(si_t *sih);
typedef uint16	(*otp_read_bit_t)(void *oh, chipcregs_t *cc, uint off);
typedef int	(*otp_read_region_t)(si_t *sih, int region, uint16 *data, uint *wlen);
typedef int	(*otp_nvread_t)(void *oh, char *data, uint *len);
typedef int	(*otp_write_region_t)(void *oh, int region, uint16 *data, uint wlen);
typedef int	(*otp_cis_append_region_t)(si_t *sih, int region, char *vars, int count);
typedef int	(*otp_lock_t)(si_t *sih);
typedef int	(*otp_nvwrite_t)(void *oh, uint16 *data, uint wlen);
typedef int	(*otp_dump_t)(void *oh, int arg, char *buf, uint size);

/* OTP function struct */
typedef struct otp_fn_s {
	otp_status_t		status;
	otp_size_t		size;
	otp_read_bit_t		read_bit;
	otp_init_t		init;
	otp_read_region_t	read_region;
	otp_nvread_t		nvread;
#ifdef BCMNVRAMW
	otp_write_region_t	write_region;
	otp_cis_append_region_t	cis_append_region;
	otp_lock_t		lock;
	otp_nvwrite_t		nvwrite;
#endif /* BCMNVRAMW */
#if defined(WLTEST)
	otp_dump_t		dump;
#endif	
	uint32			magic;
} otp_fn_t;
#define	OTP_FN_MAGIC		0x87654321

typedef struct {
	uint		ccrev;		/* chipc revision */
	otp_fn_t	*fn;		/* OTP functions */
	si_t		*sih;		/* Saved sb handle */
	osl_t		*osh;
#if defined(BCMAUTOOTP) || !defined(BCMHNDOTP)	    /* Newer IPX OTP wrapper */
	/* Geometry */
	uint16		wsize;		/* Size of otp in words */
	uint16		rows;
	uint16		cols;
	/* Flag bits (lock/prog/rv). Reflected only when OTP is power cycled */
	uint32		status;
	/* Subregion boundaries */
	uint16	hwbase;		/* hardware subregion offset */
	uint16	hwlim;		/* hardware subregion boundary */
	uint16	swbase;		/* software subregion offset */
	uint16	swlim;		/* software subregion boundary */
	uint16	fbase;		/* fuse subregion offset */
	uint16	flim;		/* fuse subregion boundary */
	/* Use these to hide differences between different revs */
	int	otpgu_base;	/* offset to General Use Region */
#ifdef BCMNVRAMW
	struct {
		uint8 width;		/* entry width in bits */
		uint8 val_shift;	/* value bit offset in the entry */
		uint8 offsets;		/* # entries */
		uint8 stat_shift;	/* valid bit in otpstatus */
		uint16 offset[MAXNUMRDES];	/* entry offset in OTP */
	} rde_cb;		/* OTP redundancy control blocks */
#endif	/* BCMNVRAMW */
#endif	/* BCMAUTOOTP || !BCMHNDOTP */
#if defined(BCMAUTOOTP) || defined(BCMHNDOTP)	    /* Older HND OTP wrapper */
	uint	size;		/* Size of otp in bytes */
	uint	hwprot;		/* Hardware protection bits */
	uint	signvalid;	/* Signature valid bits */
	int	boundary;	/* hw/sw boundary */
#endif	/* BCMAUTOOTP || BCMHNDOTP */
} otpinfo_t;

static otpinfo_t otpinfo;


/*
 * IPX OTP Code: Compiled for IPX OTP or AUTO mode
 *   Exported functions:
 *	ipxotp_status()
 *	ipxotp_size()
 *	ipxotp_init()
 *	ipxotp_read_bit()
 *	ipxotp_read_region()
 *	ipxotp_nvread()
 *	ipxotp_write_region()
 *	ipxotp_cis_append_region()
 *	ipxotp_lock()
 *	ipxotp_nvwrite()
 *	ipxotp_dump()
 *
 *   IPX internal functions:
 *	ipxotp_otpr()
 *	_ipxotp_init()
 *	ipxotp_write_bit()
 *	ipxotp_otpwb16()
 *	ipxotp_write_rde()
 *	ipxotp_fix_word16()
 *	ipxotp_check_word16()
 *	ipxotp_max_rgnsz()
 *	ipxotp_otprb16()
 *
 */

#if defined(BCMAUTOOTP) || !defined(BCMHNDOTP)	    /* Newer IPX OTP wrapper */

/* OTP layout */
/* CC revs 21, 24 and 27 OTP General Use Region word offset */
#define REVA4_OTPGU_BASE	12

/* CC revs 23, 25, 26, 28 and above OTP General Use Region word offset */
#define REVB8_OTPGU_BASE	20

/* Subregion word offsets in General Use region */
#define OTPGU_HSB_OFF		0
#define OTPGU_SFB_OFF		1
#define OTPGU_CI_OFF		2
#define OTPGU_P_OFF		3
#define OTPGU_SROM_OFF		4

/* Flag bit offsets in General Use region  */
#define OTPGU_HWP_OFF		60
#define OTPGU_SWP_OFF		61
#define OTPGU_CIP_OFF		62
#define OTPGU_FUSEP_OFF		63
#define OTPGU_CIP_MSK		0x4000
#define OTPGU_P_MSK		0xf000
#define OTPGU_P_SHIFT		(OTPGU_HWP_OFF % 16)

/* OTP Size */
#define OTP_SZ_FU_288		(288/8)		/* 288 bits */
#define OTP_SZ_FU_72		(72/8)		/* 72 bits */
#define OTP_SZ_CHECKSUM		(16/8)		/* 16 bits */
#define OTP4315_SWREG_SZ	178		/* 178 bytes */

static int
ipxotp_status(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return (int)(oi->status);
}

/* Return size in bytes */
static int
ipxotp_size(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return (int)oi->wsize * 2;
}

static uint16
ipxotp_otpr(void *oh, chipcregs_t *cc, uint wn)
{
	otpinfo_t *oi;

	oi = (otpinfo_t *)oh;

	ASSERT(wn < oi->wsize);
	ASSERT(cc != NULL);

	return R_REG(oi->osh, &cc->sromotp[wn]);
}

static uint16
ipxotp_read_bit(void *oh, chipcregs_t *cc, uint off)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint k, row, col;
	uint32 otpp, st;

	row = off / oi->cols;
	col = off % oi->cols;

	otpp = OTPP_START_BUSY |
	        ((OTPPOC_READ << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
	        ((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
	        ((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
	OTP_DBG(("%s: off = %d, row = %d, col = %d, otpp = 0x%x",
	         __FUNCTION__, off, row, col, otpp));
	W_REG(oi->osh, &cc->otpprog, otpp);

	for (k = 0;
	     ((st = R_REG(oi->osh, &cc->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return -1;
	}
	if (st & OTPP_READERR) {
		OTP_ERR(("\n%s: Could not read OTP bit %d\n", __FUNCTION__, off));
		return -1;
	}
	st = (st & OTPP_VALUE_MASK) >> OTPP_VALUE_SHIFT;

	OTP_DBG((" => %d\n", st));
	return (int)st;
}


/* Calculate max HW/SW region byte size by substracting fuse region and checksum size,
 * osizew is oi->wsize (OTP size - GU size) in words
 */
static int
ipxotp_max_rgnsz(si_t *sih, int osizew)
{
	int ret = 0;

	switch (CHIPID(sih->chip)) {
	case BCM4322_CHIP_ID:
	case BCM43221_CHIP_ID:
	case BCM43231_CHIP_ID:
		ret = osizew*2 - OTP_SZ_FU_288 - OTP_SZ_CHECKSUM;
		break;
	case BCM43222_CHIP_ID:
	case BCM43111_CHIP_ID:
	case BCM43112_CHIP_ID:

	case BCM43224_CHIP_ID:
	case BCM43225_CHIP_ID:
	case BCM43421_CHIP_ID:
	case BCM4313_CHIP_ID:
		ret = osizew*2 - OTP_SZ_FU_72 - OTP_SZ_CHECKSUM;
		break;
	case BCM4319_CHIP_ID:
		ret = osizew*2;
		break;
	default:
		ASSERT(0);	/* Don't konw about this chip */
	}

	OTP_MSG(("max region size %d bytes\n", ret));
	return ret;
}

static void
BCMNMIATTACHFN(_ipxotp_init)(otpinfo_t *oi, chipcregs_t *cc)
{
	uint	k;
	uint32 otpp, st;

	/* record word offset of General Use Region for various chipcommon revs */
	if (oi->sih->ccrev == 21 || oi->sih->ccrev == 24 || oi->sih->ccrev == 27) {
		oi->otpgu_base = REVA4_OTPGU_BASE;
	} else if (oi->sih->ccrev == 23 || oi->sih->ccrev >= 25) {
		oi->otpgu_base = REVB8_OTPGU_BASE;
	} else {
		OTP_ERR(("%s: chipc rev %d not supported\n", __FUNCTION__, oi->sih->ccrev));
	}

	/* First issue an init command so the status is up to date */
	otpp = OTPP_START_BUSY |
	        ((OTPPOC_INIT << OTPP_OC_SHIFT) & OTPP_OC_MASK);
	OTP_DBG(("%s: otpp = 0x%x", __FUNCTION__, otpp));
	W_REG(oi->osh, &cc->otpprog, otpp);
	for (k = 0;
	     ((st = R_REG(oi->osh, &cc->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return;
	}

	/* Read OTP lock bits and subregion programmed indication bits */
	oi->status = R_REG(oi->osh, &cc->otpstatus);

	if ((CHIPID(oi->sih->chip) == BCM43222_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43111_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43112_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43224_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43225_CHIP_ID) ||
		(CHIPID(oi->sih->chip) == BCM43421_CHIP_ID) ||
	0) {
		uint32 p_bits;
		p_bits = (ipxotp_otpr(oi, cc, oi->otpgu_base + OTPGU_P_OFF) & OTPGU_P_MSK)
			>> OTPGU_P_SHIFT;
		oi->status |= (p_bits << OTPS_GUP_SHIFT);
	}
	OTP_DBG(("%s: status 0x%x\n", __FUNCTION__, oi->status));

	/*
	 * h/w region base and fuse region limit are fixed to the top and
	 * the bottom of the general use region. Everything else can be flexible.
	 */
	oi->hwbase = oi->otpgu_base + OTPGU_SROM_OFF;
	oi->hwlim = oi->wsize;
	if (oi->status & OTPS_GUP_HW) {
		oi->hwlim = ipxotp_otpr(oi, cc, oi->otpgu_base + OTPGU_HSB_OFF) / 16;
		oi->swbase = oi->hwlim;
	}
	else
		oi->swbase = oi->hwbase;

	/* subtract fuse and checksum from beginning */
	oi->swlim = ipxotp_max_rgnsz(oi->sih, oi->wsize) / 2;
	if (oi->status & OTPS_GUP_SW) {
		oi->swlim = ipxotp_otpr(oi, cc, oi->otpgu_base + OTPGU_SFB_OFF) / 16;
		oi->fbase = oi->swlim;
	}
	else
		oi->fbase = oi->swbase;

	oi->flim = oi->wsize;

	OTP_DBG(("%s: hwbase %d/%d hwlim %d/%d\n", __FUNCTION__, oi->hwbase, oi->hwbase * 16,
	         oi->hwlim, oi->hwlim * 16));
	OTP_DBG(("%s: swbase %d/%d swlim %d/%d\n", __FUNCTION__, oi->swbase, oi->swbase * 16,
	         oi->swlim, oi->swlim * 16));
	OTP_DBG(("%s: fbase %d/%d flim %d/%d\n", __FUNCTION__, oi->fbase, oi->fbase * 16,
	         oi->flim, oi->flim * 16));
}

static void*
BCMNMIATTACHFN(ipxotp_init)(si_t *sih)
{
	uint idx;
	chipcregs_t *cc;
	otpinfo_t *oi;

	OTP_MSG(("%s: Use IPX OTP controller\n", __FUNCTION__));

	/* Make sure we running IPX OTP */
	ASSERT((sih->ccrev >= 21) && (sih->ccrev != 22));
	if (sih->ccrev < 21 || sih->ccrev == 22)
		return NULL;

	/* Make sure OTP is not disabled */
	if (si_is_otp_disabled(sih)) {
		OTP_MSG(("%s: OTP is disabled\n", __FUNCTION__));
#if !defined(WLTEST)
		return NULL;
#endif 
	}

	/* Make sure OTP is powered up */
	if (!si_is_otp_powered(sih)) {
		OTP_ERR(("%s: OTP is powered down\n", __FUNCTION__));
		return NULL;
	}

	oi = &otpinfo;

	/* Check for otp size */
	switch ((sih->cccaps & CC_CAP_OTPSIZE) >> CC_CAP_OTPSIZE_SHIFT) {
	case 0:
		/* Nothing there */
		OTP_ERR(("%s: no OTP\n", __FUNCTION__));
		return NULL;
	case 1:	/* 32x64 */
		oi->rows = 32;
		oi->cols = 64;
		oi->wsize = 128;
		break;
	case 2:	/* 64x64 */
		oi->rows = 64;
		oi->cols = 64;
		oi->wsize = 256;
		break;
	case 5:	/* 96x64 */
		oi->rows = 96;
		oi->cols = 64;
		oi->wsize = 384;
		break;
	default:
		/* Don't know the geometry */
		OTP_ERR(("%s: unknown OTP geometry\n", __FUNCTION__));
		return NULL;
	}

	OTP_MSG(("%s: rows %u cols %u wsize %u\n", __FUNCTION__, oi->rows, oi->cols, oi->wsize));

#ifdef BCMNVRAMW
	/* Initialize OTP redundancy control blocks */
	if (sih->ccrev == 21 || sih->ccrev == 24) {
		uint16 offset[] = {64, 79, 94, 109, 128, 143, 158, 173};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 15;
		oi->rde_cb.val_shift = 11;
		oi->rde_cb.stat_shift = 16;
	}
	else if (sih->ccrev == 27) {
		uint16 offset[] = {128, 143, 158, 173};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 15;
		oi->rde_cb.val_shift = 11;
		oi->rde_cb.stat_shift = 20;
	}
	else {
		uint16 offset[] = {141, 158, 175, 205, 222, 239, 269, 286, 303};
		bcopy(offset, &oi->rde_cb.offset, sizeof(offset));
		oi->rde_cb.offsets = ARRAYSIZE(offset);
		oi->rde_cb.width = 17;
		oi->rde_cb.val_shift = 13;
		oi->rde_cb.stat_shift = 16;
	}
	ASSERT(oi->rde_cb.offsets <= MAXNUMRDES);
#endif	/* BCMNVRAMW */

	/* Retrieve OTP region info */
	idx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	_ipxotp_init(oi, cc);

	si_setcoreidx(sih, idx);

	return (void *)oi;
}

static int
ipxotp_read_region(void *oh, int region, uint16 *data, uint *wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	chipcregs_t *cc;
	uint base, i, sz;

	/* Validate region selection */
	switch (region) {
	case OTP_HW_RGN:
		sz = (uint)oi->hwlim - oi->hwbase;
		if (!(oi->status & OTPS_GUP_HW)) {
			OTP_ERR(("%s: h/w region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
			         __FUNCTION__, oi->hwlim - oi->hwbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->hwbase;
		break;
	case OTP_SW_RGN:
		sz = ((uint)oi->swlim - oi->swbase);
		if (!(oi->status & OTPS_GUP_SW)) {
			OTP_ERR(("%s: s/w region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small should be at least %u\n",
			         __FUNCTION__, oi->swlim - oi->swbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->swbase;
		break;
	case OTP_CI_RGN:
		sz = OTPGU_CI_SZ;
		if (!(oi->status & OTPS_GUP_CI)) {
			OTP_ERR(("%s: chipid region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
			         __FUNCTION__, OTPGU_CI_SZ));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->otpgu_base + OTPGU_CI_OFF;
		break;
	case OTP_FUSE_RGN:
		sz = (uint)oi->flim - oi->fbase;
		if (!(oi->status & OTPS_GUP_FUSE)) {
			OTP_ERR(("%s: fuse region not programmed\n", __FUNCTION__));
			*wlen = sz;
			return BCME_NOTFOUND;
		}
		if (*wlen < sz) {
			OTP_ERR(("%s: buffer too small, should be at least %u\n",
			         __FUNCTION__, oi->flim - oi->fbase));
			*wlen = sz;
			return BCME_BUFTOOSHORT;
		}
		base = oi->fbase;
		break;
	default:
		OTP_ERR(("%s: reading region %d is not supported\n", __FUNCTION__, region));
		return BCME_BADARG;
	}

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Read the data */
	for (i = 0; i < sz; i ++)
		data[i] = ipxotp_otpr(oh, cc, base + i);

	si_setcoreidx(oi->sih, idx);
	*wlen = sz;
	return 0;
}

static int
ipxotp_nvread(void *oh, char *data, uint *len)
{
	return BCME_UNSUPPORTED;
}

#ifdef BCMNVRAMW
static int
ipxotp_write_bit(otpinfo_t *oi, chipcregs_t *cc, uint off)
{
	uint k, row, col;
	uint32 otpp, st;

	row = off / oi->cols;
	col = off % oi->cols;

	otpp = OTPP_START_BUSY |
	        ((1 << OTPP_VALUE_SHIFT) & OTPP_VALUE_MASK) |
	        ((OTPPOC_BIT_PROG << OTPP_OC_SHIFT) & OTPP_OC_MASK) |
	        ((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
	        ((col << OTPP_COL_SHIFT) & OTPP_COL_MASK);
	OTP_DBG(("%s: off = %d, row = %d, col = %d, otpp = 0x%x\n",
	         __FUNCTION__, off, row, col, otpp));
	W_REG(oi->osh, &cc->otpprog, otpp);

	for (k = 0;
	     ((st = R_REG(oi->osh, &cc->otpprog)) & OTPP_START_BUSY) && (k < OTPP_TRIES);
	     k ++)
		;
	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return -1;
	}

	return 0;
}

static int
ipxotp_otpwb16(otpinfo_t *oi, chipcregs_t *cc, int wn, uint16 data)
{
	uint base, i;
	int rc = 0;

	base = wn * 16;
	for (i = 0; i < 16; i++) {
		if (data & (1 << i)) {
			if ((rc = ipxotp_write_bit(oi, cc, base + i)))
				break;
		}
	}

	return rc;
}

/* Write OTP redundancy entry:
 *  rde - redundancy entry index (-ve for "next")
 *  bit - bit offset
 *  val - bit value
 */
int
ipxotp_write_rde(void *oh, int rde, uint bit, uint val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	chipcregs_t *cc;
	uint i, temp;

#ifdef BCMCBG
	if ((rde >= oi->rde_cb.offsets) || (bit >= oi->rows * oi->cols) || (val > 1))
		return BCME_RANGE;
#endif

	if (rde < 0) {
		for (rde = 0; rde < oi->rde_cb.offsets - 1; rde++) {
			if ((oi->status & (1 << (oi->rde_cb.stat_shift + rde))) == 0)
				break;
		}
		OTP_MSG(("%s: Auto rde index %d\n", __FUNCTION__, rde));
	}

	if (oi->status & (1 << (oi->rde_cb.stat_shift + rde))) {
		OTP_MSG(("%s: rde %d already in use, status 0x%08x\n", __FUNCTION__,
		         rde, oi->status));
		return BCME_ERROR;
	}

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	temp = ~(~0 << oi->rde_cb.width) &
	        ((~0 << (oi->rde_cb.val_shift + 1)) | (val << oi->rde_cb.val_shift) | bit);

	OTP_MSG(("%s: rde %d bit %d val %d bmp 0x%08x\n", __FUNCTION__, rde, bit, val, temp));

	/* Enable Write */
	OR_REG(oi->osh, &cc->otpcontrol, OTPC_PROGEN);

	for (i = 0; i < oi->rde_cb.width; i ++) {
		if (!(temp & (1 << i)))
			continue;
		ipxotp_write_bit(oi, cc, oi->rde_cb.offset[rde] + i);
	}

	/* Disable Write */
	AND_REG(oi->osh, &cc->otpcontrol, ~OTPC_PROGEN);

	si_otp_power(oi->sih, FALSE);
	si_otp_power(oi->sih, TRUE);
	_ipxotp_init(oi, cc);

	si_setcoreidx(oi->sih, idx);
	return BCME_OK;
}

/* Set up redundancy entries for the specified bits */
static int
ipxotp_fix_word16(void *oh, uint wn, uint16 mask, uint16 val)
{
	otpinfo_t *oi;
	uint bit;
	int rc = 0;

	oi = (otpinfo_t *)oh;

	ASSERT(oi != NULL);
	ASSERT(wn < oi->wsize);

	for (bit = wn * 16; mask; bit++, mask >>= 1, val >>= 1) {
		if (mask & 1) {
			if ((rc = ipxotp_write_rde(oi, -1, bit, val & 1)))
				break;
		}
	}

	return rc;
}

static int
ipxotp_check_word16(void *oh, chipcregs_t *cc, uint wn, uint16 val)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint16 word = ipxotp_otpr(oi, cc, wn);
	int rc = 0;

	if ((word ^= val)) {
		OTP_MSG(("%s: word %d is 0x%04x, wanted 0x%04x, fixing...\n",
			__FUNCTION__, wn, (word ^ val), val));
		if ((rc = ipxotp_fix_word16(oi, wn, word, val))) {
			OTP_ERR(("FAILED, ipxotp_fix_word16 returns %d\n", rc));
			/* Fatal error, unfixable. MFGC will have to fail. Board
			 * needs to be discarded!!
			 */
			return BCME_NORESOURCE;
		}
	}

	return BCME_OK;
}


/* expects the caller to disable interrupts before calling this routine */
static int
ipxotp_write_region(void *oh, int region, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx;
	chipcregs_t *cc;
	uint base, i;
	int otpgu_bit_base;
	bool rewrite = FALSE;
	int rc = 0;
	uint16 *origdata = NULL;

	otpgu_bit_base = oi->otpgu_base * 16;

	/* Validate region selection */
	switch (region) {
	case OTP_HW_RGN:
		if (wlen > (uint)(oi->hwlim - oi->hwbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP h/w region limit %u\n",
			         __FUNCTION__, wlen, oi->hwlim - oi->hwbase));
			return BCME_BUFTOOLONG;
		}
		rewrite = !!(oi->status & OTPS_GUP_HW);
		base = oi->hwbase;
		break;
	case OTP_SW_RGN:
		if (wlen > (uint)(oi->swlim - oi->swbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP s/w region limit %u\n",
			         __FUNCTION__, wlen, oi->swlim - oi->swbase));
			return BCME_BUFTOOLONG;
		}
		rewrite = !!(oi->status & OTPS_GUP_SW);
		base = oi->swbase;
		break;
	case OTP_CI_RGN:
		if (oi->status & OTPS_GUP_CI) {
			OTP_ERR(("%s: chipid region has been programmed\n", __FUNCTION__));
			return BCME_ERROR;
		}
		if (wlen > OTPGU_CI_SZ) {
			OTP_ERR(("%s: wlen %u exceeds OTP ci region limit %u\n",
			         __FUNCTION__, wlen, OTPGU_CI_SZ));
			return BCME_BUFTOOLONG;
		}
		if ((wlen == OTPGU_CI_SZ) && (data[OTPGU_CI_SZ - 1] & OTPGU_P_MSK) != 0) {
			OTP_ERR(("%s: subregion programmed bits not zero\n", __FUNCTION__));
			return BCME_BADARG;
		}
		base = oi->otpgu_base + OTPGU_CI_OFF;
		break;
	case OTP_FUSE_RGN:
		if (oi->status & OTPS_GUP_FUSE) {
			OTP_ERR(("%s: fuse region has been programmed\n", __FUNCTION__));
			return BCME_ERROR;
		}
		if (wlen > (uint)(oi->flim - oi->fbase)) {
			OTP_ERR(("%s: wlen %u exceeds OTP ci region limit %u\n",
			         __FUNCTION__, wlen, oi->flim - oi->fbase));
			return BCME_BUFTOOLONG;
		}
		base = oi->flim - wlen;
		break;
	default:
		OTP_ERR(("%s: writing region %d is not supported\n", __FUNCTION__, region));
		return BCME_ERROR;
	}

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Check for conflict; Since some bits might be programmed at ATE time, we need to
	 * avoid redundancy by clearing already written bits, but copy original for verification.
	 */
	if ((origdata = (uint16*)MALLOC(oi->osh, wlen * 2)) == NULL) {
		rc = BCME_NOMEM;
		goto exit;
	}
	for (i = 0; i < wlen; i++) {
		origdata[i] = data[i];
		data[i] = ipxotp_otpr(oi, cc, base + i);
		if (data[i] & ~origdata[i]) {
			OTP_ERR(("%s: %s region: word %d incompatible (0x%04x->0x%04x)\n",
			         __FUNCTION__, HWSW_RGN(region), i, data[i], origdata[i]));
			rc = BCME_BADARG;
			goto exit;
		}
		data[i] ^= origdata[i];
	}
	OTP_MSG(("%s: writing new bits in %s region\n", __FUNCTION__, HWSW_RGN(region)));

	/* Enable Write */
	OR_REG(oi->osh, &cc->otpcontrol, OTPC_PROGEN);

	/* Write the data */
	for (i = 0; i < wlen; i++) {
		ipxotp_otpwb16(oi, cc, base + i, data[i]);
	}

	/* One time set region flag: Update boundary/flag in memory and in OTP */
	if (!rewrite) {
		switch (region) {
		case OTP_HW_RGN:
			ipxotp_otpwb16(oi, cc, oi->otpgu_base + OTPGU_HSB_OFF, (base + i) * 16);
			ipxotp_write_bit(oi, cc, otpgu_bit_base + OTPGU_HWP_OFF);
			break;
		case OTP_SW_RGN:
			/* Write HW region limit as well */
			ipxotp_otpwb16(oi, cc, oi->otpgu_base + OTPGU_HSB_OFF, base * 16);
			/* write max swlim(covert to bits) to the sw/fuse boundary */
			ipxotp_otpwb16(oi, cc, oi->otpgu_base + OTPGU_SFB_OFF, oi->swlim * 16);
			ipxotp_write_bit(oi, cc, otpgu_bit_base + OTPGU_SWP_OFF);
			break;
		case OTP_CI_RGN:
			ipxotp_write_bit(oi, cc, otpgu_bit_base + OTPGU_CIP_OFF);
			/* Also set the OTPGU_CIP_MSK bit in the input so verification
			 * doesn't fail
			 */
			if (wlen >= OTPGU_CI_SZ)
				data[OTPGU_CI_SZ - 1] |= OTPGU_CIP_MSK;
			break;
		case OTP_FUSE_RGN:
			ipxotp_otpwb16(oi, cc, oi->otpgu_base + OTPGU_SFB_OFF, base * 16);
			ipxotp_write_bit(oi, cc, otpgu_bit_base + OTPGU_FUSEP_OFF);
			break;
		}
	}

	/* Disable Write */
	AND_REG(oi->osh, &cc->otpcontrol, ~OTPC_PROGEN);

	/* Sync region info by retrieving them again (use PMU bit to power cycle OTP) */
	si_otp_power(oi->sih, FALSE);
	si_otp_power(oi->sih, TRUE);

	/* Check and fix for region size and region programmed bits */
	if (!rewrite) {
		uint16	boundary_off = 0, boundary_val = 0;
		uint16	programmed_off = 0;
		uint16	bit = 0;

		switch (region) {
		case OTP_HW_RGN:
			boundary_off = OTPGU_HSB_OFF;
			boundary_val = (base + i) * 16;
			programmed_off = OTPGU_HWP_OFF;
			break;
		case OTP_SW_RGN:
			/* Also write 0 to HW region boundary */
			if ((rc = ipxotp_check_word16(oi, cc, oi->otpgu_base + OTPGU_HSB_OFF,
				base * 16)))
				goto exit;
			boundary_off = OTPGU_SFB_OFF;
			boundary_val = oi->swlim * 16;
			programmed_off = OTPGU_SWP_OFF;
			break;
		case OTP_CI_RGN:
			/* No CI region boundary */
			programmed_off = OTPGU_CIP_OFF;
			break;
		case OTP_FUSE_RGN:
			boundary_off = OTPGU_SFB_OFF;
			boundary_val = base * 16;
			programmed_off = OTPGU_FUSEP_OFF;
			break;
		}

		/* Do the actual checking and return BCME_NORESOURCE if we cannot fix */
		if ((region != OTP_CI_RGN) &&
			(rc = ipxotp_check_word16(oi, cc, oi->otpgu_base + boundary_off,
			boundary_val))) {
			goto exit;
		}

		if ((bit = ipxotp_read_bit(oh, cc, otpgu_bit_base + programmed_off)) == 0xffff) {
			OTP_ERR(("\n%s: FAILED bit %d reads %d\n", __FUNCTION__, otpgu_bit_base +
				programmed_off, bit));
			goto exit;
		} else if (bit == 0) {	/* error detected, fix it */
			OTP_ERR(("\n%s: FAILED bit %d reads %d, fixing\n", __FUNCTION__,
				otpgu_bit_base + programmed_off, bit));
			if ((rc = ipxotp_write_rde(oi, -1, otpgu_bit_base + programmed_off, 1))) {
				OTP_ERR(("\n%s: cannot fix, ipxotp_write_rde returns %d\n",
					__FUNCTION__, rc));
				goto exit;
			}
		}
	}

	/* Update status, apply WAR */
	_ipxotp_init(oi, cc);

	/* Recover original data... */
	if (origdata)
		bcopy(origdata, data, wlen * 2);

	/* ...so we can verify and fix where possible */
	for (i = 0; i < wlen; i++) {
		if ((rc = ipxotp_check_word16(oi, cc, base + i, data[i])))
			goto exit;
	}

exit:
	if (origdata)
		MFREE(oi->osh, origdata, wlen * 2);

	si_setcoreidx(oi->sih, idx);
	return rc;
}

static int
ipxotp_cis_append_region(si_t *sih, int region, char *vars, int count)
{
	uint8 *cis;
	osl_t *osh;
	uint sz = OTP_SZ_MAX/2; /* size in words */
	int rc = 0;
	bool newchip = FALSE;

	ASSERT(region == OTP_HW_RGN || region == OTP_SW_RGN);

	osh = si_osh(sih);
	if ((cis = MALLOC(osh, OTP_SZ_MAX)) == NULL) {
		return BCME_ERROR;
	}

	bzero(cis, OTP_SZ_MAX);

	rc = otp_read_region(sih, region, (uint16 *)cis, &sz);
	newchip = (rc == BCME_NOTFOUND) ? TRUE : FALSE;
	rc = 0;

	/* zero count for read, non-zero count for write */
	if (count) {
		int i = 0, newlen = 0;

		if (newchip) {
			int termw_len = 0;	/* length of termination word */

			/* convert halfwords to bytes offset */
			newlen = sz * 2;

			if ((CHIPID(sih->chip) == BCM4322_CHIP_ID) ||
				(CHIPID(sih->chip) == BCM43231_CHIP_ID) ||
				1) {
				/* bootloader WAR, refer to above twiki link */
				cis[newlen-1] = 0x00;
				cis[newlen-2] = 0xff;
				cis[newlen-3] = 0x00;
				cis[newlen-4] = 0xff;
				cis[newlen-5] = 0xff;
				cis[newlen-6] = 0x1;
				cis[newlen-7] = 0x2;
				termw_len = 7;
			} else {
				cis[newlen-1] = 0xff;
				cis[newlen-2] = 0xff;
				termw_len = 2;
			}

			if (count >= newlen - termw_len) {
				OTP_MSG(("OTP left %x bytes; buffer %x bytes\n", newlen, count));
				rc = BCME_BUFTOOLONG;
			}
		} else {
			int end = 0;

			/* Walk through the leading zeros (could be 0 or 8 bytes for now) */
			for (i = 0; i < (int)sz*2; i++)
				if (cis[i] != 0)
					break;

			/* Find the place to append */
			for (; i < (int)sz*2; i++) {
				if (cis[i] == 0)
					break;
				i += ((int)cis[i+1] + 1);
			}

			for (end = i; end < (int)sz*2; end++) {
				if (cis[end] != 0)
					break;
			}

			newlen = i + count;
			if (newlen & 1)		/* make it even-sized buffer */
				newlen++;

			if (newlen >= (end - 1)) {
				OTP_MSG(("OTP left %x bytes; buffer %x bytes\n", end-i, count));
				rc = BCME_BUFTOOLONG;
			}
		}

		/* copy the buffer */
		memcpy(&cis[i], vars, count);
#ifdef BCMNVRAMW
		/* Write the buffer back */
		if (!rc)
			rc = otp_write_region(sih, region, (uint16*)cis, newlen/2);

		/* Print the buffer */
		OTP_MSG(("Buffer of size %d bytes to write:\n", newlen));
		for (i = 0; i < newlen; i++) {
			OTP_DBG(("%02x ", cis[i] & 0xff));
			if ((i % 16) == 15) {
				OTP_DBG(("\n"));
			}
		}
		OTP_MSG(("\n"));
#endif /* BCMNVRAMW */
	}

	if (cis)
		MFREE(osh, cis, OTP_SZ_MAX);

	return (rc);
}

/* No need to lock for IPXOTP. HNDOTP needs it due to the randomness of unprogrammed
 * content
 */
static int
ipxotp_lock(void *oh)
{
	return 0;
}

static int
ipxotp_nvwrite(void *oh, uint16 *data, uint wlen)
{
	return -1;
}
#endif /* BCMNVRAMW */

#if defined(WLTEST)
static uint16
ipxotp_otprb16(void *oh, chipcregs_t *cc, uint wn)
{
	uint base, i;
	uint16 val;
	uint16 bit;

	base = wn * 16;

	val = 0;
	for (i = 0; i < 16; i++) {
		if ((bit = ipxotp_read_bit(oh, cc, base + i)) == 0xffff)
			break;
		val = val | (bit << i);
	}
	if (i < 16)
		val = 0xffff;

	return val;
}

static int
ipxotp_dump(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	chipcregs_t *cc;
	uint idx, i, count;
	uint16 val;
	struct bcmstrbuf b;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	count = ipxotp_size(oh);

	bcm_binit(&b, buf, size);
	for (i = 0; i < count / 2; i++) {
		if (!(i % 4))
			bcm_bprintf(&b, "\n0x%04x:", 2 * i);
		if (arg == 0)
			val = ipxotp_otpr(oh, cc, i);
		else
			val = ipxotp_otprb16(oi, cc, i);
		bcm_bprintf(&b, " 0x%04x", val);
	}
	bcm_bprintf(&b, "\n");

	si_setcoreidx(oi->sih, idx);

	return ((int)(b.buf - b.origbuf));
}
#endif	

static otp_fn_t ipxotp_fn = {
	(otp_status_t)ipxotp_status,
	(otp_size_t)ipxotp_size,
	(otp_read_bit_t)ipxotp_read_bit,
	(otp_init_t)ipxotp_init,
	(otp_read_region_t)ipxotp_read_region,
	(otp_nvread_t)ipxotp_nvread,
#ifdef BCMNVRAMW
	(otp_write_region_t)ipxotp_write_region,
	(otp_cis_append_region_t)ipxotp_cis_append_region,
	(otp_lock_t)ipxotp_lock,
	(otp_nvwrite_t)ipxotp_nvwrite,
#endif /* BCMNVRAMW */
#if defined(WLTEST)
	(otp_dump_t)ipxotp_dump,
#endif	
	OTP_FN_MAGIC
};

#endif	/* BCMAUTOOTP || !BCMHNDOTP */


/*
 * HND OTP Code: Compiled for HND OTP or AUTO mode
 *   Exported functions:
 *	hndotp_status()
 *	hndotp_size()
 *	hndotp_init()
 *	hndotp_read_bit()
 *	hndotp_read_region()
 *	hndotp_nvread()
 *	hndotp_write_region()
 *	hndotp_cis_append_region()
 *	hndotp_lock()
 *	hndotp_nvwrite()
 *	hndotp_dump()
 *
 *   HND internal functions:
 * 	hndotp_otpr()
 * 	hndotp_otproff()
 *	hndotp_write_bit()
 *	hndotp_write_word()
 *	hndotp_valid_rce()
 *	hndotp_write_rce()
 *	hndotp_write_row()
 *	hndotp_otprb16()
 *
 */

#if defined(BCMAUTOOTP) || defined(BCMHNDOTP)	    /* Older HND OTP wrapper */

/* Fields in otpstatus */
#define	OTPS_PROGFAIL		0x80000000
#define	OTPS_PROTECT		0x00000007
#define	OTPS_HW_PROTECT		0x00000001
#define	OTPS_SW_PROTECT		0x00000002
#define	OTPS_CID_PROTECT	0x00000004
#define	OTPS_RCEV_MSK		0x00003f00
#define	OTPS_RCEV_SHIFT		8

/* Fields in the otpcontrol register */
#define	OTPC_RECWAIT		0xff000000
#define	OTPC_PROGWAIT		0x00ffff00
#define	OTPC_PRW_SHIFT		8
#define	OTPC_MAXFAIL		0x00000038
#define	OTPC_VSEL		0x00000006
#define	OTPC_SELVL		0x00000001

/* OTP regions (Word offsets from otp size) */
#define	OTP_SWLIM_OFF	(-4)
#define	OTP_CIDBASE_OFF	0
#define	OTP_CIDLIM_OFF	4

/* Predefined OTP words (Word offset from otp size) */
#define	OTP_BOUNDARY_OFF (-4)
#define	OTP_HWSIGN_OFF	(-3)
#define	OTP_SWSIGN_OFF	(-2)
#define	OTP_CIDSIGN_OFF	(-1)
#define	OTP_CID_OFF	0
#define	OTP_PKG_OFF	1
#define	OTP_FID_OFF	2
#define	OTP_RSV_OFF	3
#define	OTP_LIM_OFF	4
#define	OTP_RD_OFF	4	/* Redundancy row starts here */
#define	OTP_RC0_OFF	28	/* Redundancy control word 1 */
#define	OTP_RC1_OFF	32	/* Redundancy control word 2 */
#define	OTP_RC_LIM_OFF	36	/* Redundancy control word end */

#define	OTP_HW_REGION	OTPS_HW_PROTECT
#define	OTP_SW_REGION	OTPS_SW_PROTECT
#define	OTP_CID_REGION	OTPS_CID_PROTECT

#if OTP_HW_REGION != OTP_HW_RGN
#error "incompatible OTP_HW_RGN"
#endif
#if OTP_SW_REGION != OTP_SW_RGN
#error "incompatible OTP_SW_RGN"
#endif
#if OTP_CID_REGION != OTP_CI_RGN
#error "incompatible OTP_CI_RGN"
#endif

/* Redundancy entry definitions */
#define	OTP_RCE_ROW_SZ		6
#define	OTP_RCE_SIGN_MASK	0x7fff
#define	OTP_RCE_ROW_MASK	0x3f
#define	OTP_RCE_BITS		21
#define	OTP_RCE_SIGN_SZ		15
#define	OTP_RCE_BIT0		1

#define	OTP_WPR		4
#define	OTP_SIGNATURE	0x578a
#define	OTP_MAGIC	0x4e56

static int
hndotp_status(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return ((int)(oi->hwprot | oi->signvalid));
}

static int
hndotp_size(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	return ((int)(oi->size));
}

static uint16
hndotp_otpr(void *oh, chipcregs_t *cc, uint wn)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	osl_t *osh;
	uint16 *ptr;

	ASSERT(wn < ((oi->size / 2) + OTP_RC_LIM_OFF));
	ASSERT(cc);

	osh = si_osh(oi->sih);

	ptr = (uint16 *)((uchar *)cc + CC_SROM_OTP);
	return (R_REG(osh, &ptr[wn]));
}

static uint16
hndotp_otproff(void *oh, chipcregs_t *cc, int woff)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	osl_t *osh;
	uint16 *ptr;

	ASSERT(woff >= (-((int)oi->size / 2)));
	ASSERT(woff < OTP_LIM_OFF);
	ASSERT(cc);

	osh = si_osh(oi->sih);

	ptr = (uint16 *)((uchar *)cc + CC_SROM_OTP);

	return (R_REG(osh, &ptr[(oi->size / 2) + woff]));
}

static uint16
hndotp_read_bit(void *oh, chipcregs_t *cc, uint idx)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint k, row, col;
	uint32 otpp, st;
	osl_t *osh;

	osh = si_osh(oi->sih);
	row = idx / 65;
	col = idx % 65;

	otpp = OTPP_START_BUSY | OTPP_READ |
	        ((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
	        (col & OTPP_COL_MASK);

	OTP_DBG(("%s: idx = %d, row = %d, col = %d, otpp = 0x%x", __FUNCTION__,
	         idx, row, col, otpp));

	W_REG(osh, &cc->otpprog, otpp);
	st = R_REG(osh, &cc->otpprog);
	for (k = 0; ((st & OTPP_START_BUSY) == OTPP_START_BUSY) && (k < OTPP_TRIES); k++)
		st = R_REG(osh, &cc->otpprog);

	if (k >= OTPP_TRIES) {
		OTP_ERR(("\n%s: BUSY stuck: st=0x%x, count=%d\n", __FUNCTION__, st, k));
		return 0xffff;
	}
	if (st & OTPP_READERR) {
		OTP_ERR(("\n%s: Could not read OTP bit %d\n", __FUNCTION__, idx));
		return 0xffff;
	}
	st = (st & OTPP_VALUE_MASK) >> OTPP_VALUE_SHIFT;
	OTP_DBG((" => %d\n", st));
	return (uint16)st;
}

static void*
BCMNMIATTACHFN(hndotp_init)(si_t *sih)
{
	uint idx;
	chipcregs_t *cc;
	otpinfo_t *oi;
	uint32 cap = 0, clkdiv, otpdiv = 0;
	void *ret = NULL;
	osl_t *osh;

	OTP_MSG(("%s: Use HND OTP controller\n", __FUNCTION__));
	oi = &otpinfo;

	idx = si_coreidx(sih);
	osh = si_osh(oi->sih);

	/* Check for otp */
	if ((cc = si_setcoreidx(sih, SI_CC_IDX)) != NULL) {
		cap = R_REG(osh, &cc->capabilities);
		if ((cap & CC_CAP_OTPSIZE) == 0) {
			/* Nothing there */
			goto out;
		}

		/* As of right now, support only 4320a2, 4311a1 and 4312 */
		ASSERT((oi->ccrev == 12) || (oi->ccrev == 17) || (oi->ccrev == 22));
		if (!((oi->ccrev == 12) || (oi->ccrev == 17) || (oi->ccrev == 22)))
			return NULL;

		/* Read the OTP byte size. chipcommon rev >= 18 has RCE so the size is
		 * 8 row (64 bytes) smaller
		 */
		oi->size = 1 << (((cap & CC_CAP_OTPSIZE) >> CC_CAP_OTPSIZE_SHIFT)
			+ CC_CAP_OTPSIZE_BASE);
		if (oi->ccrev >= 18)
			oi->size -= ((OTP_RC0_OFF - OTP_BOUNDARY_OFF) * 2);

		oi->hwprot = (int)(R_REG(osh, &cc->otpstatus) & OTPS_PROTECT);
		oi->boundary = -1;

		/* Check the region signature */
		if (hndotp_otproff(oi, cc, OTP_HWSIGN_OFF) == OTP_SIGNATURE) {
			oi->signvalid |= OTP_HW_REGION;
			oi->boundary = hndotp_otproff(oi, cc, OTP_BOUNDARY_OFF);
		}

		if (hndotp_otproff(oi, cc, OTP_SWSIGN_OFF) == OTP_SIGNATURE)
			oi->signvalid |= OTP_SW_REGION;

		if (hndotp_otproff(oi, cc, OTP_CIDSIGN_OFF) == OTP_SIGNATURE)
			oi->signvalid |= OTP_CID_REGION;

		/* Set OTP clkdiv for stability */
		if (oi->ccrev == 22)
			otpdiv = 12;

		if (otpdiv) {
			clkdiv = R_REG(osh, &cc->clkdiv);
			clkdiv = (clkdiv & ~CLKD_OTP) | (otpdiv << CLKD_OTP_SHIFT);
			W_REG(osh, &cc->clkdiv, clkdiv);
			OTP_MSG(("%s: set clkdiv to %x\n", __FUNCTION__, clkdiv));
		}
		OSL_DELAY(10);

		ret = (void *)oi;
	}

	OTP_MSG(("%s: ccrev %d\tsize %d bytes\thwprot %x\tsignvalid %x\tboundary %x\n",
		__FUNCTION__, oi->ccrev, oi->size, oi->hwprot, oi->signvalid,
		oi->boundary));

out:	/* All done */
	si_setcoreidx(sih, idx);

	return ret;
}

static int
hndotp_read_region(void *oh, int region, uint16 *data, uint *wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 idx, st;
	chipcregs_t *cc;
	int i;

	/* Only support HW region (no active chips use HND OTP SW region) */
	ASSERT(region == OTP_HW_REGION);

	OTP_MSG(("%s: region %x wlen %d\n", __FUNCTION__, region, *wlen));

	/* Region empty? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & region) == 0)
		return BCME_NOTFOUND;

	*wlen = ((int)*wlen < oi->boundary/2) ? *wlen : (uint)oi->boundary/2;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	for (i = 0; i < (int)*wlen; i++)
		data[i] = hndotp_otpr(oh, cc, i);

	si_setcoreidx(oi->sih, idx);

	return 0;
}

static int
hndotp_nvread(void *oh, char *data, uint *len)
{
	int rc = 0;
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 base, bound, lim = 0, st;
	int i, chunk, gchunks, tsz = 0;
	uint32 idx;
	chipcregs_t *cc;
	uint offset;
	uint16 *rawotp = NULL;

	/* save the orig core */
	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	st = hndotp_status(oh);
	if (!(st & (OTP_HW_REGION | OTP_SW_REGION))) {
		OTP_ERR(("OTP not programmed\n"));
		rc = -1;
		goto out;
	}

	/* Read the whole otp so we can easily manipulate it */
	lim = hndotp_size(oh);
	if ((rawotp = MALLOC(si_osh(oi->sih), lim)) == NULL) {
		OTP_ERR(("Out of memory for rawotp\n"));
		rc = -2;
		goto out;
	}
	for (i = 0; i < (int)(lim / 2); i++)
		rawotp[i] = hndotp_otpr(oh, cc,  i);

	if ((st & OTP_HW_REGION) == 0) {
		OTP_ERR(("otp: hw region not written (0x%x)\n", st));

		/* This could be a programming failure in the first
		 * chunk followed by one or more good chunks
		 */
		for (i = 0; i < (int)(lim / 2); i++)
			if (rawotp[i] == OTP_MAGIC)
				break;

		if (i < (int)(lim / 2)) {
			base = i;
			bound = (i * 2) + rawotp[i + 1];
			OTP_MSG(("otp: trying chunk at 0x%x-0x%x\n", i * 2, bound));
		} else {
			OTP_MSG(("otp: unprogrammed\n"));
			rc = -3;
			goto out;
		}
	} else {
		bound = rawotp[(lim / 2) + OTP_BOUNDARY_OFF];

		/* There are two cases: 1) The whole otp is used as nvram
		 * and 2) There is a hardware header followed by nvram.
		 */
		if (rawotp[0] == OTP_MAGIC) {
			base = 0;
			if (bound != rawotp[1])
				OTP_MSG(("otp: Bound 0x%x != chunk0 len 0x%x\n", bound,
				         rawotp[1]));
		} else
			base = bound;
	}

	/* Find and copy the data */

	chunk = 0;
	gchunks = 0;
	i = base / 2;
	offset = 0;
	while ((i < (int)(lim / 2)) && (rawotp[i] == OTP_MAGIC)) {
		int dsz, rsz = rawotp[i + 1];

		if (((i * 2) + rsz) >= (int)lim) {
			OTP_MSG(("  bad chunk size, chunk %d, base 0x%x, size 0x%x\n",
			         chunk, i * 2, rsz));
			/* Bad length, try to find another chunk anyway */
			rsz = 6;
		}
		if (hndcrc16((uint8 *)&rawotp[i], rsz,
		             CRC16_INIT_VALUE) == CRC16_GOOD_VALUE) {
			/* Good crc, copy the vars */
			OTP_MSG(("  good chunk %d, base 0x%x, size 0x%x\n",
			         chunk, i * 2, rsz));
			gchunks++;
			dsz = rsz - 6;
			tsz += dsz;
			if (offset + dsz >= *len) {
				OTP_MSG(("Out of memory for otp\n"));
				goto out;
			}
			bcopy((char *)&rawotp[i + 2], &data[offset], dsz);
			offset += dsz;
			/* Remove extra null characters at the end */
			while (offset > 1 &&
			       data[offset - 1] == 0 && data[offset - 2] == 0)
				offset --;
			i += rsz / 2;
		} else {
			/* bad length or crc didn't check, try to find the next set */
			OTP_MSG(("  chunk %d @ 0x%x size 0x%x: bad crc, ",
			         chunk, i * 2, rsz));
			if (rawotp[i + (rsz / 2)] == OTP_MAGIC) {
				/* Assume length is good */
				i += rsz / 2;
			} else {
				while (++i < (int)(lim / 2))
					if (rawotp[i] == OTP_MAGIC)
						break;
			}
			if (i < (int)(lim / 2))
				OTP_MSG(("trying next base 0x%x\n", i * 2));
			else
				OTP_MSG(("no more chunks\n"));
		}
		chunk++;
	}

	OTP_MSG(("  otp size = %d, boundary = 0x%x, nv base = 0x%x\n", lim, bound, base));
	if (tsz != 0) {
		OTP_MSG(("  Found %d bytes in %d good chunks out of %d\n", tsz, gchunks, chunk));
	} else {
		OTP_MSG(("  No good chunks found out of %d\n", chunk));
	}

	*len = offset;

out:
	if (rawotp)
		MFREE(si_osh(oi->sih), rawotp, lim);
	si_setcoreidx(oi->sih, idx);

	return rc;
}

#ifdef BCMNVRAMW
#if defined(WLTEST)
static	uint st_n, st_s, st_hwm, pp_hwm;
#ifdef	OTP_FORCEFAIL
static	uint forcefail_bitcount = 0;
#endif /* OTP_FORCEFAIL */
#endif 

static int
hndotp_write_bit(void *oh, chipcregs_t *cc, int bn, bool bit, int no_retry)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint row, col, j, k;
	uint32 pwait, init_pwait, otpc, otpp, pst, st;
	osl_t *osh;

	osh = si_osh(oi->sih);
	ASSERT((bit >> 1) == 0);

#ifdef	OTP_FORCEFAIL
	OTP_MSG(("%s: [0x%x] = 0x%x\n", __FUNCTION__, wn * 2, data));
#endif

	/* This is bit-at-a-time writing, future cores may do word-at-a-time */
	if (oi->ccrev == 12) {
		otpc = 0x20000001;
		init_pwait = 0x00000200;
	} else if (oi->ccrev == 22) {
		otpc = 0x20000001;
		init_pwait = 0x00000400;
	} else {
		otpc = 0x20000001;
		init_pwait = 0x00004000;
	}

	pwait = init_pwait;
	row = bn / 65;
	col = bn % 65;
	otpp = OTPP_START_BUSY |
		((bit << OTPP_VALUE_SHIFT) & OTPP_VALUE_MASK) |
		((row << OTPP_ROW_SHIFT) & OTPP_ROW_MASK) |
		(col & OTPP_COL_MASK);
	j = 0;
	while (1) {
		j++;
		if (j > 1) {
			OTP_DBG(("row %d, col %d, val %d, otpc 0x%x, otpp 0x%x\n",
				row, col, bit, (otpc | pwait), otpp));
		}

		W_REG(osh, &cc->otpcontrol, otpc | pwait);
		W_REG(osh, &cc->otpprog, otpp);
		pst = R_REG(osh, &cc->otpprog);
		for (k = 0; ((pst & OTPP_START_BUSY) == OTPP_START_BUSY) && (k < OTPP_TRIES); k++)
			pst = R_REG(osh, &cc->otpprog);
#if defined(WLTEST)
		if (k > pp_hwm)
			pp_hwm = k;
#endif 
		if (k >= OTPP_TRIES) {
			OTP_ERR(("BUSY stuck: pst=0x%x, count=%d\n", pst, k));
			st = OTPS_PROGFAIL;
			break;
		}
		st = R_REG(osh, &cc->otpstatus);
		if (((st & OTPS_PROGFAIL) == 0) || (pwait == OTPC_PROGWAIT) || (no_retry)) {
			break;
		} else {
			if ((oi->ccrev == 12) || (oi->ccrev == 22))
				pwait = (pwait << 3) & OTPC_PROGWAIT;
			else
				pwait = (pwait << 1) & OTPC_PROGWAIT;
			if (pwait == 0)
				pwait = OTPC_PROGWAIT;
		}
	}
#if defined(WLTEST)
	st_n++;
	st_s += j;
	if (j > st_hwm)
		 st_hwm = j;
#ifdef	OTP_FORCEFAIL
	if (forcefail_bitcount++ == OTP_FORCEFAIL * 16) {
		OTP_DBG(("Forcing PROGFAIL on bit %d (FORCEFAIL = %d/0x%x)\n",
			forcefail_bitcount, OTP_FORCEFAIL, OTP_FORCEFAIL));
		st = OTPS_PROGFAIL;
	}
#endif
#endif 
	if (st & OTPS_PROGFAIL) {
		OTP_ERR(("After %d tries: otpc = 0x%x, otpp = 0x%x/0x%x, otps = 0x%x\n",
		       j, otpc | pwait, otpp, pst, st));
		OTP_ERR(("otp prog failed. bit=%d, ppret=%d, ret=%d\n", bit, k, j));
		return 1;
	}

	return 0;
}

static int
hndotp_write_word(void *oh, chipcregs_t *cc, int wn, uint16 data)
{
	uint base, i;
	int err = 0;

	OTP_MSG(("%s: wn %d data %x\n", __FUNCTION__, wn, data));

	/* There is one test bit for each row */
	base = (wn * 16) + (wn / 4);

	for (i = 0; i < 16; i++) {
		err += hndotp_write_bit(oh, cc, base + i, data & 1, 0);
		data >>= 1;
		/* abort write after first error to avoid stress the charge-pump */
		if (err) {
			OTP_DBG(("%s: wn %d fail on bit %d\n", __FUNCTION__, wn, i));
			break;
		}
	}

	return err;
}

static int
hndotp_valid_rce(void *oh, chipcregs_t *cc, int i)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	osl_t *osh;
	uint32 hwv, fw, rce, e, sign, row, st;

	ASSERT(oi->ccrev >= 18);

	/* HW valid bit */
	osh = si_osh(oi->sih);
	st = R_REG(osh, &cc->otpstatus);
	hwv = (st & OTPS_RCEV_MSK) & (1 << (OTPS_RCEV_SHIFT + i));

	if (i < 3) {
		e = i;
		fw = hndotp_size(oh)/2 + OTP_RC0_OFF + e;
	} else {
		e = i - 3;
		fw = hndotp_size(oh)/2 + OTP_RC1_OFF + e;
	}

	rce = hndotp_otpr(oh, cc, fw+1) << 16 | hndotp_otpr(oh, cc, fw);
	rce >>= ((e * OTP_RCE_BITS) + OTP_RCE_BIT0 - (e * 16));
	row = rce & OTP_RCE_ROW_MASK;
	sign = (rce >> OTP_RCE_ROW_SZ) & OTP_RCE_SIGN_MASK;

	OTP_MSG(("rce %d sign %x row %d hwv %x\n", i, sign, row, hwv));

	return (sign == OTP_SIGNATURE) ? row : -1;
}

static int
hndotp_write_rce(void *oh, chipcregs_t *cc, int r, uint16* data)
{
	int i, rce = -1;
	uint32	sign;

	ASSERT(((otpinfo_t *)oh)->ccrev >= 18);
	ASSERT(r >= 0 && r < hndotp_size(oh)/(2*OTP_WPR));
	ASSERT(data);

	for (rce = OTP_RCE_ROW_SZ -1; rce >= 0; rce--) {
		int e, rt, rcr, bit, err = 0;

		int rr = hndotp_valid_rce(oh, cc, rce);
		/* redundancy row in use already */
		if (rr != -1) {
			if (rr == r) {
				OTP_MSG(("%s: row %d already replaced by RCE %d",
					__FUNCTION__, r, rce));
				return 0;
			}

			continue; /* If row used, go for the next row */
		}

		/*  
		 * previously used bad rce entry maybe treaed as valid rce and used again, abort on
		 * first bit error to avoid stress the charge pump
		 */

		/* Write the data to the redundant row */
		for (i = 0; i < OTP_WPR; i++) {
			err += hndotp_write_word(oh, cc, hndotp_size(oh)/2+OTP_RD_OFF+rce*4+i,
				data[i]);
			if (err) {
				OTP_MSG(("fail to write redundant row %d\n", rce));
				break;
			}
		}

		/* Now write the redundant row index */
		if (rce < 3) {
			e = rce;
			rcr = hndotp_size(oh)/2 + OTP_RC0_OFF;
		} else {
			e = rce - 3;
			rcr = hndotp_size(oh)/2 + OTP_RC1_OFF;
		}

		/* Write row numer bit-by-bit */
		bit = (rcr * 16 + rcr / 4) + e * OTP_RCE_BITS + OTP_RCE_BIT0;
		rt = r;
		for (i = 0; i < OTP_RCE_ROW_SZ; i++) {
			/* If any timeout happened, invalidate the subsequent bits with 0 */
			if (hndotp_write_bit(oh, cc, bit, (rt & (err ? 0 : 1)), err)) {
				OTP_MSG(("%s: timeout fixing row %d with RCE %d - at row"
					" number bit %x\n", __FUNCTION__, r, rce, i));
				err++;
			}
			rt >>= 1;
			bit ++;
		}

		/* Write the RCE signature bit-by-bit */
		sign = OTP_SIGNATURE;
		for (i = 0; i < OTP_RCE_SIGN_SZ; i++) {
			/* If any timeout happened, invalidate the subsequent bits with 0 */
			if (hndotp_write_bit(oh, cc, bit, (sign & (err ? 0 : 1)), err)) {
				OTP_MSG(("%s: timeout fixing row %d with RCE %d - at row"
					" number bit %x\n", __FUNCTION__, r, rce, i));
				err++;
			}
			sign >>= 1;
			bit ++;
		}

		if (err) {
			OTP_ERR(("%s: row %d not fixed by RCE %d due to %d timeouts. try next"
				" RCE\n", __FUNCTION__, r, rce, err));
			continue;
		} else {
			OTP_MSG(("%s: Fixed row %d by RCE %d\n", __FUNCTION__, r, rce));
			return BCME_OK;
		}
	}

	OTP_ERR(("All RCE's are in use. Failed fixing OTP.\n"));
	/* Fatal error, unfixable. MFGC will have to fail. Board needs to be discarded!!  */
	return BCME_NORESOURCE;
}

/* Write a row and fix it with RCE if any error detected */
static int
hndotp_write_row(void *oh, chipcregs_t *cc, int wn, uint16* data, bool rewrite)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	int err = 0, i;

	ASSERT(wn % OTP_WPR == 0);

	/* Write the data */
	for (i = 0; i < OTP_WPR; i++) {
		if (rewrite && (data[i] == hndotp_otpr(oh, cc, wn+i)))
			continue;

		err += hndotp_write_word(oh, cc, wn + i, data[i]);
	}

	/* Fix this row if any error */
	if (err && (oi->ccrev >= 18)) {
		OTP_DBG(("%s: %d write errors in row %d. Fixing...\n", __FUNCTION__, err, wn/4));
		if ((err = hndotp_write_rce(oh, cc, wn / OTP_WPR, data)))
			OTP_MSG(("%s: failed to fix row %d\n", __FUNCTION__, wn/4));
	}

	return err;
}

/* expects the caller to disable interrupts before calling this routine */
static int
hndotp_write_region(void *oh, int region, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 st;
	uint wn, base = 0, lim;
	int ret = BCME_OK;
	uint idx;
	chipcregs_t *cc;
	bool rewrite = FALSE;
	uint32	save_clk;

	ASSERT(wlen % OTP_WPR == 0);

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Check valid region */
	if ((region != OTP_HW_REGION) &&
	    (region != OTP_SW_REGION) &&
	    (region != OTP_CID_REGION)) {
		ret = BCME_BADARG;
		goto out;
	}

	/* Region already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & region) != 0)
		rewrite = TRUE;

	/* HW and CID have to be written before SW */
	if ((((st & (OTP_HW_REGION | OTP_CID_REGION)) == 0) &&
		(st & OTP_SW_REGION) != 0)) {
		OTP_ERR(("%s: HW/CID region should be programmed first\n", __FUNCTION__));
		ret = BCME_BADARG;
		goto out;
	}

	/* Bounds for the region */
	lim = (oi->size / 2) + OTP_SWLIM_OFF;
	if (region == OTP_HW_REGION) {
		base = 0;
	} else if (region == OTP_SW_REGION) {
		base = oi->boundary / 2;
	} else if (region == OTP_CID_REGION) {
		base = (oi->size / 2) + OTP_CID_OFF;
		lim = (oi->size / 2) + OTP_LIM_OFF;
	}

	if (wlen > (lim - base)) {
		ret = BCME_BUFTOOLONG;
		goto out;
	}
	lim = base + wlen;

#if defined(WLTEST)
	st_n = st_s = st_hwm = pp_hwm = 0;
#endif 

	/* force ALP for progrramming stability */
	save_clk = R_REG(oi->osh, &cc->clk_ctl_st);
	OR_REG(oi->osh, &cc->clk_ctl_st, CCS_FORCEALP);
	OSL_DELAY(10);

	/* Write the data row by row */
	for (wn = base; wn < lim; wn += OTP_WPR, data += OTP_WPR) {
		if ((ret = hndotp_write_row(oh, cc, wn, data, rewrite)) != 0) {
			if (ret == BCME_NORESOURCE) {
				OTP_ERR(("%s: Abort at word %x\n", __FUNCTION__, wn));
				break;
			}
		}
	}

	/* Don't need to update signature & boundary if rewrite */
	if (rewrite)
		goto out_rclk;

	/* Done with the data, write the signature & boundary if needed */
	if (region == OTP_HW_REGION) {
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_BOUNDARY_OFF, lim * 2) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_HWSIGN_OFF,
			OTP_SIGNATURE) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		oi->boundary = lim * 2;
		oi->signvalid |= OTP_HW_REGION;
	} else if (region == OTP_SW_REGION) {
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_SWSIGN_OFF,
			OTP_SIGNATURE) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		oi->signvalid |= OTP_SW_REGION;
	} else if (region == OTP_CID_REGION) {
		if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_CIDSIGN_OFF,
			OTP_SIGNATURE) != 0) {
			ret = BCME_NORESOURCE;
			goto out_rclk;
		}
		oi->signvalid |= OTP_CID_REGION;
	}

out_rclk:
	/* Restore clock */
	W_REG(oi->osh, &cc->clk_ctl_st, save_clk);

out:
#if defined(WLTEST)
	OTP_MSG(("bits written: %d, average (%d/%d): %d, max retry: %d, pp max: %d\n",
		st_n, st_s, st_n, st_n?(st_s / st_n):0, st_hwm, pp_hwm));
#endif

	si_setcoreidx(oi->sih, idx);

	return ret;
}

/* For HND OTP, there's no space for appending after filling in SROM image */
static int
hndotp_cis_append_region(si_t *sih, int region, char *vars, int count)
{
	return otp_write_region(sih, region, (uint16*)vars, count/2);
}

/* Fill all unwritten RCE signature with 0 and return the number of them */
static int
hndotp_lock(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	int i, j, e, rcr, bit, ret = 0;
	uint32 st, idx;
	chipcregs_t *cc;

	ASSERT(oi->ccrev >= 18);

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Region already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & (OTP_HW_REGION | OTP_SW_REGION)) == 0) {
		si_setcoreidx(oi->sih, idx);
		return BCME_NOTREADY;	/* Don't lock unprogrammed OTP */
	}

	/* Find the highest valid RCE */
	for (i = 0; i < OTP_RCE_ROW_SZ -1; i++) {
		if ((hndotp_valid_rce(oh, cc, i) != -1))
			break;
	}
	i--;	/* Start invalidating from the next RCE */

	for (; i >= 0; i--) {
		if ((hndotp_valid_rce(oh, cc, i) == -1)) {

			ret++;	/* This is a unprogrammed row */

			/* Invalidate the row with 0 */
			if (i < 3) {
				e = i;
				rcr = hndotp_size(oh)/2 + OTP_RC0_OFF;
			} else {
				e = i - 3;
				rcr = hndotp_size(oh)/2 + OTP_RC1_OFF;
			}

			/* Fill row numer and signature with 0 bit-by-bit */
			bit = (rcr * 16 + rcr / 4) + e * OTP_RCE_BITS + OTP_RCE_BIT0;
			for (j = 0; j < (OTP_RCE_ROW_SZ + OTP_RCE_SIGN_SZ); j++) {
				hndotp_write_bit(oh, cc, bit, 0, 1);
				bit ++;
			}

			OTP_MSG(("locking rce %d\n", i));
		}
	}

	si_setcoreidx(oi->sih, idx);

	return ret;
}

/* expects the caller to disable interrupts before calling this routine */
static int
hndotp_nvwrite(void *oh, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint32 st;
	uint16 crc, clen, *p, hdr[2];
	uint wn, base = 0, lim;
	int err, gerr = 0;
	uint idx;
	chipcregs_t *cc;

	/* otp already written? */
	st = oi->hwprot | oi-> signvalid;
	if ((st & (OTP_HW_REGION | OTP_SW_REGION)) == (OTP_HW_REGION | OTP_SW_REGION))
		return BCME_EPERM;

	/* save the orig core */
	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* Bounds for the region */
	lim = (oi->size / 2) + OTP_SWLIM_OFF;
	base = 0;

	/* Look for possible chunks from the end down */
	wn = lim;
	while (wn > 0) {
		wn--;
		if (hndotp_otpr(oh, cc, wn) == OTP_MAGIC) {
			base = wn + (hndotp_otpr(oh, cc, wn + 1) / 2);
			break;
		}
	}
	if (base == 0) {
		OTP_MSG(("Unprogrammed otp\n"));
	} else {
		OTP_MSG(("Found some chunks, skipping to 0x%x\n", base * 2));
	}
	if ((wlen + 3) > (lim - base)) {
		err =  BCME_NORESOURCE;
		goto out;
	}

#if defined(WLTEST)
	st_n = st_s = st_hwm = pp_hwm = 0;
#endif 

	/* Prepare the header and crc */
	hdr[0] = OTP_MAGIC;
	hdr[1] = (wlen + 3) * 2;
	crc = hndcrc16((uint8 *)hdr, sizeof(hdr), CRC16_INIT_VALUE);
	crc = hndcrc16((uint8 *)data, wlen * 2, crc);
	crc = ~crc;

	do {
		p = data;
		wn = base + 2;
		lim = base + wlen + 2;

		OTP_MSG(("writing chunk, 0x%x bytes @ 0x%x-0x%x\n", wlen * 2,
		         base * 2, (lim + 1) * 2));

		/* Write the header */
		err = hndotp_write_word(oh, cc, base, hdr[0]);

		/* Write the data */
		while (wn < lim) {
			err += hndotp_write_word(oh, cc, wn++, *p++);

			/* If there has been an error, close this chunk */
			if (err != 0) {
				OTP_MSG(("closing early @ 0x%x\n", wn * 2));
				break;
			}
		}

		/* If we wrote the whole chunk, write the crc */
		if (wn == lim) {
			OTP_MSG(("  whole chunk written, crc = 0x%x\n", crc));
			err += hndotp_write_word(oh, cc, wn++, crc);
			clen = hdr[1];
		} else {
			/* If there was an error adjust the count to point to
			 * the word after the error so we can start the next
			 * chunk there.
			 */
			clen = (wn - base) * 2;
			OTP_MSG(("  partial chunk written, chunk len = 0x%x\n", clen));
		}
		/* And now write the chunk length */
		err += hndotp_write_word(oh, cc, base + 1, clen);

		if (base == 0) {
			/* Write the signature and boundary if this is the HW region,
			 * but don't report failure if either of these 2 writes fail.
			 */
			if (hndotp_write_word(oh, cc, (oi->size / 2) + OTP_BOUNDARY_OFF,
			    wn * 2) == 0)
				gerr += hndotp_write_word(oh, cc, (oi->size / 2) + OTP_HWSIGN_OFF,
				                       OTP_SIGNATURE);
			else
				gerr++;
			oi->boundary = wn * 2;
			oi->signvalid |= OTP_HW_REGION;
		}

		if (err != 0) {
			gerr += err;
			/* Errors, do it all over again if there is space left */
			if ((wlen + 3) <= ((oi->size / 2) + OTP_SWLIM_OFF - wn)) {
				base = wn;
				lim = base + wlen + 2;
				OTP_ERR(("Programming errors, retry @ 0x%x\n", wn * 2));
			} else {
				OTP_ERR(("Programming errors, no space left ( 0x%x)\n", wn * 2));
				break;
			}
		}
	} while (err != 0);

	OTP_MSG(("bits written: %d, average (%d/%d): %d, max retry: %d, pp max: %d\n",
	       st_n, st_s, st_n, st_s / st_n, st_hwm, pp_hwm));

	if (gerr != 0)
		OTP_MSG(("programming %s after %d errors\n", (err == 0) ? "succedded" : "failed",
		         gerr));
out:
	/* done */
	si_setcoreidx(oi->sih, idx);

	if (err)
		return BCME_ERROR;
	else
		return 0;
}
#endif /* BCMNVRAMW */

#if defined(WLTEST)
static uint16
hndotp_otprb16(void *oh, chipcregs_t *cc, uint wn)
{
	uint base, i;
	uint16 val, bit;

	base = (wn * 16) + (wn / 4);
	val = 0;
	for (i = 0; i < 16; i++) {
		if ((bit = hndotp_read_bit(oh, cc, base + i)) == 0xffff)
			break;
		val = val | (bit << i);
	}
	if (i < 16)
		val = 0xaaaa;
	return val;
}

static int
hndotp_dump(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	chipcregs_t *cc;
	uint idx, i, count, lil;
	uint16 val;
	struct bcmstrbuf b;

	idx = si_coreidx(oi->sih);
	cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	if (arg >= 16)
		arg -= 16;

	if (arg == 2) {
		count = 66 * 4;
		lil = 3;
	} else {
		count = (oi->size / 2) + OTP_RC_LIM_OFF;
		lil = 7;
	}

	OTP_MSG(("%s: arg %d, size %d, words %d\n", __FUNCTION__, arg, size, count));
	bcm_binit(&b, buf, size);
	for (i = 0; i < count; i++) {
		if ((i & lil) == 0)
			bcm_bprintf(&b, "0x%04x:", 2 * i);

		if (arg == 0)
			val = hndotp_otpr(oh, cc, i);
		else
			val = hndotp_otprb16(oi, cc, i);
		bcm_bprintf(&b, " 0x%04x", val);
		if ((i & lil) == lil) {
			if (arg == 2) {
				bcm_bprintf(&b, " %d\n",
				            hndotp_read_bit(oh, cc, ((i / 4) * 65) + 64) & 1);
			} else {
				bcm_bprintf(&b, "\n");
			}
		}
	}
	if ((i & lil) != lil)
		bcm_bprintf(&b, "\n");

	OTP_MSG(("%s: returning %d, left %d, wn %d\n",
		__FUNCTION__, (int)(b.buf - b.origbuf), b.size, i));

	si_setcoreidx(oi->sih, idx);

	return ((int)(b.buf - b.origbuf));
}
#endif	

static otp_fn_t hndotp_fn = {
	(otp_status_t)hndotp_status,
	(otp_size_t)hndotp_size,
	(otp_read_bit_t)hndotp_read_bit,
	(otp_init_t)hndotp_init,
	(otp_read_region_t)hndotp_read_region,
	(otp_nvread_t)hndotp_nvread,
#ifdef BCMNVRAMW
	(otp_write_region_t)hndotp_write_region,
	(otp_cis_append_region_t)hndotp_cis_append_region,
	(otp_lock_t)hndotp_lock,
	(otp_nvwrite_t)hndotp_nvwrite,
#endif /* BCMNVRAMW */
#if defined(WLTEST)
	(otp_dump_t)hndotp_dump,
#endif	
	OTP_FN_MAGIC
};

#endif	/* BCMAUTOOTP || BCMHNDOTP */


/*
 * Common Code: Compiled for IPX / HND / AUTO
 *	otp_status()
 *	otp_size()
 *	otp_read_bit()
 *	otp_init()
 * 	otp_read_region()
 * 	otp_nvread()
 * 	otp_write_region()
 * 	otp_cis_append_region()
 * 	otp_lock()
 * 	otp_nvwrite()
 * 	otp_dump()
 *	otp_dumpstats()
 */

int
otp_status(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->status(oh);
}

int
otp_size(void *oh)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->size(oh);
}

uint16
otp_read_bit(void *oh, uint offset)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	uint idx = si_coreidx(oi->sih);
	chipcregs_t *cc = si_setcoreidx(oi->sih, SI_CC_IDX);
	uint16 readBit = (uint16)oi->fn->read_bit(oh, cc, offset);
	si_setcoreidx(oi->sih, idx);
	return readBit;
}

void *
BCMNMIATTACHFN(otp_init)(si_t *sih)
{
	otpinfo_t *oi;
	void *ret = NULL;

	oi = &otpinfo;
	bzero(oi, sizeof(otpinfo_t));

	oi->ccrev = sih->ccrev;

#if defined(BCMAUTOOTP)
	OTP_MSG(("%s: Compiled for AUTO OTP\n", __FUNCTION__));
	/* Auto-select based on chipc rev */
	if (oi->ccrev < 21 || oi->ccrev == 22) {	/* HND OTP */
		oi->fn = &hndotp_fn;
	} else {					/* IPX OTP */
		oi->fn = &ipxotp_fn;
	}
#else	/* BCMAUTOOTP - no auto-selection */
#if !defined(BCMHNDOTP)	    /* Newer IPX OTP wrapper */
	OTP_MSG(("%s: Compiled for IPX OTP\n", __FUNCTION__));
	oi->fn = &ipxotp_fn;
#else	/* !BCMHNDOTP - Older HND OTP controller */
	OTP_MSG(("%s: Compiled for HND OTP\n", __FUNCTION__));
	oi->fn = &hndotp_fn;
#endif
#endif	/* BCMAUTOOTP */
	ASSERT(oi->fn->magic == OTP_FN_MAGIC);

	oi->sih = sih;
	oi->osh = si_osh(oi->sih);

	ret = (oi->fn->init)(sih);

	return ret;
}

int
BCMNMIATTACHFN(otp_read_region)(si_t *sih, int region, uint16 *data, uint *wlen)
{
	bool wasup = FALSE;
	void *oh;
	int err = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		err = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		err = BCME_ERROR;
		goto out;
	}

	err = (((otpinfo_t*)oh)->fn->read_region)(oh, region, data, wlen);

out:
	if (!wasup)
		si_otp_power(sih, FALSE);

	return err;
}

int
otp_nvread(void *oh, char *data, uint *len)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->nvread(oh, data, len);
}

#ifdef BCMNVRAMW
int
BCMNMIATTACHFN(otp_write_region)(si_t *sih, int region, uint16 *data, uint wlen)
{
	bool wasup = FALSE;
	void *oh;
	int err = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		err = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		err = BCME_ERROR;
		goto out;
	}

	err = (((otpinfo_t*)oh)->fn->write_region)(oh, region, data, wlen);

out:
	if (!wasup)
		si_otp_power(sih, FALSE);

	return err;
}

int
otp_cis_append_region(si_t *sih, int region, char *vars, int count)
{
	void *oh = otp_init(sih);

	return (((otpinfo_t*)oh)->fn->cis_append_region)(sih, region, vars, count);
}

int
otp_lock(si_t *sih)
{
	bool wasup = FALSE;
	void *oh;
	int ret = 0;

	if (!(wasup = si_is_otp_powered(sih)))
		si_otp_power(sih, TRUE);

	if (!si_is_otp_powered(sih) || si_is_otp_disabled(sih)) {
		ret = BCME_NOTREADY;
		goto out;
	}

	oh = otp_init(sih);
	if (oh == NULL) {
		OTP_ERR(("otp_init failed.\n"));
		ret = BCME_ERROR;
		goto out;
	}

	ret = (((otpinfo_t*)oh)->fn->lock)(oh);

out:
	if (!wasup)
		si_otp_power(sih, FALSE);

	return ret;
}

int
otp_nvwrite(void *oh, uint16 *data, uint wlen)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->nvwrite(oh, data, wlen);
}
#endif /* BCMNVRAMW */

#if defined(WLTEST)
int
otp_dump(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;

	return oi->fn->dump(oh, arg, buf, size);
}

int
otp_dumpstats(void *oh, int arg, char *buf, uint size)
{
	otpinfo_t *oi = (otpinfo_t *)oh;
	struct bcmstrbuf b;

	bcm_binit(&b, buf, size);

	bcm_bprintf(&b, "\nOTP, ccrev 0x%04x\n", oi->ccrev);
#if defined(BCMAUTOOTP) || !defined(BCMHNDOTP)	    /* Newer IPX OTP wrapper */
	bcm_bprintf(&b, "wsize %d rows %d cols %d\n", oi->wsize, oi->rows, oi->cols);
	bcm_bprintf(&b, "hwbase %d hwlim %d swbase %d swlim %d\n", oi->hwbase, oi->hwlim,
		oi->swbase, oi->swlim, oi->fbase, oi->flim);
	bcm_bprintf(&b, "otpgu_base %d status %d\n", oi->otpgu_base, oi->status);
#endif
#if defined(BCMAUTOOTP) || defined(BCMHNDOTP)	    /* Older HND OTP wrapper */
	bcm_bprintf(&b, "OLD OTP, size %d hwprot 0x%x signvalid 0x%x boundary %d\n",
		oi->size, oi->hwprot, oi->signvalid, oi->boundary);
#endif
	bcm_bprintf(&b, "\n");

	return 200;	/* real buf length, pick one to cover above print */
}

#endif 
