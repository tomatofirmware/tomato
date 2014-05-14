#
#    Copyright (C) 2002-2009  Thomas Ries <tries@gmx.net>
#
#    This file is part of Siproxd.
#    
#    Siproxd is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#    
#    Siproxd is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#    
#    You should have received a copy of the GNU General Public License
#    along with Siproxd; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
#


if use_convenience_ltdl
   LTDLDEF = -DLTDL_CONVLIB
endif

AM_CFLAGS = -D_GNU_SOURCE $(LTDLDEF) \
            -DBUILDSTR="\"`cat .buildno`\""

#
# Plugin modules, installed in "pkglib" directory ($prefix/lib/siproxd/)
#
pkglib_LTLIBRARIES =	plugin_demo.la \
			plugin_shortdial.la \
			plugin_logcall.la \
			plugin_defaulttarget.la \
			plugin_fix_bogus_via.la \
			plugin_stun.la \
			plugin_prefix.la \
			plugin_regex.la
DLOPENPLUGINS = 	-dlopen plugin_demo.la \
			-dlopen plugin_shortdial.la \
			-dlopen plugin_logcall.la \
			-dlopen plugin_defaulttarget.la \
			-dlopen plugin_fix_bogus_via.la \
			-dlopen plugin_stun.la \
			-dlopen plugin_prefix.la \
			-dlopen plugin_regex.la
#
plugin_demo_la_SOURCES = plugin_demo.c
plugin_demo_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_shortdial_la_SOURCES = plugin_shortdial.c
plugin_shortdial_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_logcall_la_SOURCES = plugin_logcall.c
plugin_logcall_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_defaulttarget_la_SOURCES = plugin_defaulttarget.c
plugin_defaulttarget_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_fix_bogus_via_la_SOURCES = plugin_fix_bogus_via.c
plugin_fix_bogus_via_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_stun_la_SOURCES = plugin_stun.c
plugin_stun_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_prefix_la_SOURCES = plugin_prefix.c
plugin_prefix_la_LDFLAGS = -module -avoid-version -shrext '.so'
#
plugin_regex_la_SOURCES = plugin_regex.c
plugin_regex_la_LDFLAGS = -module -avoid-version -shrext '.so'


#
# Siproxd itself
# (references DLOPENPLUGINS defined above - must be placed afterwards
#  else Cygwin goes beserk when building...)
#
sbin_PROGRAMS = siproxd
siproxd_LDFLAGS=-export-dynamic
siproxd_LDADD = $(LIBLTDL) $(DLOPENPLUGINS)
siproxd_SOURCES = siproxd.c proxy.c register.c sock.c utils.c \
		  sip_utils.c sip_layer.c log.c readconf.c rtpproxy.c \
		  rtpproxy_relay.c accessctl.c route_processing.c \
		  security.c auth.c fwapi.c resolve.c \
		  dejitter.c plugins.c redirect_cache.c


#
# an example for a custom firewall control module
# that can be linked into siproxd (--with-custom-fwmodule)
#
noinst_LIBRARIES = libcustom_fw_module.a
libcustom_fw_module_a_SOURCES = custom_fw_module.c


noinst_HEADERS = log.h siproxd.h digcalc.h rtpproxy.h \
		 fwapi.h plugins.h dejitter.h \
		 redirect_cache.h

EXTRA_DIST = .buildno

#
# count BUILDs - each time increment by one and define the
# C macro BUILDNO. bind it to a file everybody includes (log.h)
#
log.h: increment_build

increment_build:
	@echo "incrementing BUILD number"
	@if [ -f .buildno ]; then \
	  bld=`cat .buildno`; \
	  bld=`expr $${bld} + 1`; \
	else \
	  bld=1; \
	fi; \
	echo "$${bld}" >.buildno;
