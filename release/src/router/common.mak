ifeq ($(SRCBASE),)
	# ..../src/router/
	# (directory of the last (this) makefile)
	export TOP := $(shell cd $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))) && pwd -P)

	# ..../src/
	export SRCBASE := $(shell (cd $(TOP)/.. && pwd -P))
else
	export TOP := $(SRCBASE)/router
endif

include $(SRCBASE)/tomato_profile.mak
include $(TOP)/.config

export BUILD := $(shell (gcc -dumpmachine))
export HOSTCC := gcc

export PLATFORM := mipsel-uclibc

export CROSS_COMPILE := mipsel-uclibc-
export CROSS_COMPILER := $(CROSS_COMPILE)
export CONFIGURE := ./configure --host=mipsel-linux --build=$(BUILD)
export TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)gcc))/.. && pwd -P)

export CC := $(CROSS_COMPILE)gcc
export AR := $(CROSS_COMPILE)ar
export AS := $(CROSS_COMPILE)as
export LD := $(CROSS_COMPILE)ld
export NM := $(CROSS_COMPILE)nm
export RANLIB := $(CROSS_COMPILE)ranlib
export STRIP := $(CROSS_COMPILE)strip -R .note -R .comment
export SIZE := $(CROSS_COMPILE)size

# Determine kernel version
kver=$(subst ",,$(word 3, $(shell grep "UTS_RELEASE" $(LINUXDIR)/include/linux/$(1))))

LINUX_KERNEL=$(call kver,version.h)
ifeq ($(LINUX_KERNEL26),)
LINUX_KERNEL=$(call kver,utsrelease.h)
endif

include $(SRCBASE)/target.mak

export LIBDIR := $(TOOLCHAIN)/lib
export USRLIBDIR := $(TOOLCHAIN)/usr/lib

export PLATFORMDIR := $(TOP)/$(PLATFORM)
export INSTALLDIR := $(PLATFORMDIR)/install
export TARGETDIR := $(PLATFORMDIR)/target

ifeq ($(EXTRACFLAGS),)
export EXTRACFLAGS := -DBCMWPA2 -fno-delete-null-pointer-checks -mips32 -mtune=mips32
endif

CPTMP = @[ -d $(TOP)/dbgshare ] && cp $@ $(TOP)/dbgshare/ || true

ifeq ($(CONFIG_LINUX26),y)
export KERNELCC := $(CC)
else
export KERNELCC := $(CC)-3.4.6
endif

#	ifneq ($(STATIC),1)
#	SIZECHECK = @$(SRCBASE)/btools/sizehistory.pl $@ $(TOMATO_PROFILE_L)_$(notdir $@)
#	else
SIZECHECK = @$(SIZE) $@
#	endif
