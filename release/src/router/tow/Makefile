include ../common.mak

CFLAGS = -O2 -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include -I.
ifeq ($(STATIC),1)
CFLAGS += -static
endif

all: base64

base64: base64.o
	@echo " [base64] CC $@"
	@$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f base64 *.o

	
%.o: %.c .%.depend
	@echo " [base64] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
