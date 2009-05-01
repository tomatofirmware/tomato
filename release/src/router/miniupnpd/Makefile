# $Id: Makefile.linux,v 1.41 2008/10/09 11:28:44 nanard Exp $
# MiniUPnP project
# http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
# Author : Thomas Bernard
# for use with GNU Make
# Hand crafted for use with Tomato

include ../common.mak

CP = cp
RM = rm -f

CFLAGS += -Os -Wall -D_GNU_SOURCE
IPTABLESPATH = ../iptables
LIBS = -L$(IPTABLESPATH)/ -liptc
BINDIR = /usr/sbin
CFLAGS += -I$(IPTABLESPATH)/include/

#Uncomment next line if using IPTABLES ver. 1.4.3 or later
#CFLAGS += -DIPTABLES_143

BASEOBJS = miniupnpd.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o \
		   upnpredirect.o getifaddr.o daemonize.o upnpglobalvars.o \
           options.o upnppermissions.o minissdp.o natpmp.o \
           upnpevents.o

LNXOBJS = linux/getifstats.o
NETFILTEROBJS = netfilter/iptcrdr.o

ALLOBJS = $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = miniupnpd testupnpdescgen

.PHONY:	all clean install depend

all:	$(EXECUTABLES)

clean:
	$(RM) $(ALLOBJS)
	$(RM) $(EXECUTABLES)
	$(RM) testupnpdescgen.o testgetifstats.o
	$(RM) testupnppermissions.o
	$(RM) miniupnpdctl.o
	$(RM) config.h

install:	miniupnpd
	@mkdir -p $(INSTALLDIR)$(BINDIR)
	@install miniupnpd $(INSTALLDIR)$(BINDIR)
	@$(STRIP) -s $(INSTALLDIR)$(BINDIR)/miniupnpd
#	mv $(INSTALLDIR)$(BINDIR)/miniupnpd $(INSTALLDIR)$(BINDIR)/upnp

# genuuid is using the uuidgen CLI tool which is part of libuuid
# from the e2fsprogs
genuuid:
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`(genuuid||uuidgen) 2>/dev/null`/" miniupnpd.conf

miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)
	@echo " [miniupnpd] CC -o $@"
	@$(CC) $(CFLAGS) -s -o $@ $^ $(LIBS)
	$(SIZECHECK)
	$(CPTMP)

testupnpdescgen:	$(TESTUPNPDESCGENOBJS)

netfilter/iptcrdr.o: netfilter/iptcrdr.c
	$(CC) $(CFLAGS) -c -o $@ $<

testgetifstats:	testgetifstats.o linux/getifstats.o

testupnppermissions:	testupnppermissions.o upnppermissions.o

testgetifaddr:	testgetifaddr.o getifaddr.o

miniupnpdctl:	miniupnpdctl.o

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) \
	testgetifstats.c 2>/dev/null
config.h:	config.h.tomato
	$(CP) $< $@

# DO NOT DELETE

miniupnpd.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
miniupnpd.o: upnphttp.h upnpdescgen.h miniupnpdpath.h getifaddr.h upnpsoap.h
miniupnpd.o: options.h minissdp.h upnpredirect.h daemonize.h upnpevents.h
miniupnpd.o: natpmp.h commonrdr.h
upnphttp.o: config.h upnphttp.h upnpdescgen.h miniupnpdpath.h upnpsoap.h
upnphttp.o: upnpevents.h
upnpdescgen.o: config.h upnpdescgen.h miniupnpdpath.h upnpglobalvars.h
upnpdescgen.o: upnppermissions.h miniupnpdtypes.h upnpdescstrings.h
upnpsoap.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
upnpsoap.o: upnphttp.h upnpsoap.h upnpreplyparse.h upnpredirect.h getifaddr.h
upnpsoap.o: getifstats.h
upnpreplyparse.o: upnpreplyparse.h minixml.h
minixml.o: minixml.h
upnpredirect.o: config.h upnpredirect.h upnpglobalvars.h upnppermissions.h
upnpredirect.o: miniupnpdtypes.h upnpevents.h netfilter/iptcrdr.h commonrdr.h
getifaddr.o: getifaddr.h
daemonize.o: daemonize.h config.h
upnpglobalvars.o: config.h upnpglobalvars.h upnppermissions.h
upnpglobalvars.o: miniupnpdtypes.h
options.o: options.h config.h upnppermissions.h upnpglobalvars.h
options.o: miniupnpdtypes.h
upnppermissions.o: config.h upnppermissions.h
minissdp.o: config.h upnpdescstrings.h miniupnpdpath.h upnphttp.h
minissdp.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h minissdp.h
minissdp.o: codelength.h
natpmp.o: config.h natpmp.h upnpglobalvars.h upnppermissions.h
natpmp.o: miniupnpdtypes.h getifaddr.h upnpredirect.h commonrdr.h
upnpevents.o: config.h upnpevents.h miniupnpdpath.h upnpglobalvars.h
upnpevents.o: upnppermissions.h miniupnpdtypes.h upnpdescgen.h
linux/getifstats.o: getifstats.h config.h
netfilter/iptcrdr.o: netfilter/iptcrdr.h commonrdr.h config.h
netfilter/iptcrdr.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
testupnpdescgen.o: config.h upnpdescgen.h
upnpdescgen.o: config.h upnpdescgen.h miniupnpdpath.h upnpglobalvars.h
upnpdescgen.o: upnppermissions.h miniupnpdtypes.h upnpdescstrings.h
testgetifstats.o: getifstats.h
