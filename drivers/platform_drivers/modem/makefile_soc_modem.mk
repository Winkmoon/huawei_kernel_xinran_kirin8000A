-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk
subdir-ccflags-y += -I$(srctree)/lib/libc_sec/securec_v2/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver/libhwsecurec/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/pub/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/include/platform/drv/ccore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/adrv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/product/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/msp/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/sys/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/msp/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/msg/public
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nva/comm/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/product_config/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/product/$(CUST_NAME)/define/acore/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/product/$(CUST_NAME)/define/common/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/product/$(OBB_PRODUCT_NAME)/product/$(CUST_NAME)/define/common/version_pub/

# ---------------------------------------------------------------------------
# Search paths shared by the module (=m) and built-in (=y) modem builds.
#
# Historically only the module branch (drv/makefile_soc_modem.mk) defined these,
# so CONFIG_MODEM_DRIVER=y failed to find headers such as mdrv_bmi_print.h
# (include/bmi/acore/kernel).  Define them here so both builds search the same
# places; adding an -I twice is harmless.
# ---------------------------------------------------------------------------
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/ccore/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/dsp/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/acore/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/acore/$(CFG_PLATFORM)/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/common/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/platform/common/$(CFG_PLATFORM)/soc/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/common/include/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/tl/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/tl/oam/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/tl/lps/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/sys/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/drv/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/pam/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/acore/guc_as/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/pam/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/guc_as/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/guc_nas/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/nv/common/tl_as
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/phy/lphy/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/nvim/include/gu/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/taf/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/taf/acore/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/rtc
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/mem \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/memory
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/memory_layout
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/om \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/om/common \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/om/dump \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/om/log
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/udi
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/timer
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/nmi
subdir-ccflags-y += -I$(srctree)/drivers/usb/gadget
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/tools
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/bmi/acore/kernel
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/include/bmi/acore/user/kernel
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/dt/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/dt/fdt/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/msg
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/fipc
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/pinctrl
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/diag/ \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/diag/msgserver \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/diag/srvchan
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/omp \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/hdlc
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/omserver/mprobe
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/dump \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/dump/logs \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/dump/mdmap
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V100)),YES)
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/dump/hal/v100
endif
ifeq ($(strip $(CFG_CONFIG_DUMP_PLATFORM_V200)),YES)
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/dump/hal/v200
endif
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/core \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/common \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/entry \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/tft \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/wan \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/dbg \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/modem_reset \
                    -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/app_if
ifeq ($(strip $(CFG_CONFIG_PFA_V100)),YES)
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v100
else
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/pfa/hal/v0
endif
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/core/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/debug/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/hal/common/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/odt/hal/v300/
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/sys_bus/intf
subdir-ccflags-y += -I$(srctree)/drivers/platform_drivers/modem/drv/sys_bus/test

# gen uuid
subdir-ccflags-y += -DCURRENT_MODEM_IMAGE_ID=\"$(GBB_MODEM_IMAGEID)\"

# Same warning relaxation the module branch uses; the built-in build otherwise
# trips over -Werror on files that are only compiled in this configuration.
subdir-ccflags-y += -Wno-unused-function
subdir-ccflags-y += -Wno-typedef-redefinition
subdir-ccflags-y += -Wno-undef

ifeq ($(strip $(OBB_LLT_MDRV)),y)
subdir-ccflags-y += -DDRV_BUILD_LLT
endif

ifeq ($(LLT_BUILD_GCOV), y)
subdir-ccflags-y += -DDRV_BUILD_GCOV
endif

# compile entry for phone
obj-$(CONFIG_MODEM_DRIVER) +=drv/
# compile entry for mbb
obj-$(CONFIG_MBB_MODEM) += drv/

ifneq ($(chip_type),es)
KBUILD_CFLAGS += -DCHIP_TYPE_CS=FEATURE_ON
endif

ifeq ($(secdog),rsa2048)
KBUILD_CFLAGS += -DSECDOG_SUPPORT_RSA_2048=FEATURE_ON
endif


ifeq ($(ota_netlock),true)
KBUILD_CFLAGS += -DFEATURE_OTA_NETLOCK=FEATURE_ON
endif

ifeq ($(simlock_cust),true)
KBUILD_CFLAGS += -DFEATURE_SIMLOCK_CUST=FEATURE_ON
endif

KBUILD_CFLAGS += -DMODULE_EXPORTED=

# compile entry for phone
obj-$(CONFIG_MODEM_DRIVER) +=ps/
# compile entry for mbb
obj-$(CONFIG_MBB_MODEM) += ps/

#add modem user
ifeq ($(choose_variant_modem),user)
KBUILD_CFLAGS += -DCHOOSE_MODEM_USER
endif

ifeq ($(cdma),false)
KBUILD_CFLAGS += -DFEATURE_UE_MODE_CDMA=FEATURE_OFF
KBUILD_CFLAGS += -DFEATURE_CSDR=FEATURE_OFF
endif


ifeq ($(strip $(CONFIG_MODEM_DRIVER)),m)

MODEM_DRIVER_MODULE := modem_driver
$(MODEM_DRIVER_MODULE)-y :=


# overwrite obj-m
obj-m	:= $(MODEM_DRIVER_MODULE).o

subdir-ccflags-y += -DCONFIG_MODEM_DRIVER -Wno-unused-function -Wno-undef

# drv
include $(srctree)/drivers/platform_drivers/modem/drv/Makefile
$(MODEM_DRIVER_MODULE)-y += $(addprefix drv/, $(drv-y))
obj-y	+= $(addprefix drv/, $(drv-builtin))

# ps
-include $(srctree)/drivers/platform_drivers/modem/ps/Makefile
$(MODEM_DRIVER_MODULE)-y += $(addprefix ps/, $(ps-objs))

endif
