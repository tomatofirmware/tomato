#
# Generic portion of the Broadcom wl driver makefile
#
# input: O_TARGET, CONFIG_WL_CONF and wl_suffix
# output: obj-m, obj-y
#
# $Id: wl_generic.mk,v 1.6 2009/05/19 01:39:54 Exp $
#

REBUILD_WL_MODULE=$(shell if [ -d "$(src)/$(SRCBASE)/wl/sys" -a "$(REUSE_PREBUILT_WL)" != "1" ]; then echo 1; else echo 0; fi)

# If source directory (src/wl/sys) exists and REUSE_PREBUILT_WL is undefined, 
# then build inside $(SRCBASE)/wl/sys, otherwise use pre-builts
ifeq ($(REBUILD_WL_MODULE),1)

    # Get the source files and flags from the specified config file
    # (Remove config's string quotes before trying to use the file)
    ifeq ($(CONFIG_WL_CONF),)
         $(error var_vlist($(VLIST)) var_config_wl_use($(shell env|grep CONFIG_WL_USE)))
         $(error CONFIG_WL_CONF is undefined)
    endif
    
    WLCONFFILE := $(strip $(subst ",,$(CONFIG_WL_CONF))) 
    WLCFGDIR   := $(src)/$(SRCBASE)/wl/config
    
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
    # If source directory (src/wl/sys) exists, use sources to build objects
    WL_OBJS   := $(foreach file, $(WL_SOURCE), \
		 $(if $(wildcard $(src)/$(SRCBASE)/wl/sys/$(file)), \
		 $(addprefix $(SRCBASE)/wl/sys/, $(patsubst %.c,%.o,$(file)))))
    WL_OBJS   += $(foreach file, $(WL_SOURCE), \
		 $(if $(wildcard $(src)/$(SRCBASE)/bcmcrypto/$(file)), \
		 $(addprefix $(SRCBASE)/bcmcrypto/, $(patsubst %.c,%.o,$(file)))))
    WL_OBJS   += $(foreach file, $(WL_SOURCE), \
		 $(if $(wildcard $(src)/$(SRCBASE)/shared/$(file)), \
		 $(addprefix $(SRCBASE)/shared/, $(patsubst %.c,%.o,$(file)))))
    WL_OBJS   += $(foreach file, $(WL_SOURCE), \
		 $(if $(wildcard $(src)/$(SRCBASE)/bcmsdio/sys/$(file)), \
		 $(addprefix $(SRCBASE)/bcmsdio/sys/, $(patsubst %.c,%.o,$(file)))))
    
    # need -I. to pick up wlconf.h in build directory
    
    EXTRA_CFLAGS += -DDMA $(WL_DFLAGS) -O2 -I$(src) -I$(src)/.. -I$(src)/$(SRCBASE)/wl/linux \
		    -I$(src)/$(SRCBASE)/wl/sys -finline-limit=2048 -Werror
    
    # If the PHY_HAL flag is defined we look in directory wl/phy for the
    # phy source files.
    ifneq ($(findstring PHY_HAL,$(WL_DFLAGS)),)
        WL_OBJS   += $(foreach file, $(WL_SOURCE), \
		     $(if $(wildcard $(src)/$(SRCBASE)/wl/phy/$(file)), \
		     $(addprefix $(SRCBASE)/wl/phy/, $(patsubst %.c,%.o,$(file)))))
        EXTRA_CFLAGS += -I$(src)/$(SRCBASE)/wl/phy
    endif

    # wl-objs is for linking to wl.o
    $(TARGET)-objs := $(WLCONF_O) $(WL_OBJS)
    obj-$(CONFIG_WL) := $(TARGET).o

else # SRCBASE/wl/sys doesn't exist

    # Otherwise, assume prebuilt object module(s) in src/wl/linux directory
    prebuilt := wl_$(wl_suffix).o
    $(TARGET)-objs := $(SRCBASE)/wl/linux/$(prebuilt)
    obj-$(CONFIG_WL) := $(TARGET).o

endif


clean-files += $(SRCBASE)/wl/sys/*.o $(SRCBASE)/wl/phy/*.o $(SRCBASE)/wl/sys/.*.*.cmd $(SRCBASE)/wl/phy/.*.*.cmd $(WLCONF_H) $(WLCONF_O)
