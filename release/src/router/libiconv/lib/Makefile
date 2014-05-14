# Makefile for libiconv/lib

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = .

# Directories used by "make install":
prefix = /usr
local_prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib

# Programs used by "make":
CC = mipsel-uclibc-gcc
CFLAGS = -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32r2 -mips32r2   -ffunction-sections -fdata-sections -fvisibility=hidden
CPPFLAGS = 
LDFLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections -fPIC $(LDFLAGS_no)
LDFLAGS_yes = -Wl,--export-all-symbols
LDFLAGS_no =
INCLUDES = -I. -I$(srcdir) -I../include -I$(srcdir)/../include -I.. -I$(srcdir)/..
# -DBUILDING_LIBICONV: Change expansion of LIBICONV_DLL_EXPORTED macro.
# -DBUILDING_DLL: Change expansion of RELOCATABLE_DLL_EXPORTED macro.
DEFS = -DLIBDIR=\"$(libdir)\" -DBUILDING_LIBICONV -DBUILDING_DLL \
-DENABLE_RELOCATABLE=1 -DIN_LIBRARY -DINSTALLDIR=\"$(libdir)\" -DNO_XMALLOC \
-Dset_relocation_prefix=libiconv_set_relocation_prefix \
-Drelocate=libiconv_relocate -DHAVE_CONFIG_H
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBTOOL_COMPILE = $(LIBTOOL) --mode=compile
LIBTOOL_LINK = $(LIBTOOL) --mode=link
LIBTOOL_INSTALL = $(LIBTOOL) --mode=install
LIBTOOL_UNINSTALL = $(LIBTOOL) --mode=uninstall
CP = cp
MV = mv
LN = ln
LN_S = ln -s
RM = rm -f

# Programs used by "make install":
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
mkinstalldirs = $(SHELL) ../build-aux/mkinstalldirs

#### End of system configuration section. ####

SHELL = /bin/sh

# Before making a release, change this according to the libtool documentation,
# section "Library interface versions".
LIBICONV_VERSION_INFO = 6:0:4

# Needed by $(LIBTOOL).
top_builddir = ..

SOURCES = $(srcdir)/iconv.c $(srcdir)/../libcharset/lib/localcharset.c $(srcdir)/relocatable.c

OBJECTS = iconv.lo localcharset.lo relocatable.lo $(OBJECTS_no)
OBJECTS_yes = iconv-exports.lo
OBJECTS_no =

all : preloadable_libiconv.so libiconv.la

libiconv.la : $(OBJECTS)
	$(LIBTOOL_LINK) $(CC) $(LDFLAGS) -o libiconv.la -rpath $(libdir) -version-info $(LIBICONV_VERSION_INFO) -no-undefined $(OBJECTS)

preloadable_libiconv.so : preloadable_libiconv_linux.so
	$(RM) preloadable_libiconv.so
	$(LN_S) preloadable_libiconv_linux.so preloadable_libiconv.so

preloadable_libiconv_linux.so : $(SOURCES)
	if test -n "yes"; then \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -fPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -Xcompiler -shared -o preloadable_libiconv_linux.so; \
	else \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -KPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -Xcompiler -shared -o preloadable_libiconv_linux.so; \
	fi

preloadable_libiconv_solaris.so : $(SOURCES)
	if test -n "yes"; then \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -fPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -shared -o preloadable_libiconv_solaris.so; \
	else \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -KPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -G -o preloadable_libiconv_solaris.so; \
	fi

preloadable_libiconv_osf.so : $(SOURCES)
	if test -n "yes"; then \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -fPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -shared -o preloadable_libiconv_osf.so; \
	else \
	  mkdir objects; \
	  for f in $(SOURCES); do \
	    $(CC) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -DPIC -DLIBICONV_PLUG -c $$f -o objects/`basename $$f | sed -e 's,\.c$$,.o,'`; \
	  done; \
	  /bin/ld -shared -expect_unresolved \* -o preloadable_libiconv_osf.so objects/*.o; \
	  rm -rf objects; \
	fi

iconv.lo : $(srcdir)/iconv.c $(srcdir)/converters.h $(srcdir)/encodings.def $(srcdir)/encodings_aix.def $(srcdir)/encodings_osf1.def $(srcdir)/encodings_dos.def $(srcdir)/encodings_local.def $(srcdir)/aliases.h $(srcdir)/aliases_aix.h $(srcdir)/aliases_osf1.h $(srcdir)/aliases_dos.h $(srcdir)/flags.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/iconv.c

localcharset.lo : $(srcdir)/../libcharset/lib/localcharset.c $(srcdir)/relocatable.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/../libcharset/lib/localcharset.c

relocatable.lo : $(srcdir)/relocatable.c $(srcdir)/relocatable.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/relocatable.c

iconv-exports.lo : $(srcdir)/../woe32dll/iconv-exports.c
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/../woe32dll/iconv-exports.c

# Installs the library and include files only. Typically called with only
# $(libdir) and $(includedir) - don't use $(prefix) and $(exec_prefix) here.
install-lib : all force
	if [ ! -d $(libdir) ] ; then $(mkinstalldirs) $(libdir) ; fi
	$(LIBTOOL_INSTALL) $(INSTALL_DATA) libiconv.la $(libdir)/libiconv.la

install : all force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(exec_prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(exec_prefix) ; fi
	if [ ! -d $(DESTDIR)$(libdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(libdir) ; fi
	$(LIBTOOL_INSTALL) $(INSTALL_DATA) libiconv.la $(DESTDIR)$(libdir)/libiconv.la
	if test -n "preloadable_libiconv.so"; then $(INSTALL_DATA) preloadable_libiconv.so $(DESTDIR)$(libdir)/preloadable_libiconv.so.new && mv $(DESTDIR)$(libdir)/preloadable_libiconv.so.new $(DESTDIR)$(libdir)/preloadable_libiconv.so ; fi

installdirs : force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(exec_prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(exec_prefix) ; fi
	if [ ! -d $(DESTDIR)$(libdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(libdir) ; fi

uninstall : force
	$(LIBTOOL_UNINSTALL) $(RM) $(DESTDIR)$(libdir)/libiconv.la
	if test -n "preloadable_libiconv.so"; then $(RM) $(DESTDIR)$(libdir)/preloadable_libiconv.so ; fi

check : all

mostlyclean : clean

clean : force
	$(RM) *.o *.lo *.a *.la preloadable_libiconv* core *.stackdump so_locations
	$(RM) -r .libs _libs

distclean : clean
	$(RM) Makefile config.h libtool

maintainer-clean : distclean

force :

