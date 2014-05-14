man_MANS = osip.1

EXTRA_DIST = osip.sgml $(man_MANS)
DOCBOOKCMD=${shell which docbook-to-man || which docbook2man}

osip.1: osip.sgml
	-$(DOCBOOKCMD) $< > $@
