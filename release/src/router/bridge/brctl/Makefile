
KERNEL_HEADERS=-I/home/vicente/tomato/release/src-rt/linux/linux-2.6/include

CC=mipsel-uclibc-gcc
CFLAGS= -Wall -Os -g -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32  
LDFLAGS=
INCLUDE=-I../libbridge $(KERNEL_HEADERS) 
LIBS= -L ../libbridge -lbridge 

prefix=
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
mandir=${prefix}/man

INSTALL=/usr/bin/install -c


common_SOURCES= brctl_cmd.c brctl_disp.c 
brctl_SOURCES=  brctl.c $(common_SOURCES)

common_OBJECTS= $(common_SOURCES:.c=.o)
brctl_OBJECTS= $(brctl_SOURCES:.c=.o)

OBJECTS= $(common_OBJECTS) $(brctl_OBJECTS)

PROGRAMS= brctl 


all:	$(PROGRAMS)

install: $(PROGRAMS)
	mkdir -p $(DESTDIR)$(sbindir)
	$(INSTALL) -m 755 $(PROGRAMS) $(DESTDIR)$(sbindir)

brctl:	$(brctl_OBJECTS) ../libbridge/libbridge.a
	$(CC) $(LDFLAGS) $(brctl_OBJECTS) $(LIBS) -o brctl

%.o: %.c brctl.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $< 

clean:
	rm -f *.o brctl core

