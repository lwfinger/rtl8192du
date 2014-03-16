EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS)
EXTRA_CFLAGS += -O1

EXTRA_CFLAGS += -Wno-unused-variable
EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused

EXTRA_CFLAGS += -Wno-uninitialized

EXTRA_CFLAGS += -I$(src)/include

CONFIG_AUTOCFG_CP = n

CONFIG_POWER_SAVING = y
CONFIG_BT_COEXISTENCE = n
CONFIG_WAKE_ON_WLAN = n

export TopDIR ?= $(shell pwd)

ccflags-y += -D__CHECK_ENDIAN__

ifeq ($(CONFIG_POWER_SAVING), y)
EXTRA_CFLAGS += -DCONFIG_POWER_SAVING
endif

ifeq ($(CONFIG_BT_COEXISTENCE), y)
EXTRA_CFLAGS += -DCONFIG_BT_COEXISTENCE
endif

ifeq ($(CONFIG_WAKE_ON_WLAN), y)
EXTRA_CFLAGS += -DCONFIG_WAKE_ON_WLAN
endif

SUBARCH := $(shell uname -m | sed -e s/i.86/i386/ | sed -e s/ppc/powerpc/)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/
INSTALL_PREFIX :=

ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

ifneq ($(KERNELRELEASE),)


rtk_core :=			\
		core/rtw_ap.o \
		core/rtw_cmd.o \
		core/rtw_debug.o \
		core/rtw_efuse.o \
		core/rtw_ieee80211.o \
		core/rtw_io.o \
		core/rtw_ioctl_set.o \
		core/rtw_mlme.o \
		core/rtw_mlme_ext.o \
		core/rtw_p2p.o \
		core/rtw_pwrctrl.o \
		core/rtw_recv.o \
		core/rtw_rf.o \
		core/rtw_security.o \
		core/rtw_sta_mgt.o \
		core/rtw_wlan_util.o \
		core/rtw_xmit.o	\
		hal/hal_com.o \
		hal/hal_intf.o \
		hal/rtl8192d_cmd.o \
		hal/rtl8192d_dm.o \
		hal/rtl8192d_hal_init.o \
		hal/rtl8192d_phycfg.o \
		hal/rtl8192d_rf6052.o \
		hal/rtl8192d_rxdesc.o \
		hal/rtl8192d_xmit.o \
		hal/rtl8192du_led.o \
		hal/rtl8192du_xmit.o \
		hal/rtl8192du_recv.o \
		hal/Hal8192DUHWImg.o \
		hal/usb_halinit.o \
		hal/usb_ops_linux.o \
		os_dep/ioctl_cfg80211.o \
		os_dep/ioctl_linux.o \
		os_dep/mlme_linux.o \
		os_dep/osdep_service.o \
		os_dep/os_intfs.o \
		os_dep/rtw_android.o \
		os_dep/usb_intf.o \
		os_dep/usb_ops_linux.o \
		os_dep/xmit_linux.o \
		os_dep/recv_linux.o

8192du-y += $(rtk_core)


obj-$(CONFIG_RTL8192DU) := 8192du.o

else

export CONFIG_RTL8192DU = m

all: modules

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

strip:
	$(CROSS_COMPILE)strip 8192du.ko --strip-unneeded

install:
	install -p -m 644 8192du.ko  $(MODDESTDIR)
	mkdir -p /lib/firmware/rtlwifi
	cp -n rtl8192dufw*.bin /lib/firmware/rtlwifi/.
	/sbin/depmod -a ${KVER}

uninstall:
	rm -f $(MODDESTDIR)/8192du.ko
	/sbin/depmod -a ${KVER}


config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in

.PHONY: modules clean

clean:
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm .tmp_versions -fr ; rm Module.symvers -fr
	rm -fr Module.markers ; rm -fr modules.order
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd hal ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
endif

