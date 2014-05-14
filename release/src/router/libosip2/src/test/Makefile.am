
SUBDIRS = res

EXTRA_DIST = tst CHECK

if COMPILE_TESTS
noinst_PROGRAMS = torture_test turl tfrom tto tcontact tvia tcallid tcontentt trecordr troute twwwa

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src/osipparser2
AM_CFLAGS = $(SIP_CFLAGS) $(SIP_PARSER_FLAGS) $(SIP_EXTRA_FLAGS)

twwwa_SOURCES =  twwwa.c
twwwa_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la

tcontentt_SOURCES =  tcontentt.c
tcontentt_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

turl_SOURCES =  turls.c
turl_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

tfrom_SOURCES =  tfrom.c
tfrom_LDADD =  $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

tto_SOURCES =  tto.c
tto_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 


tcontact_SOURCES =  tcontact.c
tcontact_LDADD =  $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

trecordr_SOURCES =  trecordr.c
trecordr_LDADD =  $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

troute_SOURCES =  troute.c
troute_LDADD =  $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

tvia_SOURCES =  tvia.c
tvia_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

tcallid_SOURCES =  tcallid.c
tcallid_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 

torture_test_SOURCES =  torture.c
torture_test_LDADD = $(PARSER_LIB) $(EXTRA_LIB) $(top_builddir)/src/osipparser2/libosipparser2.la 



check:
	@echo " *******************************"
	@echo " ****** starting tests! ********"
	@echo " *******************************"
	@./$(top_srcdir)/src/test/tst ./$(top_srcdir)/src/test/res -c

	@echo ""
	@echo "In case you have a doubt, send the generated"
	@echo "log file with your comment to <amoizard@antisip.com>."
else
check:
	@echo " ************************************"
	@echo " Please use ./configure --enable-test"
	@echo " ************************************"
endif
