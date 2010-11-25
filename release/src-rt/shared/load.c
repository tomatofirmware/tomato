/*
 * Initialization and support routines for self-booting
 * compressed image.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: load.c,v 1.19.2.1 2009/02/11 10:07:28 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndmips.h>
#include <sbmemc.h>
#include <bcmsrom.h>

void c_main(unsigned long ra);

static si_t *sih;
static chipcregs_t *cc;


extern unsigned char text_start[], text_end[];
extern unsigned char data_start[], data_end[];
extern char bss_start[], bss_end[];

#define INBUFSIZ 4096	/* Buffer size */
#define WSIZE 0x8000    	/* window size--must be a power of two, and */
				/*  at least 32K for zip's deflate method */

static uchar *inbuf;		/* input buffer */
#if !defined(USE_LZMA)
static ulong insize;		/* valid bytes in inbuf */
static ulong inptr;		/* index of next byte to be processed in inbuf */
#endif /* USE_GZIP */

static uchar *outbuf;		/* output buffer */
static ulong bytes_out;		/* valid bytes in outbuf */

static ulong inoff;		/* offset of input data */

#if !defined(USE_LZMA)
static int
fill_inbuf(void)
{
	int bytes;

	for (insize = 0; insize < INBUFSIZ; insize += bytes, inoff += bytes) {
		*((uint32 *) &inbuf[insize]) = *((uint32 *) KSEG1ADDR(SI_FLASH1 + inoff));
		bytes = sizeof(uint32);
	}

	inptr = 1;

	return inbuf[0];
}

/* Defines for gzip/bzip */
#define	malloc(size)	MALLOC(NULL, size)
#define	free(addr)	MFREE(NULL, addr, 0)

static void
error(char *x)
{
	printf("\n\n%s\n\n -- System halted", x);

	for (;;);
}
#endif /* USE_LZMA */

#if defined(USE_GZIP)

/*
 * gzip declarations
 */

#define OF(args) args
#define STATIC static

#define memzero(s, n)	memset ((s), 0, (n))

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* Diagnostic functions (stubbed out) */

#define Assert(cond, msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c, x)
#define Tracecv(c, x)

static uchar *window;		/* Sliding window buffer */
static unsigned outcnt;		/* bytes in window buffer */

static void
gzip_mark(void **ptr)
{
	/* I'm not sure what the pourpose of this is, there are no malloc
	 * calls without free's in the gunzip code.
	 */
}

static void
gzip_release(void **ptr)
{
}

static void flush_window(void);

#include "gzip_inflate.c"

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
static void
flush_window(void)
{
	ulg c = crc;
	unsigned n;
	uch *in, *out, ch;

	in = window;
	out = &outbuf[bytes_out];
	for (n = 0; n < outcnt; n++) {
		ch = *out++ = *in++;
		c = crc_32_tab[((int)c ^ ch) & 0xff] ^ (c >> 8);
	}
	crc = c;
	bytes_out += (ulg)outcnt;
	outcnt = 0;
	putc('.');
}

#elif defined(USE_BZIP2)

#include "bzip2_inflate.c"

/*
 * bzip2 declarations
 */

void bz_internal_error(int i)
{
	char msg[128];

	sprintf(msg, "Bzip2 internal error: %d", i);
	error(msg);
}

static int
bunzip2(void)
{
	bz_stream bzstream;
	int ret = 0;

	bzstream.bzalloc = 0;
	bzstream.bzfree = 0;
	bzstream.opaque = 0;
	bzstream.avail_in = 0;

	if ((ret = BZ2_bzDecompressInit(&bzstream, 0, 1)) != BZ_OK)
		return ret;

	for (;;) {
		if (bzstream.avail_in == 0) {
			fill_inbuf();
			bzstream.next_in = inbuf;
			bzstream.avail_in = insize;
		}
		bzstream.next_out = &outbuf[bytes_out];
		bzstream.avail_out = WSIZE;
		if ((ret = BZ2_bzDecompress(&bzstream)) != BZ_OK)
			break;
		bytes_out = bzstream.total_out_lo32;
		putc('.');
	}

	if (ret == BZ_STREAM_END)
		ret = BZ2_bzDecompressEnd(&bzstream);

	if (ret == BZ_OK)
		ret = 0;

	return ret;
}
#elif defined(USE_LZMA)

