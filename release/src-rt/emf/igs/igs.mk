#
# Copyright (C) 2009, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: igs.mk,v 1.1 2007/03/17 03:14:54 Exp $
#

CROSS_COMPILE = mipsel-linux-
KSRC := $(shell /bin/pwd)/../../..
KINCLUDE = -I$(KSRC)/include -I$(KSRC)/linux/linux/include/asm/gcc \
	   -I$(KSRC)/router/emf/igs -I$(KSRC)/router/emf/emf

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

OFILES = igsc.o igs_linux.o igsc_sdb.o osl_linux.o

IFLAGS1 = -I$(KSRC)/linux/linux/include
IFLAGS2 = $(KINCLUDE)
DFLAGS += -DMODULE -D__KERNEL__ $(IFLAGS) -G 0 -mno-abicalls \
	  -fno-pic -pipe -gstabs+ -mcpu=r4600 -mips2 -Wa,--trap \
	  -m4710a0kern -mlong-calls -fno-common -nostdinc \
	  -iwithprefix include


IGSFLAGS += -DBCMINTERNAL -DBCMDBG

WFLAGS = $(IFLAGS1) -Wall -Wstrict-prototypes -Wno-trigraphs -O2 \
	 -fno-strict-aliasing -fno-common -fomit-frame-pointer $(IFLAGS2)

CFLAGS += $(WFLAGS) $(DFLAGS) $(IGSFLAGS)

TARGETS = igs.o

all: $(TARGETS)

igs.o: $(OFILES)
	$(LD) -r -o $@ $(OFILES)

clean:
	rm -f $(OFILES) $(TARGETS) *~

.PHONY: all clean

include $(KSRC)/linux/linux/Rules.make
