#
# Generic portion of the Broadcom wl driver makefile
#
# input: O_TARGET, CONFIG_WL_CONF and wl_suffix
# output: obj-m, obj-y
#
# $Id$
#

REBUILD_WL_MODULE=$(shell if [ -d "$(SRCBASE)/wl/sys" -a "$(REUSE_PREBUILT_WL)" != "1" ]; then echo 1; else echo 0; fi)

# If source directory (src/wl/sys) exists and REUSE_PREBUILT_WL is undefined, 
# then build inside $(SRCBASE)/wl/sys, otherwise use pre-builts
ifeq ($(REBUILD_WL_MODULE),1)

    # If source directory (src/wl/sys) exists, use sources to build objects
    vpath %.c $(SRCBASE)/wl/sys $(SRCBASE)/shared $(SRCBASE)/bcmcrypto
    
    # Get the source files and flags from the specified config file
    # (Remove config's string quotes before trying to use the file)
    ifeq ($(CONFIG_WL_CONF),)
         $(error var_vlist($(VLIST)) var_config_wl_use($(shell env|grep CONFIG_WL_USE)))
         $(error CONFIG_WL_CONF is undefined)
    endif
    
    WLCONFFILE :=$(strip $(subst ",,$(CONFIG_WL_CONF))) 
    WLCFGDIR   := $(SRCBASE)/wl/config
    
    # define OS flag to pick up wl osl file from wl.mk
    WLLX=1
    include $(WLCFGDIR)/$(WLCONFFILE)
    include $(WLCFGDIR)/wl.mk
    
    ifeq ($(WLFILES),)
         $(error WLFILES is undefined in $(WLCFGDIR)/$(WLCONFFILE))
    endif
    
    ifeq ("$(CONFIG_WL_EMULATOR)","y") 
         WLFILES += wl_bcm57emu.c
    endif
    
    WL_SOURCE := $(WLFILES)
    WL_DFLAGS := $(WLFLAGS)
    WL_OBJS   := $(patsubst %.c,%.o,$(WL_SOURCE))
    
    # need -I. to pick up wlconf.h in build directory
    
    EXTRA_CFLAGS += -DDMA $(WL_DFLAGS) -I. -I$(SRCBASE)/wl/sys -finline-limit=2048 -Werror
    
    # obj-y is for linking to wl.o
    export-objs :=
    obj-y       := $(WL_OBJS)
    obj-m       := $(O_TARGET)

else # SRCBASE/wl/sys doesn't exist

    # Otherwise, assume prebuilt object module(s) in src/wl/linux directory
    prebuilt := wl_$(wl_suffix).o
    obj-y    := $(SRCBASE)/wl/linux/$(prebuilt)
    obj-m    := $(O_TARGET)

endif

include $(TOPDIR)/Rules.make

