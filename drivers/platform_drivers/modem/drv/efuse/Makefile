
-include $(srctree)/drivers/platform_drivers/modem/drv/product_config/product_config.mk

ifeq ($(strip $(CFG_CONFIG_EFUSE)),YES)
obj-$(CONFIG_EFUSE_IOT)		+= efuse_driver.o
obj-$(CONFIG_EFUSE_MODEM_AGENT)	+= efuse_agent.o
# efuse_layout.c implements the non-stub bsp_efuse_get_layout_info() and is
# only valid when bsp_efuse.h takes its "defined(CONFIG_EFUSE) &&
# defined(CONFIG_EFUSE_IOT)" branch; otherwise the header provides a static
# inline stub and this file fails with a redefinition.  The module build
# (=m) never compiles it either.  Keep the two in sync.
obj-$(CONFIG_EFUSE_IOT)		+= efuse_layout.o
endif
obj-y	+= efuse_ioctl.o
obj-y	+= efuse_dump.o

GCOV_PROFILE := y