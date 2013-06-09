
CC=mipsel-uclibc-gcc -std=gnu99
CFLAGS= -g -O2 -fPIC -fPIE -fwrapv -fno-strict-aliasing -fno-strict-overflow -Winit-self -Wwrite-strings -Wdiv-by-zero
CPP= -D_XPG4_2=1 -D_GNU_SOURCE=1 -D_FORTIFY_SOURCE=2 -I/usr/local/include
LDFLAGS= -pie -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -L/usr/local/lib
LIBS=-ldl -lm 

TESTS_RESULTS = \
	auth.res auth2.res auth5.res box.res box2.res box7.res box8.res \
	core1.res core2.res core3.res core4.res core5.res core6.res \
	hash.res hash3.res onetimeauth.res onetimeauth2.res \
	onetimeauth7.res scalarmult.res scalarmult2.res scalarmult5.res \
	scalarmult6.res secretbox.res secretbox2.res secretbox7.res \
	secretbox8.res stream.res stream2.res stream3.res stream4.res

all: test

run-tests: $(TESTS_RESULTS)

test: .done

.done: run-tests
	touch .done

.exe.res:
	outfile=`echo "$@" | sed 's/.res$$/.out/'` ; \
	`pwd`/$< | cmp $$outfile || exit 1

.c.exe:
	nacl_lib="`pwd`/../../libnacl/build/localhost/lib/local" ; \
	nacl_include="`pwd`/../../libnacl/build/localhost/include/local" ; \
   	$(CC) $(CPPFLAGS) $(CFLAGS) -I$$nacl_include $< $$nacl_lib/randombytes.o -o $@ $(LDFLAGS) -L$$nacl_lib -lnacl $(LIBS)

clean:
	-rm -f   .done
	-rm -f  *.exe
	-rm -f  *.res
	-rm -fr *.dSYM

distclean: clean

.SUFFIXES: .c .cpp .res .exe