#include "LzmaDec.c"
#define LZMA_HEADER_SIZE (LZMA_PROPS_SIZE + 8)

static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

extern int _memsize;
/*
 * call LZMA decoder to decompress a LZMA block
 */
static int
decompressLZMA(unsigned char *src, unsigned int srcLen, unsigned char *dest, unsigned int destLen)
{
	int res;
	SizeT inSizePure;
	ELzmaStatus status;
	SizeT outSize;

	if (srcLen < LZMA_HEADER_SIZE)
		return SZ_ERROR_INPUT_EOF;

	inSizePure = srcLen - LZMA_HEADER_SIZE;
	outSize = destLen;
	res = LzmaDecode(dest, &outSize, src + LZMA_HEADER_SIZE, &inSizePure,
	                 src, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &g_Alloc);
	srcLen = inSizePure + LZMA_HEADER_SIZE;

	if ((res == SZ_OK) ||
		((res == SZ_ERROR_INPUT_EOF) && (srcLen == inSizePure + LZMA_HEADER_SIZE)))
		res = 0;
	return res;
}

#endif /* defined(USE_GZIP) */

extern char input_data[];
extern int input_len;

static void
load(void)
{
	int ret = 0;
	uint32 *inbase;

	/* Offset from beginning of flash */
#ifdef	CONFIG_XIP
	inoff = ((ulong)text_end - (ulong)text_start) + ((ulong)input_data - (ulong)data_start);
#else
	inoff = (ulong) input_data - (ulong) text_start;
#endif /* CONFIG_XIP */
	inbase = (uint32 *) KSEG1ADDR(SI_FLASH1 + inoff);
	outbuf = (uchar *) LOADADDR;
	bytes_out = 0;
	inbuf = malloc(INBUFSIZ);	/* input buffer */

#if defined(USE_GZIP)
	window = malloc(WSIZE);
	printf("Decompressing...");
	makecrc();
	ret = gunzip();
#elif defined(USE_BZIP2)
	/* Small decompression algorithm uses up to 2300k of memory */
	printf("Decompressing...");
	ret = bunzip2();
#elif defined(USE_LZMA)
	printf("Decompressing...");
	bytes_out = (ulong)_memsize - (ulong)PHYSADDR(outbuf);
	ret = decompressLZMA((unsigned char *)inbase, input_len, outbuf, bytes_out);
#else
	printf("Copying...");
	while (bytes_out < input_len) {
		fill_inbuf();
		memcpy(&outbuf[bytes_out], inbuf, insize);
		bytes_out += insize;
	}
#endif /* defined(USE_GZIP) */
	if (ret) {
		printf("error %d\n", ret);
	} else
		printf("done\n");
}

void
c_main(unsigned long ra)
{
	BCMDBG_TRACE(0x4c4400);

#ifndef CFG_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif /* CFG_UNCACHED */

	BCMDBG_TRACE(0x4c4401);

	/* Basic initialization */
	sih = (si_t *)osl_init();

	BCMDBG_TRACE(0x4c4402);

	cc = si_setcoreidx(sih, SI_CC_IDX);

	BCMDBG_TRACE(0x4c4403);

	/* Load binary */
	load();

	BCMDBG_TRACE(0x4c4405);

	/* Flush all caches */
	blast_dcache();
	blast_icache();

	BCMDBG_TRACE(0x4c4406);

	/* Jump to load address */
	((void (*)(void)) LOADADDR)();
}
