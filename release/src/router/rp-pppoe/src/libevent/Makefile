# Generated automatically from Makefile.in by configure.
# $Id$
#
# Makefile for event-handling library
#
# Copyright 2002 Roaring Penguin Software Inc.
#
# This software may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
# LIC: GPL

OBJS=event.o event_tcp.o hash.o event_sig.o
SRCS=$(OBJS:.o=.c)
HDRS=event.h event_tcp.h eventpriv.h hash.h
CFLAGS=-g -O2 -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-strict-aliasing -Wall -Wstrict-prototypes -I.. $(DEFINES)
AR=ar

all: libevent.a

libevent.a: $(OBJS)
	rm -f libevent.a
	$(AR) -cq libevent.a $(OBJS)
	mipsel-uclibc-ranlib libevent.a

event.o: event.c $(HDRS)
	mipsel-uclibc-gcc $(CFLAGS) -c -o $@ $<

hash.o: hash.c $(HDRS)
	mipsel-uclibc-gcc $(CFLAGS) -c -o $@ $<

event_sig.o: event_sig.c $(HDRS)
	mipsel-uclibc-gcc $(CFLAGS) -c -o $@ $<

event_tcp.o: event_tcp.c $(HDRS)
	mipsel-uclibc-gcc $(CFLAGS) -c -o $@ $<

clean: FORCE
	rm -f *.a *.o *~

FORCE:

.phony: FORCE
