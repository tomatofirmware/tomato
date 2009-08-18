/* precomp2.c -- example program: how to generate pre-compressed data

   This file is part of the LZO real-time data compression library.

   Copyright (C) 2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/opensource/lzo/
 */


/*************************************************************************
// This program shows how to generate pre-compressed data.
//
// Please study precomp.c first.
//
// We will be trying LZO1X-999 and LZO1Y-999, and we will be trying
// various parameters using the internal interface to squeeze out
// a little bit of extra compression.
//
// NOTE: this program can be quite slow for highly redundant files
**************************************************************************/

#include "lzo/lzoconf.h"
#include "lzo/lzo1x.h"
#include "lzo/lzo1y.h"

LZO_EXTERN(int)
lzo1x_999_compress_internal ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len,
                                    lzo_callback_p cb,
                                    int try_lazy,
                                    lzo_uint good_length,
                                    lzo_uint max_lazy,
                                    lzo_uint nice_length,
                                    lzo_uint max_chain,
                                    lzo_uint32 flags );

LZO_EXTERN(int)
lzo1y_999_compress_internal ( const lzo_bytep in , lzo_uint  in_len,
                                    lzo_bytep out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_bytep dict, lzo_uint dict_len,
                                    lzo_callback_p cb,
                                    int try_lazy,
                                    lzo_uint good_length,
                                    lzo_uint max_lazy,
                                    lzo_uint nice_length,
                                    lzo_uint max_chain,
                                    lzo_uint32 flags );

#define USE_LZO1X 1
#define USE_LZO1Y 1

#define PARANOID 1


/* portability layer */
#define WANT_LZO_MALLOC 1
#define WANT_LZO_FREAD 1
#define WANT_LZO_WILDARGV 1
#include "examples/portab.h"


/*************************************************************************
//
**************************************************************************/

