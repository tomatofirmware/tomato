
KERNEL_HEADERS=-I/home/vicente/repos/tomato/release/src-rt/linux/linux-2.6/include

#AR=ar
RANLIB=mipsel-uclibc-ranlib

CC=mipsel-uclibc-gcc
CFLAGS = -Wall -Os -g -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   $(KERNEL_HEADERS)

prefix=
exec_prefix=${prefix}
includedir=${prefix}/include
libdir=${exec_prefix}/lib

libbridge_SOURCES= \
	libbridge_devif.c \
	libbridge_if.c \
	libbridge_init.c \
	libbridge_misc.c 

libbridge_OBJECTS=$(libbridge_SOURCES:.c=.o)

all:	libbridge.a

install: libbridge.a
	mkdir -p $(DESTDIR)$(includedir)
	install -m 644 libbridge.h $(DESTDIR)$(includedir)
	mkdir -p $(DESTDIR)$(libdir)
	install -m 644 libbridge.a $(DESTDIR)$(libdir)


clean:
	rm -f *.o libbridge.a

libbridge.a:	$(libbridge_OBJECTS)
	$(AR) rcs $@ $(libbridge_OBJECTS)
	$(RANLIB) $@

%.o: %.c libbridge.h libbridge_private.h
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

libbridge_compat.o:	libbridge_compat.c if_index.c
	$(CC) $(CFLAGS) -c libbridge_compat.c

