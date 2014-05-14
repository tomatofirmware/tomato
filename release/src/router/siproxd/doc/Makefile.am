#
#    Copyright (C) 2002-2008  Thomas Ries <tries@gmx.net>
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

# missing with older automake/autoconf packages
docdir = ${datadir}/doc/${PACKAGE}

SUBDIRS = 

EXTRA_DIST = 	siproxd.conf.example \
		siproxd_passwd.cfg \
		FAQ KNOWN_BUGS \
		FLI4L_HOWTO.txt \
		RFC3261_compliance.txt \
		sample_cfg_budgetone.txt \
		sample_cfg_x-lite.txt \
		sample_asterisk.txt \
		siproxd_guide.sgml
#
# docbook stuff
#
all-local: local_dirs docbook_html docbook_pdf

local_dirs:
	mkdir -p html
	mkdir -p pdf

docbook_html: siproxd_guide.sgml
if have_docbook2html
	if [ html/siproxd_guide.html -ot siproxd_guide.sgml ]; then \
	  docbook2html -o html/ siproxd_guide.sgml; \
	fi
endif

docbook_pdf: siproxd_guide.sgml
if have_docbook2pdf
	if [ pdf/siproxd_guide.pdf -ot siproxd_guide.sgml ]; then \
	  docbook2pdf -o pdf/ siproxd_guide.sgml; \
	fi
endif

clean-local:
	rm -rf html/*
	rm -rf pdf/*

#
# install local data
#
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	$(INSTALL_DATA) $(srcdir)/siproxd.conf.example	$(DESTDIR)$(sysconfdir)/
	$(INSTALL_DATA) $(srcdir)/siproxd_passwd.cfg	$(DESTDIR)$(sysconfdir)/
	chmod 600 $(DESTDIR)$(sysconfdir)/siproxd_passwd.cfg
	$(mkinstalldirs) $(DESTDIR)$(docdir)
if have_docbook2pdf
	$(INSTALL_DATA) $(srcdir)/pdf/*			$(DESTDIR)$(docdir)/
endif
if have_docbook2html
	$(INSTALL_DATA) $(srcdir)/html/*		$(DESTDIR)$(docdir)/
endif