int __lzo_cdecl_main main(int argc, char *argv[])
{
    int r;
    int lazy;
    const int max_try_lazy = 5;
    const lzo_uint big = 65536L;    /* can result in very slow compression */
    const lzo_uint32 flags = 0x1;

    lzo_bytep in;
    lzo_uint in_len;

    lzo_bytep out;
    lzo_uint out_bufsize;
    lzo_uint out_len = 0;

    lzo_bytep wrkmem;
    lzo_uint wrk_len;

    lzo_uint best_len;
    int best_compress = -1;
    int best_lazy = -1;

    lzo_uint orig_len;
    lzo_uint32 uncompressed_checksum;
    lzo_uint32 compressed_checksum;

    FILE *f;
    const char *progname = NULL;
    const char *in_name = NULL;
    const char *out_name = NULL;
    long l;


    lzo_wildargv(&argc, &argv);

    printf("\nLZO real-time data compression library (v%s, %s).\n",
           lzo_version_string(), lzo_version_date());
    printf("Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer\nAll Rights Reserved.\n\n");

    progname = argv[0];
    if (argc < 2 || argc > 3)
    {
        printf("usage: %s file [output-file]\n", progname);
        exit(1);
    }
    in_name = argv[1];
    if (argc > 2) out_name = argv[2];

/*
 * Step 1: initialize the LZO library
 */
    if (lzo_init() != LZO_E_OK)
    {
        printf("internal error - lzo_init() failed !!!\n");
        printf("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable `-DLZO_DEBUG' for diagnostics)\n");
        exit(1);
    }

/*
 * Step 2: allocate the work-memory
 */
    wrk_len = 1;
#ifdef USE_LZO1X
    if (wrk_len < LZO1X_999_MEM_COMPRESS)
        wrk_len = LZO1X_999_MEM_COMPRESS;
#endif
#ifdef USE_LZO1Y
    if (wrk_len < LZO1Y_999_MEM_COMPRESS)
        wrk_len = LZO1Y_999_MEM_COMPRESS;
#endif
    wrkmem = (lzo_bytep) lzo_malloc(wrk_len);
    if (wrkmem == NULL)
    {
        printf("%s: out of memory\n", progname);
        exit(1);
    }

/*
 * Step 3: open the input file
 */
    f = fopen(in_name,"rb");
    if (f == NULL)
    {
        printf("%s: cannot open file %s\n", progname, in_name);
        exit(1);
    }
    fseek(f,0,SEEK_END);
    l = ftell(f);
    fseek(f,0,SEEK_SET);
    if (l <= 0)
    {
        printf("%s: %s: empty file\n", progname, in_name);
        fclose(f);
        exit(1);
    }
    in_len = (lzo_uint) l;
    out_bufsize = in_len + in_len / 16 + 64 + 3;
    best_len = in_len;

/*
 * Step 4: allocate compression buffers and read the file
 */
    in = (lzo_bytep) lzo_malloc(in_len);
    out = (lzo_bytep) lzo_malloc(out_bufsize);
    if (in == NULL || out == NULL)
    {
        printf("%s: out of memory\n", progname);
        exit(1);
    }
    in_len = (lzo_uint) lzo_fread(f,in,in_len);
    printf("%s: loaded file %s: %ld bytes\n", progname, in_name, (long) in_len);
    fclose(f);

/*
 * Step 5: compute a checksum of the uncompressed data
 */
    uncompressed_checksum = lzo_adler32(0,NULL,0);
    uncompressed_checksum = lzo_adler32(uncompressed_checksum,in,in_len);

/*
 * Step 6a: compress from `in' to `out' with LZO1X-999
 */
#ifdef USE_LZO1X
    for (lazy = 0; lazy <= max_try_lazy; lazy++)
    {
        out_len = out_bufsize;
        r = lzo1x_999_compress_internal(in,in_len,out,&out_len,wrkmem,
                                        NULL, 0, 0,
                                        lazy, big, big, big, big, flags);
        if (r != LZO_E_OK)
        {
            /* this should NEVER happen */
            printf("internal error - compression failed: %d\n", r);
            exit(1);
        }
        printf("LZO1X-999: lazy =%2d: %8lu -> %8lu\n",
                lazy, (unsigned long) in_len, (unsigned long) out_len);
        if (out_len < best_len)
        {
            best_len = out_len;
            best_lazy = lazy;
            best_compress = 1;      /* LZO1X-999 */
        }
    }
#endif /* USE_LZO1X */

/*
 * Step 6b: compress from `in' to `out' with LZO1Y-999
 */
#ifdef USE_LZO1Y
    for (lazy = 0; lazy <= max_try_lazy; lazy++)
    {
        out_len = out_bufsize;
        r = lzo1y_999_compress_internal(in,in_len,out,&out_len,wrkmem,
                                        NULL, 0, 0,
                                        lazy, big, big, big, big, flags);
        if (r != LZO_E_OK)
        {
            /* this should NEVER happen */
            printf("internal error - compression failed: %d\n", r);
            exit(1);
        }
        printf("LZO1Y-999: lazy =%2d: %8lu -> %8lu\n",
                lazy, (unsigned long) in_len, (unsigned long) out_len);
        if (out_len < best_len)
        {
            best_len = out_len;
            best_lazy = lazy;
            best_compress = 2;      /* LZO1Y-999 */
        }
    }
#endif /* USE_LZO1Y */

/*
 * Step 7: check if compressible
 */
    if (best_len >= in_len)
    {
        printf("This file contains incompressible data.\n");
        return 0;
    }

/*
 * Step 8: compress data again using the best compressor found
 */
    out_len = out_bufsize;
    if (best_compress == 1)
        r = lzo1x_999_compress_internal(in,in_len,out,&out_len,wrkmem,
                                        NULL, 0, 0,
                                        best_lazy, big, big, big, big, flags);
    else if (best_compress == 2)
        r = lzo1y_999_compress_internal(in,in_len,out,&out_len,wrkmem,
                                        NULL, 0, 0,
                                        best_lazy, big, big, big, big, flags);
    else
        r = -100;
    assert(r == LZO_E_OK);
    assert(out_len == best_len);

/*
 * Step 9: optimize compressed data (compressed data is in `out' buffer)
 */
#if 1
    /* Optimization does not require any data in the buffer that will
     * hold the uncompressed data. To prove this, we clear the buffer.
     */
    lzo_memset(in,0,in_len);
#endif

    orig_len = in_len;
    r = -100;
#ifdef USE_LZO1X
    if (best_compress == 1)
        r = lzo1x_optimize(out,out_len,in,&orig_len,NULL);
#endif
#ifdef USE_LZO1Y
    if (best_compress == 2)
        r = lzo1y_optimize(out,out_len,in,&orig_len,NULL);
#endif
    if (r != LZO_E_OK || orig_len != in_len)
    {
        /* this should NEVER happen */
        printf("internal error - optimization failed: %d\n", r);
        exit(1);
    }

/*
 * Step 10: compute a checksum of the compressed data
 */
    compressed_checksum = lzo_adler32(0,NULL,0);
    compressed_checksum = lzo_adler32(compressed_checksum,out,out_len);

/*
 * Step 11: write compressed data to a file
 */
    printf("%s: %s: %ld -> %ld, checksum 0x%08lx 0x%08lx\n",
            progname, in_name, (long) in_len, (long) out_len,
            (long) uncompressed_checksum, (long) compressed_checksum);

    if (out_name && out_name[0])
    {
        printf("%s: writing to file %s\n", progname, out_name);
        f = fopen(out_name,"wb");
        if (f == NULL)
        {
            printf("%s: cannot open output file %s\n", progname, out_name);
            exit(1);
        }
        if (lzo_fwrite(f,out,out_len) != out_len || fclose(f) != 0)
        {
            printf("%s: write error !!\n", progname);
            exit(1);
        }
    }

/*
 * Step 12: verify decompression
 */
#ifdef PARANOID
    lzo_memset(in,0,in_len);    /* paranoia - clear output buffer */
    orig_len = in_len;
    r = -100;
#ifdef USE_LZO1X
    if (best_compress == 1)
        r = lzo1x_decompress_safe(out,out_len,in,&orig_len,NULL);
#endif
#ifdef USE_LZO1Y
    if (best_compress == 2)
        r = lzo1y_decompress_safe(out,out_len,in,&orig_len,NULL);
#endif
    if (r != LZO_E_OK || orig_len != in_len)
    {
        /* this should NEVER happen */
        printf("internal error - decompression failed: %d\n", r);
        exit(1);
    }
    if (uncompressed_checksum != lzo_adler32(lzo_adler32(0,NULL,0),in,in_len))
    {
        /* this should NEVER happen */
        printf("internal error - decompression data error\n");
        exit(1);
    }
    /* Now you could also verify decompression under similar conditions as in
     * your application, e.g. overlapping assembler decompression etc.
     */
#endif

    lzo_free(in);
    lzo_free(out);
    lzo_free(wrkmem);

    return 0;
}

/*
vi:ts=4:et
*/

