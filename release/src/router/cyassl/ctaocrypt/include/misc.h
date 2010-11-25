/* misc.h
 *
 * Copyright (C) 2006-2009 Sawtooth Consulting Ltd.
 *
 * This file is part of CyaSSL.
 *
 * CyaSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CyaSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#ifndef CTAO_CRYPT_MISC_H
#define CTAO_CRYPT_MISC_H


#include "types.h"
#include <string.h>



#ifdef __cplusplus
    extern "C" {
#endif


#ifdef NO_INLINE
word32 rotlFixed(word32, word32);
word32 rotrFixed(word32, word32);

word32 ByteReverseWord32(word32);
void   ByteReverseWords(word32*, const word32*, word32);
void   ByteReverseBytes(byte*, const byte*, word32);

void XorWords(word*, const word*, word32);
void xorbuf(byte*, const byte*, word32);
#endif /* NO_INLINE */


#ifdef __cplusplus
    }   /* extern "C" */
#endif


#endif /* CTAO_CRYPT_MISC_H */

